using GLib;

[CCode (lower_case_cprefix = "cscoin_")]
namespace CSCoin
{
	extern string solve_challenge (int          challenge_id,
	                               string       challenge_name,
	                               string       last_solution_hash,
	                               string       hash_prefix,
	                               int          nb_elements,
	                               Cancellable? cancellable = null) throws Error;

	Json.Node generate_command (string command_name, ...)
	{
		var builder = new Json.Builder ();

		builder.begin_object ();
		builder.set_member_name ("command");
		builder.add_string_value (command_name);

		builder.set_member_name ("args");
		builder.begin_object ();
		var list = va_list ();
		unowned string? key, val;
		for (key = list.arg<string?> (), val = list.arg<string?> ();
		     key != null && val != null;
		     key = list.arg<string?> (), val = list.arg<string?> ())
		{
			builder.set_member_name (key.replace ("-", "_"));
			builder.add_string_value (val);
		}
		// TODO: add command arguments
		builder.end_object ();

		builder.end_object ();

		return builder.get_root ();
	}

	string wallet_path;

	const OptionEntry[] options =
	{
		{"wallet", 'w', 0, OptionArg.FILENAME, ref wallet_path, "Path to the wallet if it exists, otherwise it will be created", "FILE"},
		{null}
	};

	int main (string[] args)
	{
		try
		{
			var parser = new OptionContext ();
			parser.add_main_entries (options, null);
			parser.parse (ref args);
		}
		catch (OptionError err)
		{
			return 1;
		}

		if (args.length < 2)
		{
			stderr.printf ("Usage: %s <url>\n", args[0]);
			return 1;
		}

		var wallet = new OpenSSL.RSA ();

		message ("Generating a random key pair...");
		var e = new OpenSSL.Bignum ();
		e.set_word (65537);
		wallet.generate_key_ex (1024, e);

		var wallet_id = Checksum.compute_for_string (ChecksumType.SHA256, wallet.d.bn2dec ());

		var ws_url = args[1];

		var ws_session = new Soup.Session ();
		var ws_message = new Soup.Message ("GET", ws_url);

		ws_session.websocket_connect_async.begin (ws_message, null, null, null, (obj, res) => {
			Soup.WebsocketConnection ws;
			try
			{
				ws = ws_session.websocket_connect_async.end (res);
			}
			catch (Error err)
			{
				error (err.message);
			}

			Cancellable? current_challenge_cancellable = null;

			var challenge_executor = new ThreadPool<Json.Object>.with_owned_data ((challenge) => {
				var time_left = challenge.get_int_member ("time_left");
				var challenge_id = challenge.get_int_member ("challenge_id");
				var challenge_name = challenge.get_string_member ("challenge_name");
				var last_solution_hash = challenge.get_string_member ("last_solution_hash");
				var hash_prefix = challenge.get_string_member ("hash_prefix");
				var parameters = challenge.get_member ("parameters");

				message ("Received challenge #%lld: challenge-name: %s, last-solution-hash: %s, hash-prefix: %s, parameters: %s.",
				         challenge_id,
				         challenge_name,
				         last_solution_hash,
				         hash_prefix,
				         Json.to_string (parameters, false));

				var started = get_monotonic_time ();

				string nonce;
				try
				{
					nonce = solve_challenge ((int) challenge_id,
					                         challenge_name,
					                         last_solution_hash,
					                         hash_prefix,
					                         (int) parameters.get_object ().get_int_member ("nb_elements"),
					                         current_challenge_cancellable);
				}
				catch (Error err)
				{
					critical (err.message);
					return;
				}

				message ("Solved challenge #%lld in %lldms (%lds was given) with nonce '%s'.",
				         challenge_id,
				         (get_monotonic_time () - started) / 1000,
				         time_left,
				         nonce);

				message ("Submitting nonce '%s' for challenge #%lld to authority...", nonce, challenge_id);
				ws.send_text (Json.to_string (generate_command ("submission", wallet_id: wallet_id, nonce: nonce.to_string ()), false));
			}, 1, true);

			ws.closing.connect (() => {
				warning ("The connection is closing...");
			});

			ws.closed.connect (() => {
				message ("The connection is closed and will be reestablished in a few...");
			});

			ws.error.connect ((err) => {
				critical (err.message);
			});

			ws.message.connect ((type, payload) => {
				var response = Json.from_string ((string) payload.get_data ()).get_object ();

				if (response.has_member ("challenge_id"))
				{
					/* cancel any running challenge */
					current_challenge_cancellable.cancel ();
					current_challenge_cancellable = new Cancellable ();

					try
					{
						challenge_executor.add (response);
					}
					catch (ThreadError err)
					{
						critical ("Could not enqueue the challenge #%lld: %s", response.get_int_member ("challenge_id"), err.message);
					}
				}
				else if (response.has_member ("error"))
				{
					critical ("Received an error from CA: %s.", response.get_string_member ("error"));
				}
			});

			ws.send_text (Json.to_string (generate_command ("register_wallet", name: "diroum", key: "", signature: ""), false));
			ws.send_text (Json.to_string (generate_command ("get_current_challenge"), false));
		});

		new MainLoop ().run ();

		return 0;
	}
}

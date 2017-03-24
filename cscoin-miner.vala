using GLib;

[CCode (lower_case_cprefix = "cscoin_")]
namespace CSCoin
{
	string generate_command (string command_name, ...)
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

		return Json.to_string (builder.get_root (), false);
	}

	/**
	 * Path to the RSA wallet containing both public and private keys.
	 */
	string wallet_path;

	const OptionEntry[] options =
	{
		{"wallet", 'w', 0, OptionArg.FILENAME, ref wallet_path, "Path to the wallet.", "FILE"},
		{null}
	};

	int main (string[] args)
	{
		// default options
		wallet_path = "default.pem";

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
			stderr.printf ("Usage: %s [--wallet=<wallet_file>] <ws_url>\n", args[0]);
			return 1;
		}

		Wallet wallet;
		try
		{
			message ("Loading an existing wallet from '%s'...", wallet_path);
			wallet = new Wallet.from_path (wallet_path);
		}
		catch (Error err)
		{
			stderr.printf ("Could not read the wallet at '%s'.", wallet_path);
			return 1;
		}

		message ("The 'wallet_id' is '%s'.", wallet.get_wallet_id ());

		loop.begin (args[1], wallet);

		new MainLoop ().run ();

		return 0;
	}

	async void loop (string ws_url, Wallet wallet)
	{
		var ws_session = new Soup.Session ();

		Soup.WebsocketConnection ws = null;

		var challenge_executor = new ThreadPool<Challenge>.with_owned_data ((challenge) => {
			message ("Received challenge #%lld: challenge-name: %s, last-solution-hash: %s, hash-prefix: %s.",
			         challenge.challenge_id,
			         challenge.challenge_type.to_string (),
			         challenge.last_solution_hash,
			         challenge.hash_prefix);

			var started = get_monotonic_time ();

			string? nonce;
			try
			{
				nonce = solve_challenge (challenge.challenge_id,
				                         challenge.challenge_type,
				                         challenge.last_solution_hash,
				                         challenge.hash_prefix,
				                         challenge.parameters,
				                         challenge.cancellable);

				if (nonce == null)
				{
					message ("Could not solve challenge #%d, waiting until the next one...", challenge.challenge_id);
				}
				else
				{
					message ("Solved challenge #%lld in %lldms (%lds was given) with nonce '%s'.",
					         challenge.challenge_id,
					         (get_monotonic_time () - started) / 1000,
					         challenge.time_left,
					         nonce);

					message ("Submitting nonce '%s' for challenge #%lld to authority...", nonce, challenge.challenge_id);
					ws.send_text (generate_command ("submission", wallet_id: wallet.get_wallet_id (),
					                                              nonce:     nonce.to_string ()));
				}
			}
			catch (IOError.CANCELLED err)
			{
				message ("Challenge #%lld have been cancelled, waiting until the next one...", challenge.challenge_id);
			}
			catch (Error err)
			{
				critical ("%s (%s, %d)", err.message, err.domain.to_string (), err.code);
			}
		}, 1, true);

		Challenge? current_challenge = null;

		do
		{
			message ("Establishing a connection with the authority...");

			try
			{
				ws = yield ws_session.websocket_connect_async (new Soup.Message ("GET", ws_url), null, null, null);
			}
			catch (Error err)
			{
				critical (err.message);
				Idle.add (loop.callback);
				yield;
				continue;
			}

			message ("Established a connection with the authority!");

			ws.message.connect ((type, payload) => {
				var response = Json.from_string ((string) payload.get_data ()).get_object ();

				if (response.has_member ("challenge_id"))
				{
					var challenge = new Challenge.from_json_object (response, new Cancellable ());

					if (current_challenge != null)
					{
						if (challenge.challenge_id == current_challenge.challenge_id)
						{
							message ("Challenge #%lld is already executing.", current_challenge.challenge_id);
							return;
						}
						else
						{
							/* cancel any running challenge */
							current_challenge.cancellable.cancel ();
						}
					}

					try
					{
						challenge_executor.add (challenge);
					}
					catch (ThreadError err)
					{
						critical ("Could not enqueue the challenge #%lld: %s", challenge.challenge_id, err.message);
						return;
					}

					Timeout.add_seconds (challenge.time_left, () => {
						challenge.cancellable.cancel ();
						return false;
					});

					current_challenge = challenge;
				}
				else if (response.has_member ("error"))
				{
					critical ("Received an error from CA: %s.", response.get_string_member ("error"));
				}
			});

			ws.closing.connect (() => {
				warning ("The connection is closing, cancelling any running challenges...");
			});

			ws.closed.connect (() => {
				message ("The connection is closed and will be reestablished in a few...");
				Idle.add (loop.callback);
			});

			ws.error.connect ((err) => {
				critical (err.message);
			});

			ws.send_text (generate_command ("register_wallet", name:      "AEDIROUM",
			                                                   key:       wallet.get_key (),
			                                                   signature: wallet.get_signature ()));

			ws.send_text (generate_command ("get_current_challenge"));

			yield;
		}
		while (true);
	}
}

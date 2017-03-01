using GLib;

[CCode (lower_case_cprefix = "cscoin_")]
namespace CSCoin
{
	extern string solve_challenge (int    challenge_id,
	                               string challenge_name,
	                               string last_solution_hash,
	                               string hash_prefix,
	                               int    nb_elements);

	Json.Node generate_command (string command_name, ...)
	{
		var builder = new Json.Builder ();

		builder.begin_object ();
		builder.set_member_name ("command");
		builder.add_string_value (command_name);

		builder.set_member_name ("args");
		builder.begin_array ();
		// TODO: add command arguments
		builder.end_array ();

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

		var wallet_id = "";

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

			var challenge_executor = new ThreadPool<Json.Object>.with_owned_data ((challenge) => {
				var time_left = challenge.get_int_member ("time_left");
				var challenge_id = challenge.get_int_member ("challenge_id");

				message ("Received challenge #%lld.", challenge_id);

				var started = get_monotonic_time ();
				var nonce = solve_challenge ((int) challenge_id,
				                             challenge.get_string_member ("challenge_type"),
				                             challenge.get_string_member ("last_hash_solution"),
				                             challenge.get_string_member ("hash_prefix"),
				                             (int) challenge.get_object_member ("args").get_int_member ("nb_elements"));

				message ("Solved with nonce '%s' in %lldms (%lds was given).", nonce,
				                                                               (get_monotonic_time () - started) / 1000,
				                                                               time_left);

				message ("Submitting nonce '%s' for challenge %lld to authority...", nonce, challenge_id);
				ws.send_text (Json.to_string (generate_command ("submission", wallet_id: wallet_id, nonce: nonce), false));
				// TODO: submit nonce to authority
			}, -1, false);

			ws.send_text (Json.to_string (generate_command ("get_current_challenge"), false));

			ws.message.connect ((type, payload) => {
				var response = Json.from_string ((string) payload.get_data ()).get_object ();
				if (response.has_member ("challenge_id"))
				{
					try
					{
						challenge_executor.add (response);
						challenge_executor.move_to_front (response);
					}
					catch (ThreadError err)
					{
						critical ("Could not enqueue the challenge #%lld: %s", response.get_int_member ("challenge_id"), err.message);
					}
				}
				else if (response.has_member ("error"))
				{
					warning (response.get_string_member ("error"));
				}
				else
				{
					// TODO: handle other responses...
				}
			});
		});

		new MainLoop ().run ();

		return 0;
	}
}

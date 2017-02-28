using GLib;

[CCode (lower_case_cprefix = "cscoin_")]
namespace CSCoin
{
	extern string solve_challenge (int    challenge_id,
	                               string challenge_name,
	                               string last_solution_hash,
	                               string hash_prefix,
	                               int    nb_elements);

	int main (string[] args)
	{
		var nonce = solve_challenge (0,
		                             "sorted_list",
		                             "0000000000000000000000000000000000000000000000000000000000000000",
		                             "94f9",
		                             2000);

		message ("solved! nonce: %s", nonce);

		return 0;
	}
}

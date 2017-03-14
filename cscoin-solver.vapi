[CCode (lower_case_cprefix = "cscoin_", cheader_filename = "cscoin-solver.h")]
namespace CSCoin
{
	public struct ChallengeParameters
	{
		[CCode (cname = "sorted_list.nb_elements")]
		public int nb_elements;
		[CCode (cname = "shortest_path.grid_size")]
		public int grid_size;
		[CCode (cname = "shortest_path.nb_blockers")]
		public int nb_blockers;
	}

	public string solve_challenge (int                 challenge_id,
	                               string              challenge_name,
	                               string              last_solution_hash,
	                               string              hash_prefix,
	                               ChallengeParameters parameters,
	                               GLib.Cancellable?   cancellable = null) throws GLib.Error;
}

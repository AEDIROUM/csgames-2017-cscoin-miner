using GLib;

extern struct CSCoin.Parameters
{
	public int nb_elements;
}

extern string cscoin_solve_challenge (int               challenge_id,
                                      string            challenge_name,
                                      string            last_solution_hash,
                                      string            hash_prefix,
                                      CSCoin.Parameters parameters,
                                      Cancellable?      cancellable = null) throws Error;

void main ()
{
	var last_solution_hash = Checksum.compute_for_string (ChecksumType.SHA256, "test");
	for (var i = 0; i < 20; i++)
	{
		cscoin_solve_challenge (0, "sorted_list", last_solution_hash, i.to_string ("%04x"), CSCoin.Parameters () {nb_elements = 20});
	}
}

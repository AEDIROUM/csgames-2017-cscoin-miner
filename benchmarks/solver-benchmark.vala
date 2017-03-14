using GLib;

void main ()
{
	var last_solution_hash = Checksum.compute_for_string (ChecksumType.SHA256, "test");
	for (var i = 0; i < 20; i++)
	{
		CSCoin.solve_challenge (0, CSCoin.ChallengeType.SORTED_LIST, last_solution_hash, i.to_string ("%04x"), CSCoin.ChallengeParameters () {nb_elements = 20});
	}
}

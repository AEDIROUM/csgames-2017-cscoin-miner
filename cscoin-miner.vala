using GLib;

extern void init_genrand64 (uint64 seed);
extern uint64 genrand64_int64 ();

string solve_challenge (int    challenge_id,
                        string challenge_name,
                        string last_solution_hash,
                        string hash_prefix,
                        int    nb_elements)
{
	var nonce = 0;

	while (true)
	{
		var seed_hash = Checksum.compute_for_string (ChecksumType.SHA256, last_solution_hash + nonce.to_string ());
		var seed = uint64.parse ("0x%s%s%s%s%s%s%s%s".printf (seed_hash[14:16],
		                                                      seed_hash[12:14],
		                                                      seed_hash[10:12],
		                                                      seed_hash[8:10],
		                                                      seed_hash[6:8],
		                                                      seed_hash[4:6],
		                                                      seed_hash[2:4],
		                                                      seed_hash[0:2]));

		init_genrand64 (seed);

		var numbers = new uint64[nb_elements];

		for (var i = 0; i < nb_elements; i++)
		{
			numbers[i] = genrand64_int64 ();
		}

		switch (challenge_name)
		{
			case "sorted_list":
				Posix.qsort (numbers, numbers.length, sizeof (uint64), (a, b) => {
					return *(uint64*) a < *(uint64*) b ? -1 : 1;
				});
				break;
			case "reverse_sorted_list":
				Posix.qsort (numbers, numbers.length, sizeof (uint64), (a, b) => {
					return *(uint64*) a < *(uint64*) b ? 1 : -1;
				});
				break;
			default:
				assert_not_reached ();
		}

		var solution_checksum = new Checksum (ChecksumType.SHA256);

		for (var i = 0; i < nb_elements; i++)
		{
			var nb = "%llu".printf (numbers[i]);
			solution_checksum.update (nb.data, nb.length);
		}

		uint8 solution_digest[32];
		size_t solution_digest_len = 32;
		solution_checksum.get_digest (solution_digest, ref solution_digest_len);

		if (hash_prefix == "%x%x".printf (solution_digest[0], solution_digest[1])) {
			break;
		} else {
			nonce++;
		}
	}

	return nonce.to_string ();
}

int main (string[] args)
{
	 var nonce = solve_challenge (0,
	                              "sorted_list",
	                              "0000000000000000000000000000000000000000000000000000000000000000",
	                              "94f9",
	                              20);

	message ("solved! nonce: %s", nonce);

	return 0;
}

public class CSCoin.Challenge : GLib.Object
{
	public int                 time_left          { get; construct; }
	public int                 challenge_id       { get; construct; }
	public ChallengeType       challenge_type     { get; construct; }
	public string              last_solution_hash { get; construct; }
	public string              hash_prefix        { get; construct; }
	public ChallengeParameters parameters         { get; construct; }
	public Cancellable         cancellable        { get; construct; }

	public Challenge.from_json_object (Json.Object node, Cancellable cancellable)
	{
		var parameters = node.get_member ("parameters").get_object ();
		var challenge_parameters = ChallengeParameters ();
		var challenge_type = ((EnumClass) typeof (ChallengeType).class_ref ()).get_value_by_nick (node.get_string_member ("challenge_name")).value;

		switch (challenge_type)
		{
			case ChallengeType.SORTED_LIST:
			case ChallengeType.REVERSE_SORTED_LIST:
				challenge_parameters.nb_elements = (int) parameters.get_int_member ("nb_elements");
				break;
			case ChallengeType.SHORTEST_PATH:
				challenge_parameters.grid_size   = (int) parameters.get_int_member ("grid_size");
				challenge_parameters.nb_blockers = (int) parameters.get_int_member ("nb_blockers");
				break;
		}

		base (
			time_left:          (int) node.get_int_member ("time_left"),
			challenge_id:       (int) node.get_int_member ("challenge_id"),
			challenge_type:     challenge_type,
			last_solution_hash: node.get_string_member ("last_solution_hash"),
			hash_prefix:        node.get_string_member ("hash_prefix"),
			parameters:         challenge_parameters,
			cancellable:        cancellable);
	}
}

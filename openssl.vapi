[CCode (cprefix = "")]
namespace OpenSSL
{
	[Compact]
	[CCode (cname = "struct bignum_st", lower_case_cprefix = "BN_", cheader_filename = "openssl/bn.h")]
	public class Bignum
	{
		public Bignum ();
		public void set_word (ulong w);
		public string bn2dec ();
	}

	[Compact]
	[CCode (lower_case_cprefix = "RSA_", cheader_filename = "openssl/rsa.h")]
	public class RSA
	{
		public Bignum e;
		public Bignum d;
		public RSA ();
		public int generate_key_ex (int bits, Bignum e, void* callback = null);
		public bool sign (int type, uint8[] m);
		public bool verify (int type, uint8[] m, uint8[] sig_buf);
	}
}

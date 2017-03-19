[CCode (cprefix = "")]
namespace OpenSSL
{
	[CCode (cprefix = "NID_", cheader_filename = "openssl/objects.h")]
	public enum NID
	{
		sha256
	}

	[Compact]
	[CCode (cname = "BIO_METHOD")]
	public class BIOMethod
	{

	}

	[Compact]
	[CCode (lower_case_cprefix = "BIO_", cheader_filename = "openssl/bio.h")]
	public class BIO
	{
		public static unowned BIOMethod s_mem ();
		public BIO (BIOMethod type);
		public int read (uint8[] buf);
	}

	[Compact]
	[CCode (lower_case_cprefix = "RSA_", cheader_filename = "openssl/rsa.h")]
	public class RSA
	{
		public int size ();
		[CCode (instance_pos = 5)]
		public bool sign (int type, uint8[] m, [CCode (array_length = false)] uint8[] sigret, out int siglen);
	}

	[CCode (lower_case_cprefix = "PEM_")]
	namespace PEM
	{
		[CCode (cname = "pem_password_cb")]
		public delegate int PasswordCallback (uint8[] buf, int flag);
		public void read_RSAPrivateKey (GLib.FileStream f, out RSA x, PasswordCallback? cb = null);
		public bool write_bio_RSAPublicKey (BIO bp, RSA x);
	}

	[CCode (cname = "i2d_RSA_PUBKEY")]
	public int i2d_RSA_PUBKEY (RSA rsa, [CCode (array_length = false)] out uchar[] pp);
}

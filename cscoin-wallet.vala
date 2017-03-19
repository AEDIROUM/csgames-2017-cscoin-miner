public class CSCoin.Wallet : GLib.Object
{
	private OpenSSL.RSA wallet;

	public Wallet.from_path (string wallet_path) throws Error
	{
		FileStream wallet_fs;
		if ((wallet_fs = FileStream.open (wallet_path, "r")) == null)
		{
			throw new IOError.FAILED ("Could not oppen the wallet at '%s'.", wallet_path);
		}

		OpenSSL.RSA wallet;
		OpenSSL.PEM.read_RSAPrivateKey (wallet_fs, out wallet);

		if (wallet == null)
		{
			throw new IOError.FAILED ("Could not parse the wallet at '%s'.", wallet_path);
		}

		this.wallet = (owned) wallet;
	}

	public string get_wallet_id ()
	{
		uchar[] wallet_der;
		var wallet_der_len = OpenSSL.i2d_RSA_PUBKEY (wallet, out wallet_der);
		wallet_der.length  = wallet_der_len;
		return Checksum.compute_for_data (ChecksumType.SHA256, wallet_der);
	}

	public string get_key ()
	{
		var pem_buffer = new OpenSSL.BIO (OpenSSL.BIO.s_mem ());
		OpenSSL.PEM.write_bio_RSAPublicKey (pem_buffer, wallet);
		uint8 pem_str[4096];
		pem_buffer.read (pem_str);
		return (string) pem_str;
	}

	public string get_signature ()
	{
		/* generate a wallet signature */
		var sig = new uint8[wallet.size ()];

		uchar[] wallet_der;
		var wallet_der_len = OpenSSL.i2d_RSA_PUBKEY (wallet, out wallet_der);
		wallet_der.length  = wallet_der_len;

		var checksum = new Checksum (ChecksumType.SHA256);
		checksum.update (wallet_der, wallet_der_len);
		uint8 message_digest[32];
		size_t message_digest_len = 32;
		checksum.get_digest (message_digest, ref message_digest_len);

		int sig_len;
		assert (wallet.sign (OpenSSL.NID.sha256, message_digest, sig, out sig_len));

		var sig_hex = new StringBuilder ();
		for (var i = 0; i < sig_len; i++)
		{
			sig_hex.append_printf ("%02x", sig[i]);
		}

		return sig_hex.str;
	}
}

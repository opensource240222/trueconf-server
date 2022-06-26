#pragma once

enum class dh_oid {
	//DH1536,	no encryption algorithms provided for this group
	DH1024,		// most desirable
	//DHdummy	// do not use weak algs
};

enum class encryption_mode{
	no_encryption = 100,
	AES_128CBC = 0,			// most desirable
	//AES_128EOFB			// less desirable
	// do not use weak algs
	/*TripleDES_168CBC,
	TripleDES_168EOFB_64,
	DES_56CBC,
	DES_56EOFB_64*/
};

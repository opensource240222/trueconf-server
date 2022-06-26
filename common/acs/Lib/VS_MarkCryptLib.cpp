
#include "VS_MarkCryptLib.h"

unsigned char   encrypt_table[256] = { 0 }, decrypt_table[256] = { 0 },
				rnd1[64] = { 23,10,26,30,27,23,9,30,4,2,25,16,19,30,2,28,20,10,3,16,7,9,29,17,21,3,15,12,15,25,16,12,
							13,13,2,17,22,10,21,28,17,18,1,14,8,13,27,15,2,18,22,24,27,27,9,27,20,23,26,24,27,29,29,16 },
				rnd2[64] = { 29,14,29,21,4,20,18,13,30,17,18,17,8,12,11,1,25,6,2,12,18,2,6,4,20,14,19,22,14,21,12,26,
							17,1,16,25,22,0,23,17,29,28,25,9,12,15,1,20,0,24,24,7,4,1,21,1,11,22,25,13,31,20,9,15 };

bool   flagInstall = false;
bool VS_MarkCryptInstall( void )
{
	if (flagInstall)	return flagInstall;
	unsigned char   table_h[] = VS_MARK_ENCRYPT_TABLE, table_a[32];
	if (sizeof(table_h) != 32)		return false;
	unsigned long i, j;
	for (i = 0, j = 'Z'; j >= 'A'; ++i, --j)	table_a[i] = (unsigned char)j;
	for (j = 'l'; i < 32; ++i, ++j)		table_a[i] = (unsigned char)j;
	for (i = 0; i < 32; ++i)	encrypt_table[i] = table_a[table_h[i]];
	for (i = 0; i < 32; ++i)	decrypt_table[encrypt_table[i]] = (unsigned char)i;
	return flagInstall = true;
}
// end VS_MarkCryptInstall

bool VS_MarkEncrypt( unsigned char crypt[], const unsigned char src[] )
{
	if (!flagInstall)	return false;
//	unsigned char   src32[32];
	unsigned long i, j;
	for (i = j = 0; i < VS_MARK_SRC_LEN; ++i)
	{
		unsigned char   b[10], c[16];
		b[0] = *( src + i );								++i;
		b[1] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[2] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[3] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[4] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[5] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[6] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[7] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[8] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[9] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		b[10] = (i < VS_MARK_SRC_LEN) ? *( src + i ) : 0;	++i;
		c[0] = (b[0] >> 3);
/*		c[1] =
		c[2] =
		c[3] =
		c[4] =
		c[5] =
		c[6] =
		c[7] =
		c[8] =
		c[9] =
		c[10] =
		c[11] =
		c[12] =
		c[13] =
		c[14] =
		c[15] =		*/
	}
	static unsigned long   k = 0;
	if (++k > 1000000)	k = 1;
	crypt[0] = (unsigned char)((double)k*(255.*1.618033988735));
	crypt[0] &= 31;
	crypt[0] = 31 - crypt[0];
	unsigned long   offset1 = crypt[0];
	crypt[1] = (unsigned char)((double)(k+1423156)*(255.*1.618033988735));
	crypt[1] &= 31;
	unsigned long   offset2 = crypt[1];
	for (i = 2; i < VS_MARK_CRYPT_LEN - VS_MARK_SRC_LEN; ++i)	crypt[i] = rnd2[i+offset2];
	for (j = 0; i < VS_MARK_CRYPT_LEN; ++i, ++j)	crypt[i] = src[j];
	crypt[0] = encrypt_table[crypt[0]];
	crypt[1] = encrypt_table[crypt[1]];
	for (i = 2; i < VS_MARK_CRYPT_LEN; ++i)
		crypt[i] = encrypt_table[(((int)32+crypt[i]-rnd1[i+offset1])&31)];
	return true;
}
// end VS_MarkEncrypt

bool VS_MarkDecrypt( const unsigned char crypt[], unsigned char src[] )
{
	if (!flagInstall)	return false;
	unsigned long   offset1 = decrypt_table[crypt[0]],
					offset2 = decrypt_table[crypt[1]];
	unsigned long i, j;
	for (i = 2; i < VS_MARK_CRYPT_LEN - VS_MARK_SRC_LEN; ++i)
		if (decrypt_table[(int)(((int)crypt[i])+(int)rnd1[i+offset1])&31] != rnd2[i+offset2])
			return false;
	for (j = 0; i < VS_MARK_CRYPT_LEN; ++i, ++j)
		src[j] = decrypt_table[(int)(((int)crypt[i])+(int)rnd1[i+offset1])&31];
	return true;
}
// end VS_MarkDecrypt

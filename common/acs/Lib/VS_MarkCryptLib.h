
#ifndef VS_MARK_CRYPT_LIB_H
#define VS_MARK_CRYPT_LIB_H

#ifndef VS_MARK_CRYPT_LEN
#define   VS_MARK_CRYPT_LEN   32
#endif // VS_MARK_CRYPT_LEN

#ifndef VS_MARK_SRC_LEN
#define   VS_MARK_SRC_LEN   16
#endif // VS_MARK_SRC_LEN

#ifndef VS_MARK_ENCRYPT_TABLE
#define   VS_MARK_ENCRYPT_TABLE   {	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,\
									19,20,21,22,23,24,25,26,27,28,29,30,31 }
#endif // VS_MARK_ENCRYPT_TABLE

#if ((VS_MARK_CRYPT_LEN < 26) || (VS_MARK_CRYPT_LEN > 32))
#error " !!!!! ((VS_MARK_CRYPT_LEN < 16) || (VS_MARK_CRYPT_LEN > 32)) !!!!! "
#endif // ((VS_MARK_CRYPT_LEN < 16) || (VS_MARK_CRYPT_LEN > 32))

#if ((VS_MARK_CRYPT_LEN < (VS_MARK_SRC_LEN + 16)))
#error " !!!!! ((VS_MARK_CRYPT_LEN < (VS_MARK_SRC_LEN + 16)) !!!!! "
#endif // ((VS_MARK_CRYPT_LEN <= (VS_MARK_SRC_LEN + 16))

bool VS_MarkCryptInstall( void );
bool VS_MarkEncrypt( unsigned char crypt[], const unsigned char src[] );
bool VS_MarkDecrypt( const unsigned char crypt[], unsigned char src[] );

#endif // VS_MARK_CRYPT_LIB_H

/**
 **************************************************************************
 * \file VS_SymmetricCrypt.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Stream crypter
 *
 * \b Project Client
 * \author SmirnovK, Matvey
 * \date 25.03.2009
 *
 * $Revision: 1 $
 *
 * $History: VS_StreamCrypter.h $
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 11.11.11   Time: 17:32
 * Created in $/VSNA/SecureLib
 * - added stream crypter in stream router
 * - bugfix#9470
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 31.03.09   Time: 18:15
 * Updated in $/VSNA/VSClient
 * - stream symmetric crypt support
 *
 ****************************************************************************/
#ifndef VS_SYMMETRIC_CRYPT_H
#define VS_SYMMETRIC_CRYPT_H

#include <stdint.h>

class VS_SymmetricCrypt;

class VS_StreamCrypter
{
protected:
	VS_SymmetricCrypt*	m_crypter;
	bool				m_isInit;
public:
	VS_StreamCrypter();
	~VS_StreamCrypter();
	bool Init(const unsigned char* key, uint32_t key_len);
	bool Init(const char *key);
	void Free();
    unsigned long GetBlockSize() const;
	bool IsValid() const {return m_isInit;}
	bool Encrypt(const unsigned char *buf, uint32_t buf_sz, unsigned char* encr_buf, uint32_t *encr_buf_sz);
	bool Decrypt(const unsigned char *buf, uint32_t buf_sz, unsigned char* decr_buf, uint32_t *decr_buf_sz);

	VS_StreamCrypter(const VS_StreamCrypter&) = delete;
	VS_StreamCrypter& operator=(const VS_StreamCrypter&) = delete;

	VS_StreamCrypter(VS_StreamCrypter&&);
	VS_StreamCrypter& operator=(VS_StreamCrypter&&);
};

#endif /*VS_SYMMETRIC_CRYPT_H*/
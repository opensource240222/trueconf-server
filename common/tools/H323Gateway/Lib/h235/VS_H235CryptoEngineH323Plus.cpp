#include "VS_H235CryptoEngineH323Plus.h"

#include <openssl/rand.h>
#include <openssl/modes.h>

#include <cassert>
#include <cstring>

// the IV sequence is always 6 bytes long (2 bytes seq number + 4 bytes timestamp) see the H.235 v3 specification, B.3.1.1 CBC initialization vector
const unsigned int IV_SEQUENCE_LEN = 6;

H235CryptoEngineH323Plus::H235CryptoEngineH323Plus(const encryption_mode mode, const std::vector<uint8_t> & key):
	  m_encryptCtx(EVP_CIPHER_CTX_new()),
	  m_decryptCtx(EVP_CIPHER_CTX_new()),
	  m_initialised(false),
      m_mode(mode)

   /*m_inSize(0), m_outSize(0),
   m_enc_blockSize(0), m_enc_ivLength(0), m_dec_blockSize(0), m_dec_ivLength(0)*/
{
    SetKey(key);
}

H235CryptoEngineH323Plus::~H235CryptoEngineH323Plus()
{
    if (m_initialised) {
        EVP_CIPHER_CTX_cleanup(m_encryptCtx);
        EVP_CIPHER_CTX_cleanup(m_decryptCtx);
    }
	EVP_CIPHER_CTX_free(m_encryptCtx);
	EVP_CIPHER_CTX_free(m_decryptCtx);
}

void H235CryptoEngineH323Plus::SetKey(std::vector<uint8_t> key)
{
    m_currentCipher = NULL;

	switch (m_mode)
	{
	case encryption_mode::no_encryption:
		assert(false);
		return;
	case encryption_mode::AES_128CBC:
		m_currentCipher = EVP_aes_128_cbc();
		break;
	default:
		printf("H235\tUnsupported mode\n");
		assert(false);
		return;
	}

	if (key.empty()){
		printf("H235\tError empty key!\n");
		return;
	}

    EVP_CIPHER_CTX_init(m_encryptCtx);
    EVP_EncryptInit_ex(m_encryptCtx, m_currentCipher, NULL, key.data(), NULL);
    m_enc_blockSize =  EVP_CIPHER_CTX_block_size(m_encryptCtx);
    m_enc_ivLength = EVP_CIPHER_CTX_iv_length(m_encryptCtx);

    EVP_CIPHER_CTX_init(m_decryptCtx);
	EVP_DecryptInit_ex(m_decryptCtx, m_currentCipher, NULL, key.data(), NULL);
    m_dec_blockSize =  EVP_CIPHER_CTX_block_size(m_decryptCtx);
    m_dec_ivLength = EVP_CIPHER_CTX_iv_length(m_decryptCtx);

	AES_set_encrypt_key(key.data(), key.size() * 8, &m_encryptKey);
	AES_set_decrypt_key(key.data(), key.size() * 8, &m_decryptKey);

    m_initialised = true;
}

void H235CryptoEngineH323Plus::SetIV(unsigned char * iv, const unsigned char * ivSequence, unsigned ivLen)
{
    // fill iv by repeating ivSequence until block size is reached
    if (ivSequence) {
        for (unsigned i = 0; i < (ivLen / IV_SEQUENCE_LEN); i++) {
            memcpy(iv + (i * IV_SEQUENCE_LEN), ivSequence, IV_SEQUENCE_LEN);
        }
        // copy partial ivSequence at end
        if (ivLen % IV_SEQUENCE_LEN > 0) {
            memcpy(iv + ivLen - (ivLen % IV_SEQUENCE_LEN), ivSequence, ivLen % IV_SEQUENCE_LEN);
        }
    } else {
        memset(iv, 0, ivLen);
    }
}

std::vector<uint8_t> H235CryptoEngineH323Plus::Encrypt(const uint8_t * _data, const size_t _data_size, const unsigned char * ivSequence, bool & rtpPadding)
{
	if (!m_initialised || !_data)
        return std::vector<uint8_t>();

    unsigned char iv[EVP_MAX_IV_LENGTH];

    // max ciphertext len for a n bytes of plaintext is n + BLOCK_SIZE -1 bytes
	int ciphertext_len = _data_size + EVP_CIPHER_CTX_block_size(m_encryptCtx);
    int final_len = 0;
    std::vector<uint8_t> ciphertext(ciphertext_len);

    SetIV(iv, ivSequence, EVP_CIPHER_CTX_iv_length(m_encryptCtx));
    EVP_EncryptInit_ex(m_encryptCtx, NULL, NULL, NULL, iv);

    // always use padding, because our ciphertext stealing implementation
    // doesn't seem to produce compatible results
    //rtpPadding = (data.GetSize() < EVP_CIPHER_CTX_block_size(m_encryptCtx));  // not interoperable!
	rtpPadding = (_data_size % EVP_CIPHER_CTX_block_size(m_encryptCtx) > 0);
    EVP_CIPHER_CTX_set_padding(m_encryptCtx, rtpPadding ? 1 : 0);

	if (!rtpPadding && (_data_size % EVP_CIPHER_CTX_block_size(m_encryptCtx) > 0)) {
        // use cyphertext stealing
		if (!CRYPTO_cts128_encrypt(_data, ciphertext.data(), _data_size, &m_encryptKey, iv,
			   reinterpret_cast<cbc128_f>(AES_cbc_encrypt))) {
            //PTRACE(1, "H235\tEVP_EncryptUpdate_cts() failed");
        }
    } else {
        /* update ciphertext, ciphertext_len is filled with the length of ciphertext generated,
         *len is the size of plaintext in bytes */
		if (!EVP_EncryptUpdate(m_encryptCtx, ciphertext.data(), &ciphertext_len, _data, _data_size)) {
            //PTRACE(1, "H235\tEVP_EncryptUpdate() failed");
        }

        // update ciphertext with the final remaining bytes, if any use RTP padding
        if (!EVP_EncryptFinal_ex(m_encryptCtx, ciphertext.data() + ciphertext_len, &final_len)) {
            //PTRACE(1, "H235\tEVP_EncryptFinal_ex() failed");
        }
    }

    ciphertext.resize(ciphertext_len + final_len);
    return ciphertext;
}

//PINDEX H235CryptoEngineH323Plus::EncryptInPlace(const BYTE * inData, PINDEX inLength, BYTE * outData, unsigned char * ivSequence, bool & rtpPadding)
//{
//    if (!m_initialised) {
//        //PTRACE(1, "H235\tERROR: Encryption not initialised!!");
//        memset(outData,0,inLength);
//        return inLength;
//    }
//
//    // max ciphertext len for a n bytes of plaintext is n + BLOCK_SIZE -1 bytes
//    m_outSize = 0;
//    m_inSize =  inLength + m_enc_blockSize;
//
//    SetIV(m_iv, ivSequence, m_enc_ivLength);
//    EVP_EncryptInit_ex(&m_encryptCtx, NULL, NULL, NULL, m_iv);
//
//    rtpPadding = (inLength % m_enc_blockSize > 0);
//    EVP_CIPHER_CTX_set_padding(&m_encryptCtx, rtpPadding ? 1 : 0);
//
//    if (!rtpPadding && (inLength % m_enc_blockSize > 0)) {
//        // use cyphertext stealing
//        if (!EVP_EncryptUpdate_cts(&m_encryptCtx, outData, &m_inSize, inData, inLength)) {
//            //PTRACE(1, "H235\tEVP_EncryptUpdate_cts() failed");
//        }
//#if PTLIB_VER >= 2130
//        if (!EVP_EncryptFinal_ctsA(&m_encryptCtx, outData + m_inSize, &m_outSize)) {
//#else
//        if (!EVP_EncryptFinal_cts(&m_encryptCtx, outData + m_inSize, &m_outSize)) {
//#endif
//            //PTRACE(1, "H235\tEVP_EncryptFinal_cts() failed");
//        }
//    } else {
//        /* update ciphertext, ciphertext_len is filled with the length of ciphertext generated,
//         *len is the size of plaintext in bytes */
//        if (!EVP_EncryptUpdate(&m_encryptCtx, outData, &m_inSize, inData, inLength)) {
//            //PTRACE(1, "H235\tEVP_EncryptUpdate() failed");
//        }
//
//        // update ciphertext with the final remaining bytes, if any use RTP padding
//        if (!EVP_EncryptFinal_ex(&m_encryptCtx, outData + m_inSize, &m_outSize)) {
//            //PTRACE(1, "H235\tEVP_EncryptFinal_ex() failed");
//        }
//    }
//    return m_inSize + m_outSize;
//}

std::vector<uint8_t> H235CryptoEngineH323Plus::Decrypt(const uint8_t * _data, const size_t _data_size, const unsigned char * ivSequence, bool & rtpPadding)
{
	if (!m_initialised || !_data)
        return std::vector<uint8_t>();

    unsigned char iv[EVP_MAX_IV_LENGTH];

    /* plaintext will always be equal to or lesser than length of ciphertext*/
	int plaintext_len = _data_size;
    int final_len = 0;
    std::vector<uint8_t> plaintext(plaintext_len);

    SetIV(iv, ivSequence, EVP_CIPHER_CTX_iv_length(m_decryptCtx));
    EVP_DecryptInit_ex(m_decryptCtx, NULL, NULL, NULL, iv);

    EVP_CIPHER_CTX_set_padding(m_decryptCtx, rtpPadding ? 1 : 0);

	if (!rtpPadding && _data_size % EVP_CIPHER_CTX_block_size(m_decryptCtx) > 0) {
        // use cyphertext stealing
		if (!CRYPTO_cts128_decrypt(_data, plaintext.data(), _data_size, &m_decryptKey, iv,
				reinterpret_cast<cbc128_f>(AES_cbc_encrypt))) {
            //PTRACE(1, "H235\tEVP_DecryptUpdate_cts() failed");
        }
    } else {
		if (!EVP_DecryptUpdate(m_decryptCtx, plaintext.data(), &plaintext_len, _data, _data_size)) {
            //PTRACE(1, "H235\tEVP_DecryptUpdate() failed");
        }
        if (!EVP_DecryptFinal(m_decryptCtx, plaintext.data() + plaintext_len, &final_len)) {
            //PTRACE(1, "H235\tEVP_DecryptFinal_ex() failed - incorrect padding ?");
        }
    }

	rtpPadding = false;	// we return the real length of the decrypted data without padding
    plaintext.resize(plaintext_len + final_len);
    return plaintext;
}

//PINDEX H235CryptoEngineH323Plus::DecryptInPlace(const BYTE * inData, PINDEX inLength, BYTE * outData, unsigned char * ivSequence, bool & rtpPadding)
//{
//
//    /* plaintext will always be equal to or lesser than length of ciphertext*/
//    m_outSize = 0;
//    m_inSize =  inLength;
//
//    SetIV(m_iv, ivSequence, m_dec_ivLength);
//    EVP_DecryptInit_ex(&m_decryptCtx, NULL, NULL, NULL, m_iv);
//
//    EVP_CIPHER_CTX_set_padding(&m_decryptCtx, rtpPadding ? 1 : 0);
//
//    if (!rtpPadding && inLength % m_dec_blockSize > 0) {
//        // use cyphertext stealing
//        if (!EVP_DecryptUpdate_cts(&m_decryptCtx, outData, &m_inSize, inData, inLength)) {
//            //PTRACE(1, "H235\tEVP_DecryptUpdate_cts() failed");
//        }
//        if(!EVP_DecryptFinal_cts(&m_decryptCtx, outData + m_inSize, &m_outSize)) {
//            //PTRACE(1, "H235\tEVP_DecryptFinal_cts() failed");
//        }
//    } else {
//        if (!EVP_DecryptUpdate(&m_decryptCtx, outData, &m_inSize, inData, inLength)) {
//            //PTRACE(1, "H235\tEVP_DecryptUpdate() failed");
//        }
//        if (!EVP_DecryptFinal_relaxed(&m_decryptCtx, outData + m_inSize, &m_outSize)) {
//            //PTRACE(1, "H235\tEVP_DecryptFinal_ex() failed - incorrect padding ?");
//        }
//    }
//	rtpPadding = false;	// we return the real length of the decrypted data without padding
//    return m_inSize + m_outSize;
//}

std::vector<uint8_t> H235CryptoEngineH323Plus::GenerateRandomKey()
{
    std::vector<uint8_t> result = GenerateRandomKey(m_mode);
    SetKey(result);
    return result;
}

std::vector<uint8_t> H235CryptoEngineH323Plus::GenerateRandomKey(const encryption_mode m)
{
    std::vector<uint8_t> key;
	switch (m)
	{
	case encryption_mode::AES_128CBC:
		key.resize(16);
		break;
	default:
		return key;
	}

    RAND_bytes(key.data(), key.size());

    return key;
}

///////////////////////////////////////////////////////////////////////////////////
#include "tools/H323Gateway/Lib/src/VS_H235Enumerations.h"
#include <cstdint>
#include <vector>
#include <openssl/evp.h>
#include <openssl/aes.h>

class H235CryptoEngineH323Plus
{
public:

 /**@name Constructor */
  //@{
    /** Create a H.235 crypto engine
     */
    H235CryptoEngineH323Plus(const encryption_mode mode, const std::vector<uint8_t> & key);

   /** Destroy the crypto engine
     */
    ~H235CryptoEngineH323Plus();
  //@}

    /** Set key (for key updates)
      */
    void SetKey(std::vector<uint8_t> key);

    /** Encrypt data
      */
	std::vector<uint8_t> Encrypt(const uint8_t * _data, const size_t _data_size, const unsigned char * ivSequence, bool & rtpPadding);

    /** Decrypt data
      */
	std::vector<uint8_t> Decrypt(const uint8_t * _data, const size_t _data_size, const unsigned char * ivSequence, bool & rtpPadding);

    /** Generate a random key of a size suitable for the alogorithm
      */
    std::vector<uint8_t> GenerateRandomKey();   // Use internal Algorithm and set

	std::vector<uint8_t> GenerateRandomKey(const encryption_mode m);  // Use assigned Algorithm

	//PString GetAlgorithmOID() const { return m_algorithmOID; }

 //   bool IsMaxBlocksPerKeyReached() const { return m_operationCnt > AES_KEY_LIMIT; }
 //   void ResetBlockCount() { m_operationCnt = 0; }

protected:
    static void SetIV(unsigned char * iv, const unsigned char * ivSequence, unsigned ivLen);

    EVP_CIPHER_CTX *m_encryptCtx, *m_decryptCtx;
	const EVP_CIPHER* m_currentCipher;
	AES_KEY m_encryptKey;
	AES_KEY m_decryptKey;
    bool m_initialised = false;
	encryption_mode m_mode = encryption_mode::no_encryption;

    //unsigned char m_iv[EVP_MAX_IV_LENGTH];
    //int m_inSize;
    //int m_outSize;

    int m_enc_blockSize = 0;
    int m_enc_ivLength = 0;
    int m_dec_blockSize = 0;
    int m_dec_ivLength = 0;
};
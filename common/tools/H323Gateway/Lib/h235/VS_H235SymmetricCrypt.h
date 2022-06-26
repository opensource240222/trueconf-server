#include "SecureLib/VS_SymmetricCrypt.h"
#include <cstdint>
#include <vector>
#include <openssl/evp.h>

class VS_H235SymmetricCrypt : public VS_SymmetricCrypt{
public:
	VS_H235SymmetricCrypt();
	bool SetIV(const uint8_t* iv, unsigned iv_size, bool forEncyptCtx);
	void SetPadding(bool padding, bool forEncyptCtx);
	bool DecryptCTS(const unsigned char *encr_buf, const uint32_t buf_len, unsigned char *decr_data, uint32_t &OUTdecr_len);
	bool DecryptRelaxed(const unsigned char *encr_buf, const uint32_t buf_len, unsigned char *decr_data, uint32_t &OUTdecr_len, bool padding);
	bool EncryptCTS(const unsigned char *data, const uint32_t data_len, unsigned char *OUTencr_buf, uint32_t &OUTbuf_len);
private:
	uint8_t m_IV[EVP_MAX_IV_LENGTH];
};
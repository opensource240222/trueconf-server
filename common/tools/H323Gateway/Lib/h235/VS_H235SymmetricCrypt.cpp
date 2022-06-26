#include "VS_H235SymmetricCrypt.h"

#include <openssl/aes.h>
#include <openssl/modes.h>

#include <algorithm>
#include <cassert>
#include <cstring>

VS_H235SymmetricCrypt::VS_H235SymmetricCrypt() :m_IV{} {}

void PrepareIV(uint8_t * OUT_iv, const uint8_t * ivSequence, const unsigned ivSequence_len,	const unsigned block_size)
{
	// fill iv by repeating ivSequence until block size is reached
	if (ivSequence) {
		for (unsigned i = 0; i < (block_size / ivSequence_len); ++i) {
			memcpy(OUT_iv + (i * ivSequence_len), ivSequence, ivSequence_len);
		}
		// copy partial ivSequence at end
		if (block_size % ivSequence_len > 0) {
			memcpy(OUT_iv + block_size - (block_size % ivSequence_len), ivSequence, block_size % ivSequence_len);
		}
	}
	else {
		memset(OUT_iv, 0, block_size);
	}
}

bool VS_H235SymmetricCrypt::SetIV(const uint8_t* iv, unsigned iv_size, bool forEncyptCtx){
	auto IV_size = GetBlockSize();
	IV_size = std::min(static_cast<uint32_t>(EVP_MAX_IV_LENGTH), IV_size);

	PrepareIV(m_IV, iv, iv_size, IV_size);

	int res(0);
	/* Initialise IV */
	if (forEncyptCtx)
		res = EVP_EncryptInit_ex(GetEncryptCtx<EVP_CIPHER_CTX*>(), nullptr, nullptr, nullptr, m_IV);
	else
		res = EVP_DecryptInit_ex(GetDecryptCtx<EVP_CIPHER_CTX*>(), nullptr, nullptr, nullptr, m_IV);

	return res == 1;
}

void VS_H235SymmetricCrypt::SetPadding(bool rtpPadding, bool forEncyptCtx){
	EVP_CIPHER_CTX* ctx = forEncyptCtx ? GetEncryptCtx<EVP_CIPHER_CTX*>() : GetDecryptCtx<EVP_CIPHER_CTX*>();
	EVP_CIPHER_CTX_set_padding(ctx, rtpPadding ? 1 : 0);
}

bool VS_H235SymmetricCrypt::DecryptCTS(const unsigned char * encr_buf, const uint32_t buf_len, unsigned char * OUTdecr_data, uint32_t & OUTdecr_len)
{
	auto decrCTX = GetDecryptCtx<EVP_CIPHER_CTX*>();
	assert(decrCTX != nullptr);

	std::vector<unsigned char> key(EVP_CIPHER_CTX_key_length(decrCTX));
	uint32_t keyLength;
	if (!GetKey(EVP_CIPHER_CTX_key_length(decrCTX), key.data(), &keyLength))
		return false;
	assert (keyLength == static_cast<uint32_t>(EVP_CIPHER_CTX_key_length(decrCTX)));
	AES_KEY decryptKey;
	AES_set_decrypt_key(key.data(), key.size() * 8, &decryptKey);
	if (!(OUTdecr_len = CRYPTO_cts128_decrypt(encr_buf, OUTdecr_data, buf_len, &decryptKey, m_IV,
		   reinterpret_cast<cbc128_f>(AES_cbc_encrypt))))
		return false;
	return true;
}
namespace
{

size_t VerifyPkcs7Padding(const unsigned char* decryptedData, uint32_t length, uint32_t blockSize)
{
	if (length % blockSize)
		return 0;
	uint8_t paddingLength;
	if ((paddingLength = decryptedData[length - 1]) == 0 || paddingLength > blockSize)
		return 0;
	for (uint8_t i = 1; i < paddingLength; ++i)
		if (decryptedData[length - i - 1] != paddingLength)
			return 0;
	return paddingLength;
}

size_t VerifyAnsiX923Padding(const unsigned char* decryptedData, uint32_t length, uint32_t blockSize)
{
	if (length % blockSize)
		return 0;
	uint8_t paddingLength;
	if ((paddingLength = decryptedData[length - 1]) == 0 || paddingLength > blockSize)
		return 0;
	// In the original code this piece was commented out, so I'm afraid someone did some shit somewhere
	// that will break everything later
	for (uint8_t i = 1; i < paddingLength; ++i)
		if (decryptedData[length - i - 1] != '\0')
			return 0;
	return paddingLength;
}
}

bool VS_H235SymmetricCrypt::DecryptRelaxed(const unsigned char * encr_buf, const uint32_t buf_len, unsigned char * OUTdecr_data, uint32_t & OUTdecr_len, bool padding)
{
	auto decrCTX = GetDecryptCtx<EVP_CIPHER_CTX*>();
	assert(decrCTX != nullptr);
	std::vector<unsigned char> key(EVP_CIPHER_CTX_key_length(decrCTX));
	uint32_t keyLength;
	if (!GetKey(EVP_CIPHER_CTX_key_length(decrCTX), key.data(), &keyLength))
		return false;
	assert (keyLength == static_cast<uint32_t>(EVP_CIPHER_CTX_key_length(decrCTX)));
	AES_KEY decryptKey;
	AES_set_decrypt_key(key.data(), key.size() * 8, &decryptKey);

	AES_cbc_encrypt(encr_buf, OUTdecr_data, buf_len, &decryptKey, m_IV, 0);
	OUTdecr_len = buf_len;

	if (padding)
	{
		size_t paddingSize = VerifyPkcs7Padding(OUTdecr_data, buf_len, EVP_CIPHER_CTX_block_size(decrCTX));
		if (paddingSize == 0)
			paddingSize = VerifyAnsiX923Padding(OUTdecr_data, buf_len, EVP_CIPHER_CTX_block_size(decrCTX));
		OUTdecr_len -= paddingSize;
	}

	return true;
}

bool VS_H235SymmetricCrypt::EncryptCTS(const unsigned char * data, const uint32_t data_len, unsigned char * OUTencr_buf, uint32_t &OUTbuf_len)
{
	auto encrCTX = GetEncryptCtx<EVP_CIPHER_CTX*>();
	assert(encrCTX != nullptr);

	std::vector<unsigned char> key(EVP_CIPHER_CTX_key_length(encrCTX));
	uint32_t keyLength;
	if (!GetKey(EVP_CIPHER_CTX_key_length(encrCTX), key.data(), &keyLength))
		return false;
	assert (keyLength == static_cast<uint32_t>(EVP_CIPHER_CTX_key_length(encrCTX)));
	AES_KEY encryptKey;
	AES_set_encrypt_key(key.data(), key.size() * 8, &encryptKey);
	if (!(OUTbuf_len = CRYPTO_cts128_encrypt(data, OUTencr_buf, data_len, &encryptKey, m_IV,
		   reinterpret_cast<cbc128_f>(AES_cbc_encrypt))))
		return false;
	return true;
}
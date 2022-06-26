#include "VS_H235CryptoEngine.h"
#include "VS_H235SymmetricCrypt.h"
#include "std-generic/cpplib/scope_exit.h"

#include <tuple>
#include <cassert>

std::tuple<VS_SymmetricAlg, VS_SymmetricCipherMode> FindCipherAndMode(const encryption_mode m){
	switch (m)
	{
	case encryption_mode::no_encryption:
		assert(false);
		return std::make_tuple(alg_sym_NONE, mode_NONE);
	case encryption_mode::AES_128CBC:
		return std::make_tuple(alg_sym_AES128, mode_CBC);
	/*case encryption_mode::AES_128EOFB:
		return std::make_tuple(alg_sym_AES128, mode_OFB);*/
	default:
		assert(false);
		return std::make_tuple(alg_sym_NONE, mode_NONE);
		break;
	}
}

bool VS_H235CryptoEngine::Init(const std::vector<uint8_t> &key, const encryption_mode emode){
	if (key.empty() || emode==encryption_mode::no_encryption)
		return false;

	VS_SymmetricAlg alg = alg_sym_NONE;
	VS_SymmetricCipherMode m = mode_NONE;
	std::tie(alg, m) = FindCipherAndMode(emode);
	return Init(key, alg, m);
}

bool VS_H235CryptoEngine::Init(const std::vector<uint8_t> &key, const VS_SymmetricAlg &alg, const VS_SymmetricCipherMode &m){
	if (key.empty())
		return false;
	Free();

	m_crypter = new VS_H235SymmetricCrypt;
	if (!m_crypter->Init(alg, m) || !m_crypter->SetKey(key.size(), key.data()))
	{
		Free();
		return false;
	}
	else
	{
		m_isInit = true;
		return true;
	}
}

VS_H235SymmetricCrypt* GetValidImpl(VS_SymmetricCrypt* impl){
	auto pImpl = dynamic_cast<VS_H235SymmetricCrypt*>(impl);
	assert(pImpl != nullptr);

	return pImpl;
}

std::tuple<bool/*res*/, bool /*padding was used*/> VS_H235CryptoEngine::Encrypt(const uint8_t *buf, const size_t buf_len, const uint8_t * iv, std::vector<uint8_t> &encr_buf, const bool use_cts, encryption_method *OUT_method) const{
	assert(m_isInit != false);
	if (!Valid()) return std::make_tuple(false, false);
	if (!buf || buf_len == 0) return std::make_tuple(false, false);

	auto pCrypter = GetValidImpl(m_crypter);
	if (!pCrypter)	return std::make_tuple(false, false);
	auto bsize = m_crypter->GetBlockSize();

	// max ciphertext len for a n bytes of plaintext is n + BLOCK_SIZE - 1 bytes
	int ciphertext_len = buf_len + bsize;
	uint32_t encr_size(ciphertext_len);
	bool not_padded = (buf_len % bsize > 0);
	encr_buf.resize(ciphertext_len);

	if (iv != nullptr) {
		if (!pCrypter->SetIV(iv, IV_SEQUENCE_LEN, true))  return std::make_tuple(false, false);
	}

	if (not_padded && use_cts) {
		if (OUT_method) *OUT_method = encryption_method::cts;
		if (pCrypter->EncryptCTS(buf, buf_len, encr_buf.data(), encr_size)) {
			encr_buf.resize(encr_size);
			return std::make_tuple(true, false);
		}
	}
	else {
		if (OUT_method) *OUT_method = encryption_method::padding;
		pCrypter->SetPadding(not_padded, true);
		if (pCrypter->Encrypt(buf, buf_len, encr_buf.data(), &encr_size)) {
			encr_buf.resize(encr_size);
			return std::make_tuple(true, not_padded);
		}
	}

	encr_buf.resize(0);
	return std::make_tuple(false, false);
}

std::tuple<bool/*res*/, bool /*padding was used*/> VS_H235CryptoEngine::Encrypt(const std::vector<uint8_t> &buf, const uint8_t * iv, std::vector<uint8_t> &encr_buf, const bool use_cts, encryption_method *OUT_method) const{
	return Encrypt(buf.data(), buf.size(), iv, encr_buf, use_cts, OUT_method);
}

bool VS_H235CryptoEngine::Decrypt(const uint8_t *buf, const size_t buf_len, const uint8_t * iv, const bool padding, std::vector<uint8_t> &decr_buf, encryption_method *OUTmethod) const{
	assert(m_isInit != false);
	if (!Valid()) return false;
	if (!buf || buf_len == 0) return false;

	auto pCrypter = GetValidImpl(m_crypter);
	if (!pCrypter)	return false;
	auto bsize = pCrypter->GetBlockSize();

	/**
	plaintext will always be equal to or lesser than length of ciphertext,
	but when padding is used buffer passed to EVP_DecryptUpdate() should have
	sufficient room for (input len + cipher_block_size)
	*/
	uint32_t plaintext_len = buf_len + bsize;
	uint32_t final_len = plaintext_len;

	if (iv != nullptr){
		if(!pCrypter->SetIV(iv, IV_SEQUENCE_LEN, false)) return false;
	}

	decr_buf.resize(plaintext_len);
	if (!padding && buf_len%bsize > 0){
		if (OUTmethod) *OUTmethod = encryption_method::cts;
		if (pCrypter->DecryptCTS(buf, buf_len, decr_buf.data(), final_len)) {
			assert(plaintext_len >= final_len);
			decr_buf.resize(final_len);
			return true;
		}
	}
	else {
		if (OUTmethod) *OUTmethod = encryption_method::padding;
		pCrypter->SetPadding(padding, false);
		if (pCrypter->DecryptRelaxed(buf, buf_len, decr_buf.data(), final_len, padding)) {
			assert(plaintext_len >= final_len);
			decr_buf.resize(final_len);
			return true;
		}
	}

	decr_buf.resize(0);
	return false;
}

bool VS_H235CryptoEngine::Decrypt(const std::vector<uint8_t> &buf, const uint8_t * iv, const bool padding, std::vector<uint8_t> &decr_buf, encryption_method *OUTmethod) const{
	return Decrypt(buf.data(), buf.size(), iv, padding, decr_buf, OUTmethod);
}

bool VS_H235CryptoEngine::Valid() const{
	return m_isInit;
}

std::vector<uint8_t> VS_H235CryptoEngine::GenerateKey(uint32_t key_size) const{
	assert(m_isInit != false);
	if (!Valid()) return std::vector<uint8_t>();

	auto pCrypter = GetValidImpl(m_crypter);
	if (!pCrypter)	return std::vector<uint8_t>();

	std::vector<uint8_t> key(key_size);
	if (pCrypter->GenerateKey(key_size, key.data()))
		return key;
	else
		return {};

}

std::vector<uint8_t> VS_H235CryptoEngine::GetKey() const{
	assert(m_isInit != false);
	if (!Valid()) return std::vector<uint8_t>();

	auto pCrypter = GetValidImpl(m_crypter);
	if (!pCrypter)	return std::vector<uint8_t>();

	std::vector<uint8_t> key(pCrypter->GetKeyLen());
	uint32_t key_len(0);
	if (pCrypter->GetKey(key.capacity(), key.data(), &key_len)){
		key.resize(key_len);
		return key;
	}

	return std::vector<uint8_t>();
}

VS_SymmetricAlg VS_H235CryptoEngine::GetAlgorithm() const{
	auto pCrypter = GetValidImpl(m_crypter);
	if (!pCrypter)	return VS_SymmetricAlg::alg_sym_NONE;

	return pCrypter->GetAlg();
}
VS_SymmetricCipherMode VS_H235CryptoEngine::GetMode() const{
	auto pCrypter = GetValidImpl(m_crypter);
	if (!pCrypter)	return VS_SymmetricCipherMode::mode_NONE;

	return pCrypter->GetCipherMode();
}
#pragma once

#include "SecureLib/VS_StreamCrypter.h"
#include "tools/H323Gateway/Lib/src/VS_H235Enumerations.h"
#include "SecureLib/SecureTypes.h"

#include <vector>
#include <cstdint>

class VS_H235CryptoEngine : protected VS_StreamCrypter{
public:
	bool Init(const std::vector<uint8_t> &key, const encryption_mode emode);
	bool Init(const std::vector<uint8_t> &key, const VS_SymmetricAlg &alg, const VS_SymmetricCipherMode &m);
	std::tuple<bool/*res*/, bool /*padding was used*/> Encrypt(const std::vector<uint8_t> &buf, const uint8_t * iv, std::vector<uint8_t> &encr_buf, const bool use_cts = false, encryption_method *OUT_method = nullptr) const;
	std::tuple<bool/*res*/, bool /*padding was used*/> Encrypt(const uint8_t *buf, const size_t buf_len, const uint8_t * iv, std::vector<uint8_t> &encr_buf, const bool use_cts = false, encryption_method *OUT_method = nullptr) const;
	bool Decrypt(const std::vector<uint8_t> &buf, const uint8_t * iv, const bool padding, std::vector<uint8_t> &decr_buf, encryption_method *OUTmethod = nullptr) const;
	bool Decrypt(const uint8_t *buf, const size_t buf_len, const uint8_t * iv, const bool padding, std::vector<uint8_t> &decr_buf, encryption_method *OUTmethod = nullptr) const;
	std::vector<uint8_t> GenerateKey(uint32_t key_size) const;
	std::vector<uint8_t> GetKey() const;
	bool Valid() const;
	VS_SymmetricAlg GetAlgorithm() const;
	VS_SymmetricCipherMode GetMode() const;

	// the IV sequence is always 6 bytes long (2 bytes seq number + 4 bytes timestamp) see the H.235 v3 specification, B.3.1.1 CBC initialization vector
	static const unsigned int IV_SEQUENCE_LEN = 6;

	VS_H235CryptoEngine() {}
	VS_H235CryptoEngine(const VS_H235CryptoEngine&) = delete;
	VS_H235CryptoEngine& operator=(const VS_H235CryptoEngine&) = delete;

	VS_H235CryptoEngine(VS_H235CryptoEngine&& other) : VS_StreamCrypter(std::move(other)) {}
	VS_H235CryptoEngine& operator=(VS_H235CryptoEngine&& other) {
		VS_StreamCrypter::operator=(std::move(other));
		return *this;
	}
};
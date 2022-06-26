#include "Handshake.h"
#include "Const.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_Sign.h"
#include "../std/cpplib/VS_Utils.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_RegistryConst.h"
#include "../acs/VS_AcsDefinitions.h"

#include <cassert>
#include <cstring>

static const unsigned c_rnd_data_size = 16;

namespace transport {

const char PrimaryField[net::HandshakeHeader::primary_field_size] = { '_','V','S','_','T','R','A','N','S','P','O','R','T','_',0 };

// use only for reply to old architecture clients
std::unique_ptr<net::HandshakeHeader, array_deleter<char>> CreateHandshakeReply_OLDARCH()
{
	std::unique_ptr<net::HandshakeHeader, array_deleter<char>> hs;

	const size_t body_size =
		+ 1/*type*/
		+ 1/*result_code*/
		+ 2/*max_conn_silence_ms*/
		+ 1/*fatal_silence_coef*/
		+ 1/*key_size*/
		+ 2/*key*/
		+ 1/*hops*/
		;
	hs.reset(reinterpret_cast<net::HandshakeHeader*>(new char[sizeof(net::HandshakeHeader) + body_size]));
	auto p = reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader);

	*p++ = 0; // type
	*p++ = 2; // result_code
	*reinterpret_cast<uint16_t*>(p) = 0; // max_conn_silence_ms
	p += 2;
	*p++ = 0; // fatal_silence_coef
	*p++ = 1; // key size
	*p++ = '0'; // key
	*p++ = '\0';
	*p++ = 0; // hops

	assert(p == reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader) + body_size);

	std::memcpy(hs->primary_field, PrimaryField, sizeof(PrimaryField));
	hs->version = c_version | c_ssl_support_mask;
	hs->body_length = body_size - 1;
	net::UpdateHandshakeChecksums(*hs);
	return hs;
}

std::unique_ptr<net::HandshakeHeader, array_deleter<char>> CreateHandshake(
	string_view cid,
	string_view sid,
	uint8_t hops,
	bool ssl_support,
	bool tcp_keep_alive_support,
	const void* rnd_data,
	size_t rnd_data_size)
{
	std::unique_ptr<net::HandshakeHeader, array_deleter<char>> hs;

	if (cid.size() > 255)
		return hs;
	if (sid.size() > 255)
		return hs;
	if (rnd_data && rnd_data_size > 0xffff)
		return hs;

	VS_Sign signer;
	VS_SignArg signarg = {alg_pk_RSA, alg_hsh_SHA1};
	VS_RegistryKey	key(false, CONFIGURATION_KEY);
	std::unique_ptr<char, free_deleter> PrivKeyBuf;
	const bool can_sign = signer.Init(signarg)
		&& key.GetValue(PrivKeyBuf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY)
		&& signer.SetPrivateKey(PrivKeyBuf.get(), store_PEM_BUF)
		;

	if (!rnd_data || rnd_data_size == 0)
	{
		rnd_data = nullptr;
		rnd_data_size = c_rnd_data_size;
	}

	size_t body_size =
		+ 1/*size*/ + cid.size() + 1/*'\0'*/
		+ 1/*size*/ + sid.size() + 1/*'\0'*/
		+ 1/*hops*/
		+ (can_sign && hops != 0 ? 2/*size*/ + rnd_data_size : 0)
		+ (can_sign && hops != 0 ? 2/*size*/ + VS_SIGN_SIZE : 0)
		+ 1/*tcp_keep_alive_support*/
		;
	hs.reset(reinterpret_cast<net::HandshakeHeader*>(new char[sizeof(net::HandshakeHeader) + body_size]));
	auto p = reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader);

	auto put_str = [&](string_view str) {
		*p++ = str.size();
		std::memcpy(p, str.data(), str.size());
		p += str.size();
		*p++ = '\0';
	};

	put_str(cid);
	put_str(sid);
	*p++ = hops;
	if (can_sign && hops != 0)
	{
		*reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(rnd_data_size);
		p += 2;
		if (rnd_data)
			std::memcpy(p, rnd_data, rnd_data_size);
		else
			VS_GenKeyByMD5(p);
		const auto p_rnd_data = p;
		p += rnd_data_size;

		uint32_t sign_size = VS_SIGN_SIZE;
		if (!signer.SignData(p_rnd_data, rnd_data_size, p + 2, &sign_size) || sign_size > 0xffff)
		{
			hs.reset();
			return hs;
		}
		body_size = body_size - VS_SIGN_SIZE + sign_size;
		*reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(sign_size);
		p += 2;
		p += sign_size;
	}
	*p++ = tcp_keep_alive_support ? 1 : 0;
	assert(p == reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader) + body_size);

	std::memcpy(hs->primary_field, PrimaryField, sizeof(PrimaryField));
	hs->version = c_version | (ssl_support ? c_ssl_support_mask : 0);
	hs->body_length = body_size - 1;
	net::UpdateHandshakeChecksums(*hs);
	return hs;
}

bool ParseHandshake(
	const net::HandshakeHeader* hs,
	const char*& cid,
	const char*& sid,
	uint8_t& hops,
	const void*& rnd_data,
	size_t& rnd_data_size,
	const void*& sign,
	size_t& sign_size,
	bool& tcp_keep_alive_support)
{
	if (!hs || hs->version < 1 || hs->body_length + 1 < 6) /* { 0, '\0', 0, '\0', hops=0, tcp_keep_alive_support } */
		return false;
	auto p = reinterpret_cast<const uint8_t*>(hs) + sizeof(net::HandshakeHeader);
	const auto p_end = p + hs->body_length + 1;

	auto get_str = [&](const char*& str) {
		if (p >= p_end)
			return false;
		auto sz = *p++;
		str = reinterpret_cast<const char*>(p);
		p += sz;
		if (p >= p_end)
			return false;
		if (*p++ != 0)
			return false;
		return true;
	};

	if (!get_str(cid))
		return false;
	if (!get_str(sid))
		return false;

	if (p >= p_end)
		return false;
	hops = *p++;

	if (hops != 0 && p + 1 < p_end)
	{
		if (p + 1 >= p_end)
			return false;
		rnd_data_size = *reinterpret_cast<const uint16_t*>(p);
		p += 2;
		rnd_data = p;
		p += rnd_data_size;

		if (p + 1 >= p_end)
			return false;
		sign_size = *reinterpret_cast<const uint16_t*>(p);
		p += 2;
		sign = p;
		p += sign_size;
	}
	else
	{
		rnd_data = nullptr;
		rnd_data_size = 0;
		sign = nullptr;
		sign_size = 0;
	}

	if (p >= p_end)
		return false;
	tcp_keep_alive_support = *p++ != 0;

	return p <= p_end;
}

std::unique_ptr<net::HandshakeHeader, array_deleter<char>> CreateHandshakeReply(
	string_view sid,
	string_view cid,
	HandshakeResult result,
	uint16_t max_conn_silence_ms,
	uint8_t fatal_silence_coef,
	uint8_t hops,
	bool ssl_support,
	bool tcp_keep_alive_support)
{
	std::unique_ptr<net::HandshakeHeader, array_deleter<char>> hs;

	if (sid.size() > 255)
		return hs;
	if (cid.size() > 255)
		return hs;

	const size_t body_size =
		+ 1/*result*/
		+ 2/*max_conn_silence_ms*/
		+ 1/*fatal_silence_coef*/
		+ 1/*hops*/
		+ 1/*size*/ + sid.size() + 1/*'\0'*/
		+ 1/*size*/ + cid.size() + 1/*'\0'*/
		+ 1/*tcp_keep_alive_support*/
		;
	hs.reset(reinterpret_cast<net::HandshakeHeader*>(new char[sizeof(net::HandshakeHeader) + body_size]));
	auto p = reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader);

	auto put_str = [&](string_view str) {
		*p++ = str.size();
		std::memcpy(p, str.data(), str.size());
		p += str.size();
		*p++ = '\0';
	};

	*p++ = static_cast<uint8_t>(result);
	*reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(max_conn_silence_ms);
	p += 2;
	*p++ = static_cast<uint8_t>(fatal_silence_coef);
	*p++ = static_cast<uint8_t>(hops);
	put_str(sid);
	put_str(cid);
	*p++ = tcp_keep_alive_support ? 1 : 0;
	assert(p == reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader) + body_size);

	std::memcpy(hs->primary_field, PrimaryField, sizeof(PrimaryField));
	hs->version = c_version | (ssl_support ? c_ssl_support_mask : 0);
	hs->body_length = body_size - 1;
	net::UpdateHandshakeChecksums(*hs);
	return hs;
}

bool ParseHandshakeReply(
	const net::HandshakeHeader* hs,
	const char*& sid,
	const char*& cid,
	HandshakeResult& result,
	uint16_t& max_conn_silence_ms,
	uint8_t& fatal_silence_coef,
	uint8_t& hops,
	bool& tcp_keep_alive_support)
{
	if (!hs || hs->version < 1 || hs->body_length + 1 < 10) /* { result, max_conn_silence_ms, fatal_silence_coef, hops, 0, '\0', 0, '\0', tcp_keep_alive_support } */
		return false;
	auto p = reinterpret_cast<const uint8_t*>(hs) + sizeof(net::HandshakeHeader);
	const auto p_end = p + hs->body_length + 1;

	auto get_str = [&](const char*& str) {
		if (p >= p_end)
			return false;
		auto sz = *p++;
		str = reinterpret_cast<const char*>(p);
		p += sz;
		if (p >= p_end)
			return false;
		if (*p++ != 0)
			return false;
		return true;
	};

	if (p >= p_end)
		return false;
	result = static_cast<HandshakeResult>(*p++);

	if (p + 1 >= p_end)
		return false;
	max_conn_silence_ms = *reinterpret_cast<const uint16_t*>(p);
	p += 2;

	if (p >= p_end)
		return false;
	fatal_silence_coef = *p++;

	if (p >= p_end)
		return false;
	hops = *p++;

	if (!get_str(sid))
		return false;
	if (!get_str(cid))
		return false;

	if (p >= p_end)
		return false;
	tcp_keep_alive_support = *p++ != 0;

	return p <= p_end;
}

}

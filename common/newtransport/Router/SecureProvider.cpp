#include "SecureProvider.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_SymmetricCrypt.h"
#include "../../std/debuglog/VS_Debug.h"

#include "std-generic/compat/memory.h"
#include <cassert>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

#define TRANSPORT_VERBOSE_LOGS 0

namespace transport {

std::function<std::unique_ptr<SecureProvider>()> SecureProvider::s_factory;

std::unique_ptr<SecureProvider> SecureProvider::Create()
{
	if (s_factory)
		return s_factory();
	else
		return vs::make_unique<SecureProviderImpl>();
}

SecureProviderImpl::SecureProviderImpl()
	: m_next_input_size(0)
{
}

SecureProviderImpl::~SecureProviderImpl() = default;

void SecureProviderImpl::StartSecureHandshake(unsigned version, VS_SecureHandshakeType type, VS_PKey& key, const VS_Container& cert_chain)
{
	m_hs.StopHandshake();
	m_next_input_size = 0;

	if (!m_hs.Init(version, type))
	{
		dstream1 << "transport::SecureProvider: failed to initialize secure handshake";
		return;
	}
	if (version == 2 || type == handshake_type_Server)
	{
		if (!m_hs.SetPrivateKey(&key))
		{
			dstream1 << "transport::SecureProvider: failed to set PKey";
			return;
		}
	}
	m_cert_chain = cert_chain;
	m_state = State::handshake_in_progress;
}

const VS_SymmetricCrypt* SecureProviderImpl::ReadCrypt() const
{
	return m_read_crypt.get();
}

const VS_SymmetricCrypt* SecureProviderImpl::WriteCrypt() const
{
	return m_write_crypt.get();
}

size_t SecureProviderImpl::HandlePacket(const void* data, size_t size)
{
	if (m_next_input_size == 0 || size < m_next_input_size)
		return 0;

	if (!m_hs.ProcessPacket(data, m_next_input_size))
	{
		m_state = State::error;
		auto ds = dstream1;
		ds << "transport::SecureProvider: failed to process secure handshake packet";
#if TRANSPORT_VERBOSE_LOGS
		ds << ", expected size: " << m_next_input_size << ", data:\n" << std::hex;
		for (size_t i = 0; i < size; ++i)
			ds << static_cast<unsigned>(reinterpret_cast<const uint8_t*>(data)[i]) << ' ';
#endif
		return 0;
	}
	const size_t consumed = m_next_input_size;
	m_next_input_size = 0;
	return consumed;
}

std::vector<uint8_t> SecureProviderImpl::GetPacket()
{
	std::vector<uint8_t> result;
	if (m_next_input_size > 0)
		return result; // Don't try to advance to the next state until we have completed GetPacket request of the handshaker.
	switch (m_hs.Next())
	{
	case secure_st_SendCert:
	{
#if TRANSPORT_VERBOSE_LOGS
		dstream4 << "transport::SecureProvider: secure_st_SendCert";
#endif
		size_t cert_chain_size;
		if (!m_cert_chain.Serialize(nullptr, cert_chain_size))
		{
			m_state = State::error;
			dstream1 << "transport::SecureProvider: secure_st_SendCert failed (1)";
			break;
		}
		assert(cert_chain_size < 0xffffffff);
		result.resize(sizeof(uint32_t) + cert_chain_size);
		if (!m_cert_chain.Serialize(result.data() + sizeof(uint32_t), cert_chain_size))
		{
			m_state = State::error;
			dstream1 << "transport::SecureProvider: secure_st_SendCert failed (2)";
			result.clear();
			break;
		}
		*reinterpret_cast<uint32_t*>(result.data()) = static_cast<uint32_t>(cert_chain_size);
		result.resize(sizeof(uint32_t) + cert_chain_size);
	}
		break;
	case secure_st_GetPacket:
	{
#if TRANSPORT_VERBOSE_LOGS
		dstream4 << "transport::SecureProvider: secure_st_GetPacket";
#endif
		void* packet_buffer = nullptr;
		uint32_t packet_size = 0;
		if (!m_hs.PreparePacket(&packet_buffer, &packet_size))
		{
			m_state = State::error;
			dstream1 << "transport::SecureProvider: secure_st_GetPacket failed (1)";
			break;
		}
		m_hs.FreePacket(&packet_buffer);
		assert(m_next_input_size == 0);
		m_next_input_size = packet_size;
	}
		break;
	case secure_st_SendPacket:
	{
#if TRANSPORT_VERBOSE_LOGS
		dstream4 << "transport::SecureProvider: secure_st_SendPacket";
#endif
		void* packet_buffer = nullptr;
		uint32_t packet_size = 0;
		if (!m_hs.PreparePacket(&packet_buffer, &packet_size))
		{
			m_state = State::error;
			dstream1 << "transport::SecureProvider: secure_st_SendPacket failed (1)";
			break;
		}
		result.assign(reinterpret_cast<uint8_t*>(packet_buffer), reinterpret_cast<uint8_t*>(packet_buffer) + packet_size);
		m_hs.FreePacket(&packet_buffer);
	}
		break;
	case secure_st_Error:
		m_state = State::error;
		dstream1 << "transport::SecureProvider: secure_st_Error, error code: " << m_hs.GetHandshakeErrorCode();
		break;
	case secure_st_Finish:
#if TRANSPORT_VERBOSE_LOGS
		dstream4 << "transport::SecureProvider: secure_st_Finish";
#endif
		m_write_crypt.reset(m_hs.GetWriteSymmetricCrypt());
		m_read_crypt.reset(m_hs.GetReadSymmetricCrypt());
		if (!m_write_crypt || !m_read_crypt)
		{
			m_state = State::error;
			dstream1 << "transport::SecureProvider: secure_st_Finish failed (1)";
			break;
		}
		m_state = State::handshake_completed;
		break;
	}
	return result;
}

}

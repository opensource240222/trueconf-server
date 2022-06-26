#pragma once

#include "SecureLib/VS_SecureHandshake.h"
#include "std-generic/cpplib/VS_Container.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace transport {

class SecureProvider
{
public:
	static std::unique_ptr<SecureProvider> Create();
	template <class Factory>
	static void SetFactory(Factory&& factory)
	{
		s_factory = std::forward<Factory>(factory);
	}
private:
	static std::function<std::unique_ptr<SecureProvider>()> s_factory;

public:
	enum class State
	{
		handshake_completed,
		handshake_in_progress,
		error,
	};

	SecureProvider() : m_state(State::error) {}
	virtual ~SecureProvider() {};

	State GetState() const
	{
		return m_state;
	}

	virtual const VS_SymmetricCrypt* ReadCrypt() const = 0;
	virtual const VS_SymmetricCrypt* WriteCrypt() const = 0;

	virtual void StartSecureHandshake(unsigned version, VS_SecureHandshakeType type, VS_PKey& key, const VS_Container& cert_chain) = 0;
	virtual size_t HandlePacket(const void* data, size_t size) = 0;
	virtual std::vector<uint8_t> GetPacket() = 0;

protected:
	State m_state;
};

class SecureProviderImpl : public SecureProvider
{
public:
	SecureProviderImpl();
	~SecureProviderImpl();

	const VS_SymmetricCrypt* ReadCrypt() const override;
	const VS_SymmetricCrypt* WriteCrypt() const override;

	void StartSecureHandshake(unsigned version, VS_SecureHandshakeType type, VS_PKey& key, const VS_Container& cert_chain) override;
	size_t HandlePacket(const void* data, size_t size) override;
	std::vector<uint8_t> GetPacket() override;

private:
	VS_SecureHandshake m_hs;
	VS_Container m_cert_chain;
	std::unique_ptr<VS_SymmetricCrypt> m_read_crypt;
	std::unique_ptr<VS_SymmetricCrypt> m_write_crypt;
	size_t m_next_input_size;

};

}

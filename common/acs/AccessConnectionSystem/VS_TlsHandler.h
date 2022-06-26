#pragma once

#include "acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionHandlerTypes.h"
#include "acs/connection/VS_IOHandler.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std/cpplib/VS_SimpleStr.h"
#include <boost/shared_ptr.hpp>

#include <array>
#include <map>

class VS_SetConnectionInterface;
class VS_ConnectionTLS;

// VS_TlsHadnler class is used to take TLS connections from acs, perform TLS handshake and
// re-delegate an underlying connection to another handler. Has an ACS-like interface and
// imitates acs-like behaviour
class VS_TlsHandler :
	public VS_AccessConnectionHandler
{
public:
//	VS_AccessConnectionHandler interface
	VS_TlsHandler(void);
	virtual	~VS_TlsHandler(void);
	bool IsValid(void) const override;
	bool Init(const char *handler_name) override;
	VS_ACS_Response	Connection(unsigned long *in_len) override;
	VS_ACS_Response	Protocol(const void *in_buffer, unsigned long *in_len,
		void **out_buffer = nullptr, unsigned long *out_len = nullptr,
		void **context = nullptr) override;
	void Accept(VS_ConnectionTCP *conn, const void *in_buffer,
		const unsigned long in_len, const void *context) override;
	void Destructor(const void *context) override;
	void Destroy(const char *handler_name) override;
	char *HandlerName(void) const override {return m_handlerName.m_str;}

//	ACS wannabe interface
	bool AddHandler(const char *name, VS_AccessConnectionHandler *handler);
	void RemoveHandler(const char *name);
	void RemoveAllHandlers(void);

private:
	static const size_t defaultBufferSize = 64 * 1024;

	enum DelegationResult
	{
		dr_accepted,
		dr_rejected,
		dr_need_more,
		dr_try_others, // Returned only if someone claimed the connection, but then started acting strangely
                       // or even rejected it
		dr_error
	};

	DelegationResult TryDelegate(VS_ConnectionTLS* connection, const void* buffer,
		unsigned long size, size_t& desiredSize, size_t& maxDesiredSize, const char*& claimant);

//	Yeah, bad design
	class ConnectionContext: public VS_IOHandler
	{
	public:
		typedef std::function<void(ConnectionContext*)> ReadCallback;
		typedef std::function<void(const unsigned long, ConnectionContext*)> ErrorCallback;

		VS_ConnectionTLS* connection;
		std::vector<unsigned char> buffer;
		size_t dataLength;
		size_t desiredDataLength;
		const char* claimant;
		ConnectionContext(
			VS_ConnectionTLS* newConnection,
			const std::vector<unsigned char>& newBuffer,
			size_t newDesiredDataLength = 0,
			const char* newClaimant = nullptr)
			:	connection(newConnection),
				buffer(newBuffer),
				dataLength(0),
				desiredDataLength(newDesiredDataLength),
				claimant(newClaimant)
			{
				buffer.resize(64 > desiredDataLength ? 64 : desiredDataLength); // minimal size
			}
		void SetCallbacks(ReadCallback readHandler, ErrorCallback errorHandler)
			{m_readHandler = readHandler; m_errorHandler = errorHandler;}
		void RequestRead();
	private:
		void Handle(const unsigned long sz, const struct VS_Overlapped *ov) override;
		void HandleError(const unsigned long err, const struct VS_Overlapped *ov) override;
		ReadCallback m_readHandler;
		ErrorCallback m_errorHandler;

	};
//	Callbacks
	void OnHandshakeFinished(VS_ConnectionTLS* connection, const void* buffer, const unsigned long size);
	void OnDataReceived(ConnectionContext* context);
	void OnError(const unsigned long error, ConnectionContext* context);

	void AddConnectionContext(VS_ConnectionTLS* connection, const void* buffer,
		size_t size, size_t desiredSize, const char* claimant);

	VS_SimpleStr m_handlerName;
	boost::shared_ptr<VS_WorkThread> m_workThread;
	std::map<std::string, VS_AccessConnectionHandler*, vs::str_less> m_handlers;
	std::map<VS_ConnectionTLS*, ConnectionContext> m_connections;
};

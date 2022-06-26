#include "VS_TlsHandler.h"

#include "acs/connection/VS_ConnectionTLS.h"
#include "acs/Lib/VS_LoadCertsForTLS.h"
#include "std/cpplib/VS_WorkThreadIOCP.h"
#include "std-generic/cpplib/string_view.h"
#include "TLS_Check.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT
namespace
{
const char moduleName[] = "TlsHandler: ";
const unsigned long defaultHandshakeTimeout = 5000;//milliseconds
}


VS_TlsHandler::VS_TlsHandler(void): m_workThread(new VS_WorkThreadIOCP), m_handlers()
{
	m_workThread->Start("TlsHandler");
}

VS_TlsHandler::~VS_TlsHandler(void)
{
}

bool VS_TlsHandler::IsValid(void) const
{	return VS_AccessConnectionHandler::IsValid();	}

bool VS_TlsHandler::Init(const char *handler_name)
{
	m_handlerName = handler_name;
	return true;
}

VS_ACS_Response VS_TlsHandler::Connection(unsigned long *in_len)
{
	if (in_len)
		*in_len = TLS_HANDSHAKE_RECORD_HEADER_SIZE;
	return vs_acs_next_step;
}

VS_ACS_Response VS_TlsHandler::Protocol(const void* in_buffer, unsigned long* in_len,
											 void** /*out_buffer*/, unsigned long* /*out_len*/,
											 void** /*context*/)
{
	if (*in_len < TLS_HANDSHAKE_RECORD_HEADER_SIZE)
	{
		*in_len = TLS_HANDSHAKE_RECORD_HEADER_SIZE;
		return vs_acs_next_step;
	}
	if (VS_IsTLSClientHello(reinterpret_cast<const uint8_t*>(in_buffer), *in_len))
		return vs_acs_accept_connections;
	else
		return vs_acs_connection_is_not_my;
}

void VS_TlsHandler::Accept(VS_ConnectionTCP *conn, const void *in_buffer,
								const unsigned long in_len, const void* /*context*/)
{
	const SSL_METHOD* method;
	if (VS_IsTLSClientHello(reinterpret_cast<const uint8_t *>(in_buffer), in_len) &&
		(method = VS_TLSClientHelloCheck(reinterpret_cast<const uint8_t*>(in_buffer), in_len)) != nullptr)
	{
		VS_ConnectionTLS *conTLS = new VS_ConnectionTLS(conn, m_workThread, method);
		delete conn;

		if (conTLS->IsValid())
		{
			unsigned long timeout = defaultHandshakeTimeout;
			//load certificates here
			if (!VS_LoadCertsForTLS(*conTLS)
				|| !conTLS->Handshake(in_buffer, in_len,
					[this](bool success, VS_ConnectionTLS* connection,
						const void* buffer, const unsigned long size)
					{
						if (success)
							OnHandshakeFinished(connection, buffer, size);
						else
						{
							dstream2 << moduleName << "warning: handshake failed mid-way!\nTLS state string: "
								<< connection->GetStateStringLong();
							delete connection;
						}
					},
					timeout,
					true
					)
				)
			{
				delete conTLS;
			}
		}
		else
			delete conTLS;
	} else
		delete conn;
}

void VS_TlsHandler::OnHandshakeFinished(VS_ConnectionTLS* connection, const void* buffer,
	const unsigned long size)
{
	dstream4 << moduleName << "note: successfully finished the handshake!";
	size_t desiredSize = 0;
	size_t maxDesiredSize = 0;
	const char* claimant = nullptr;
	auto result = TryDelegate(connection, buffer, size, desiredSize, maxDesiredSize, claimant);
	switch (result)
	{
	case dr_accepted:
		return;
	case dr_rejected:
	case dr_try_others: // never actually returns this
		delete connection;
		return;
	case dr_need_more:
		AddConnectionContext(connection, buffer, size, desiredSize, claimant);
		return;
	case dr_error:
		dstream2 << moduleName << "warning: error encountered while trying to delegate the connection!";
		VS_FALLTHROUGH;
	default:
		return;
	}
}

void VS_TlsHandler::OnDataReceived(ConnectionContext* context)
{
	size_t maxDesiredSize = context->desiredDataLength;
	if (context->dataLength >= context->desiredDataLength)
	{
		auto connection = context->connection;
		switch (auto result = TryDelegate(
				context->connection, context->buffer.data(), context->dataLength,
				context->desiredDataLength, maxDesiredSize, context->claimant))
		{
		case dr_accepted:
			m_connections.erase(connection);
			context = nullptr;
			break;
		case dr_error:
			dstream2 << moduleName << "warning: error encountered while trying to delegate the connection!";
			VS_FALLTHROUGH;
		case dr_rejected:
			m_connections.erase(connection);
			delete connection;
			context = nullptr;
			break;
		case dr_try_others:
			OnDataReceived(context);
			break;
		case dr_need_more:
		default:
			break;
		}
	}
	if (context)
	{
		context->buffer.resize(maxDesiredSize);
		context->RequestRead();
	}
}

void VS_TlsHandler::OnError(const unsigned long error, ConnectionContext* context)
{
	dstream3 << moduleName << "minor warning: error while trying to read from connection!\n"
		<< "Error code: " << error;
	auto connection = context->connection;
	m_connections.erase(context->connection);
	delete connection;
}

void VS_TlsHandler::AddConnectionContext(VS_ConnectionTLS* connection, const void* buffer,
		size_t size, size_t desiredSize, const char* claimant)
{
	auto emplaceResult = m_connections.emplace(connection,
		ConnectionContext(connection,
			std::vector<unsigned char>(
				static_cast<const unsigned char*>(buffer),
				static_cast<const unsigned char*>(buffer) + size
			),
			desiredSize,
			claimant));
	ConnectionContext& context = emplaceResult.first->second;
	context.connection->SetIOHandler(&context);
	context.SetCallbacks(
		[this](ConnectionContext* context) // On receiving data
		{
			OnDataReceived(context);
		},
		[this](const unsigned long error, ConnectionContext* context) // On error
		{
			OnError(error, context);
		}
	);
	context.RequestRead();
}

VS_TlsHandler::DelegationResult VS_TlsHandler::TryDelegate(VS_ConnectionTLS* connection, const void* buffer,
	unsigned long size, size_t& desiredSize, size_t& maxDesiredSize, const char*& claimant)
{
	if (claimant)
	{
		unsigned long returnedSize = size;
		auto handler = m_handlers.find(string_view(claimant));
		switch (VS_ACS_Response response = handler->second->Protocol(buffer, &returnedSize,
				nullptr, nullptr, nullptr))
		{
		case vs_acs_accept_connections:
			handler->second->Accept(connection, buffer, size, nullptr);
			dstream4 << moduleName << "note: TLS connection established, passed to \"" <<
				handler->second->HandlerName() << "\"";
			return dr_accepted;
		case vs_acs_connection_is_not_my:
			dstream2 << moduleName << "warning: TlsHandler has been fooled by his own handler!\n"
				<< "It claimed the connection first, but then rejected it!\n"
				<< "Handler name: " << claimant;
			claimant = nullptr;
			return dr_try_others;
		case vs_acs_my_connections:
		case vs_acs_next_step:
			dstream2 << moduleName << "warning: handler \"" << claimant <<
				"\" returned vs_acs_next_step after claiming the connection!";
			if (returnedSize > desiredSize)
			{
				desiredSize = returnedSize;
				maxDesiredSize = maxDesiredSize > returnedSize ? maxDesiredSize : returnedSize;
				return dr_need_more;
			}
			claimant = nullptr;
			return dr_try_others;
		case vs_acs_free_connections:
			dstream2 << moduleName << "warning: handler \"" << handler->second->HandlerName()
				<< "\" returned free_connections, which has NEVER been returned before!";
		default:
			return dr_error;
		}
	}
	size_t newDesiredSize = static_cast<size_t>(-1);
	bool rejected = true;
	for (auto& handler: m_handlers)
	{
		unsigned long returnedSize = size;
		switch (VS_ACS_Response response = handler.second->Protocol(buffer, &returnedSize,
				nullptr, nullptr, nullptr))
		{
		case vs_acs_accept_connections:
			handler.second->Accept(connection, buffer, size, nullptr);
			dstream4 << moduleName << "note: TLS connection established, passed to \"" <<
				handler.second->HandlerName() << "\"";
			return dr_accepted;
		case vs_acs_connection_is_not_my:
			continue;
		case vs_acs_my_connections:
			if (returnedSize > size)
			{
				maxDesiredSize = desiredSize = returnedSize;
				claimant = handler.second->HandlerName();
				return dr_need_more;
			} else
				continue;
		case vs_acs_next_step:
			if (returnedSize > size && returnedSize < newDesiredSize)
			{
				rejected = false;
				newDesiredSize = returnedSize;
			}
			if (returnedSize > maxDesiredSize)
				maxDesiredSize = returnedSize;
			continue;
		case vs_acs_free_connections:
			dstream2 << moduleName << "warning: handler \"" << handler.second->HandlerName()
				<< "\" returned free_connections, which has NEVER been returned before!";
		default:
			dstream2 << moduleName << "warning: handler \"" << handler.second->HandlerName()
				<< "\" returned an unexpected value " << response << "!";
		}
	}
	if (rejected)
	{
		dstream2 << moduleName << "warning: TLS connection was established, but no handler accepted the underlying protocol!\n"
			<< "Either handlers list is incomplete, or someone is playing with TLS";
		return dr_rejected;
	}
	else
	{
		desiredSize = newDesiredSize;
		return dr_need_more;
	}
}

void VS_TlsHandler::Destructor(const void *context)
{
}

void VS_TlsHandler::Destroy(const char *handler_name)
{
}

bool VS_TlsHandler::AddHandler(const char *name, VS_AccessConnectionHandler *handler)
{
	if (m_handlers.insert({name, handler}).second)
		return true;
	return false;
}
void VS_TlsHandler::RemoveHandler(const char *name)
{
	m_handlers.erase(name);
}
void VS_TlsHandler::RemoveAllHandlers(void)
{
	m_handlers.clear();
}

void VS_TlsHandler::ConnectionContext::Handle(const unsigned long sz, const VS_Overlapped* ov)
{
	auto true_size = connection->SetReadResult(sz, ov, 0, true);
	if (true_size < 0)
		return;
	dataLength += true_size;
	if (m_readHandler != nullptr)
		m_readHandler(this);
}

void VS_TlsHandler::ConnectionContext::HandleError(const unsigned long err, const VS_Overlapped* /*ov*/)
{
	if (m_errorHandler != nullptr)
		m_errorHandler(err, this);
}

void VS_TlsHandler::ConnectionContext::RequestRead()
{
	connection->Read(buffer.data() + dataLength, buffer.size() - dataLength);
}

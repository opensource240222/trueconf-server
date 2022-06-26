/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_RouterMessage.h
/// \brief
/// \note
///

#ifndef VS_ROUTER_MESSAGE_H
#define VS_ROUTER_MESSAGE_H

#include "../Message.h"

class VS_RouterMessage : public transport::Message
{
public:
	VS_RouterMessage() = default;
	VS_RouterMessage(const void* data, size_t size) : transport::Message(data, size) {}
	VS_RouterMessage(const transport::Message& msg) : transport::Message(msg) {}
	VS_RouterMessage(transport::Message&& msg)      : transport::Message(std::move(msg)) {}
	VS_RouterMessage& operator=(const transport::Message& msg)
	{
		transport::Message::operator=(msg);
		return *this;
	}
	VS_RouterMessage& operator=(transport::Message&& msg)
	{
		transport::Message::operator=(std::move(msg));
		return *this;
	}

	VS_RouterMessage( const char *our_CID, const char *our_service,
						const char *add_string,
						const char *CID, const char *service,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size,
						const char * to_user=0, const char * from_user=0, const char *to_server =0, const char *from_server = 0);

	//Ќет CID, все остальное есть в конструкторе и методе SET
	VS_RouterMessage(	const char *our_service,
						const char *add_string,
						const char *service,
						const char * to_user,
						const char * from_user,
						const char *to_server, const char *from_server,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size);

	VS_RouterMessage(const VS_RouterMessage *request,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size,
						const char* add_string=0);

	// make local request to our service
	VS_RouterMessage( const char* server,
						const char* our_service, const char* to_service,
						const char* body, const unsigned long body_size);

	bool	Set( const char *our_CID, const char *our_service,
						const char *add_string,
						const char *CID, const char *service,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size,
						const char * to_user=0, const char * from_user=0,
						const char *to_server = 0, const char *from_server = 0);
	bool	Set( const char *our_service,
						const char *add_string,
						const char *service,
						const char * to_user, const char * from_user,
						const char *to_server, const char *from_server,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size);
	bool	Set( VS_RouterMessage *request,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size );
	void	CopyFrom(const VS_RouterMessage* msg);
};
// end VS_RouterMessage class

#endif // VS_ROUTER_MESSAGE_H

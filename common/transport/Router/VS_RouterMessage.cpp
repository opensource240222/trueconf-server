//////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Непосредственная реализация поддержки управляющего протокола на сервере
//
//  Created: 12.11.02     by  A.Slavetsky
//
//////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_TransportRouter.cpp
/// \brief Непосредственная реализация поддержки управляющего протокола на сервере
/// \note
///

#include "VS_RouterMessage.h"

extern std::string g_tr_endpoint_name;

static string_view to_sv(const char* s)
{
	if (s)
		return s;
	else
		return {};
}

VS_RouterMessage::VS_RouterMessage( const char *our_CID, const char *our_service,
										const char *add_string,
										const char *CID, const char *service,
										const unsigned long ms_timelimit,
										const void *body, const unsigned long size,
										const char * to_user, const char * from_user,
										const char * to_server, const char * from_server)
	: transport::Message(true, Message::GetNewSeqNumber(), ms_timelimit,
		to_sv(our_CID),
		to_sv(our_service),
		to_sv(add_string),
		to_sv(CID),
		to_sv(service),
		to_sv(from_user),
		to_sv(to_user),
		to_sv(from_server),
		to_sv(to_server),
		body, size)
{
}

VS_RouterMessage::VS_RouterMessage(	const char *our_service,
						const char *add_string,
						const char *service,
						const char * to_user,
						const char * from_user,
						const char *to_server, const char *from_server,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size)
	: transport::Message(true, Message::GetNewSeqNumber(), ms_timelimit,
		{},
		to_sv(our_service),
		to_sv(add_string),
		{},
		to_sv(service),
		to_sv(from_user),
		to_sv(to_user),
		to_sv(from_server),
		to_sv(to_server),
		body, size)
{
}

VS_RouterMessage::VS_RouterMessage( const VS_RouterMessage *request,
										const unsigned long ms_timelimit,
										const void *body, const unsigned long size,const char* add_string )
	: transport::Message(false, request->SeqNum(), ms_timelimit,
		request->DstCID_sv(),
		request->DstService_sv(),
		to_sv(add_string),
		request->SrcCID_sv(),
		request->SrcService_sv(),
		request->DstUser_sv(),
		request->SrcUser_sv(),
		g_tr_endpoint_name,
		request->SrcServer_sv(),
		body, size)
{
}

// add_string param - type of notify
VS_RouterMessage::VS_RouterMessage( const char* server, const char* our_service, const char* to_service, const char* body, const unsigned long body_size )
	: transport::Message(true, Message::GetNewSeqNumber(), 1000,
		{},
		to_sv(our_service),
		{},
		{},
		to_sv(to_service),
		{},
		{},
		to_sv(server),
		to_sv(server),
		body, body_size)
{
}

bool VS_RouterMessage::Set( const char *our_CID, const char *our_service,
								const char *add_string,
								const char *CID, const char *service,
								const unsigned long ms_timelimit,
								const void *body, const unsigned long size,
								const char * to_user, const char * from_user,
								const char *to_server, const char *from_server)
{
	transport::Message new_value(true, Message::GetNewSeqNumber(),
		ms_timelimit,
		to_sv(our_CID),
		to_sv(our_service),
		to_sv(add_string),
		to_sv(CID),
		to_sv(service),
		to_sv(to_user),
		to_sv(from_user),
		to_sv(to_server),
		to_sv(from_server),
		body, size);
	if (!new_value.IsValid())
		return false;
	transport::Message::operator=(std::move(new_value));
	return true;
}

bool VS_RouterMessage::Set( const char *our_service,
						const char *add_string,
						const char *service,
						const char * to_user, const char * from_user,
						const char *to_server, const char *from_server,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size)
{
	transport::Message new_value(true, Message::GetNewSeqNumber(), ms_timelimit,
		{},
		to_sv(our_service),
		to_sv(add_string),
		{},
		to_sv(service),
		to_sv(to_user),
		to_sv(from_user),
		to_sv(from_server),
		to_sv(to_server),
		body, size);
	if (!new_value.IsValid())
		return false;
	transport::Message::operator=(std::move(new_value));
	return true;
}

bool VS_RouterMessage::Set( VS_RouterMessage *request,
								const unsigned long ms_timelimit,
								const void *body, const unsigned long size )
{
	transport::Message new_value(false, request->SeqNum(), ms_timelimit,
		request->DstCID_sv(),
		request->DstService_sv(),
		{},
		request->SrcCID_sv(),
		request->SrcService_sv(),
		request->DstUser_sv(),
		request->SrcUser_sv(),
		g_tr_endpoint_name,
		request->SrcServer_sv(),
		body, size);
	if (!new_value.IsValid())
		return false;
	transport::Message::operator=(std::move(new_value));
	return true;
}

void VS_RouterMessage::CopyFrom(const VS_RouterMessage* from)
{
	transport::Message::operator=(transport::Message(from->IsRequest(), from->SeqNum(), from->TimeLimit(), from->SrcCID_sv(), from->SrcService_sv(), from->AddString_sv(), from->DstCID_sv(), from->DstService_sv(), from->SrcUser_sv(), from->DstUser_sv(), from->SrcServer_sv(), from->DstServer_sv(), from->Body(), from->BodySize()));
}
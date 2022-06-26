#if defined(_WIN32) // Not ported yet

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
/// \file VS_TransportMessage.cpp
/// \brief
/// \note
///
#include <cstddef>
#include <cstdint>

#include "VS_TransportMessage.h"
#include "../VS_TransportDefinitions.h"
#include "../Lib/VS_TransportLib.h"
#include "../../acs/VS_AcsDefinitions.h"

#include <utility>
#include <atomic>
#include <string>

extern std::string g_tr_endpoint_name;

VS_TransportMessage::VS_TransportMessage( void ) : isValid(false), setMsg(false), mess(0) {}
VS_TransportMessage::~VS_TransportMessage( void ) {		Reset();	}
VS_TransportMessage::VS_TransportMessage(VS_TransportMessage &&src)
{
	*this = std::move(src);
}
VS_TransportMessage & VS_TransportMessage::operator=(VS_TransportMessage&&src)
{
	mess = src.mess;
	src.mess = 0;
	isValid = src.isValid;
	setMsg = src.setMsg;
	src.Reset();
	return *this;
}



bool VS_TransportMessage::IsValid ( void ) const {	return isValid;		}

const char *VS_TransportMessage::SrcCID( void ) const		{	return GetName( 1 );	}
const char *VS_TransportMessage::SrcService( void ) const	{	return GetName( 2 );	}
const char *VS_TransportMessage::AddString( void ) const	{	return GetName( 3 );	}
const char *VS_TransportMessage::DstCID( void ) const		{	return GetName( 4 );	}
const char *VS_TransportMessage::DstService( void ) const	{	return GetName( 5 );	}
const char *VS_TransportMessage::SrcUser( void ) const		{	return GetName( 6 );	}
const char *VS_TransportMessage::DstUser( void ) const		{	return GetName( 7 );	}
const char *VS_TransportMessage::SrcServer( void ) const	{	return GetName( 8 );	}
const char *VS_TransportMessage::DstServer( void ) const	{	return GetName( 9 );	}

bool VS_TransportMessage::SetSrcCID(const char *cid)			{	return SetName( 1, cid );		}
bool VS_TransportMessage::SetSrcService(const char *service)	{	return SetName( 2, service );	}
bool VS_TransportMessage::SetString(const char *string)		{	return SetName( 3, string );	}
bool VS_TransportMessage::SetDstCID( const char *cid )		{	return SetName( 4, cid );		}
bool VS_TransportMessage::SetDstService(const char *service)	{	return SetName( 5, service );	}
bool VS_TransportMessage::SetSrcUser(const char *user)		{	return SetName( 6, user );		}
bool VS_TransportMessage::SetDstUser(const char *user)		{	return SetName( 7, user );		}
bool VS_TransportMessage::SetSrcServer( const char *server )	{	return SetName( 8, server );	}
bool VS_TransportMessage::SetDstServer( const char *server )	{	return SetName( 9, server );	}

bool VS_TransportMessage::SetTimeLimit(unsigned time_limit)
{
	if (!isValid)
		return false;
	reinterpret_cast<transport::MessageFixedPart*>(mess)->ms_life_count = time_limit;
	return true;
}

bool VS_TransportMessage::IsFromServer() const
{
	const char * srcUser = SrcUser();
	const char *srcCID = SrcCID();
	return (srcUser == 0 || *srcUser == 0) && (srcCID == 0 || *srcCID == 0);
}

unsigned long VS_TransportMessage::TimeLimit( void ) const
{
	return !isValid ? 0 : ((transport::MessageFixedPart*)mess)->ms_life_count;
}

const unsigned char* VS_TransportMessage::Body() const
{
	if (!isValid)
		return nullptr;
	return mess + reinterpret_cast<const transport::MessageFixedPart*>(mess)->head_length;
}

size_t VS_TransportMessage::BodySize() const
{
	if (!isValid)
		return 0;
	return reinterpret_cast<const transport::MessageFixedPart*>(mess)->body_length + 1;
}

unsigned long VS_TransportMessage::Sequence( void ) const
{
	return !isValid ? 0 : ((transport::MessageFixedPart*)mess)->seq_number;
}

unsigned char VS_TransportMessage::Version( void ) const
{
	return !isValid ? 0 : (unsigned char)((transport::MessageFixedPart*)mess)->version;
}

bool VS_TransportMessage::IsRequest( void ) const
{
	return !isValid ? false : (bool)((transport::MessageFixedPart*)mess)->request;
}

bool VS_TransportMessage::IsReply( void ) const
{
	return !isValid ? false : !((transport::MessageFixedPart*)mess)->request;
}

bool VS_TransportMessage::IsNotify( void ) const
{
	return !isValid ? false : (bool)((transport::MessageFixedPart*)mess)->notify;
}

transport::MessageType VS_TransportMessage::Type( void ) const
{
	return !isValid ? transport::MessageType::Invalid : ( ((transport::MessageFixedPart*)mess)->notify ? transport::MessageType::Notify : ( ((transport::MessageFixedPart*)mess)->request ? transport::MessageType::Request : transport::MessageType::Reply ));
}

size_t VS_TransportMessage::Size() const
{
	if (!isValid)
		return 0;
	return reinterpret_cast<const transport::MessageFixedPart*>(mess)->head_length + reinterpret_cast<const transport::MessageFixedPart*>(mess)->body_length + 1;
}

const char *VS_TransportMessage::GetName( const unsigned n_name, const unsigned char *mess ) const
{
	const unsigned char *ms = !mess ? (!isValid ? 0 : VS_TransportMessage::mess) : mess;
	const unsigned char *ret(0);
	if (ms)
	{
		const unsigned long   length = ((transport::MessageFixedPart*)ms)->head_length;
		unsigned long   index = sizeof(transport::MessageFixedPart) - 1, size_name;
		for (unsigned i = 0; i < n_name; ++i)
		{
			if (++index >= length)
				return 0;
			size_name = ms[index];
			if (++index >= length)
				return 0;
			ret = &ms[index];
			if ((index += size_name) >= length)
				return 0;
			if (ms[index])
				return 0;
		}	}
	return reinterpret_cast<const char *>(ret);
}

bool VS_TransportMessage::SetName(const unsigned int number, const char * value)
{
	if (value==0)
		return false;
	if (!isValid)
		return false;
	const char *old_name = GetName(number);
	if (!old_name)
		return false;
	bool old_setMsg = setMsg;
	int old_name_len = (int)strlen(old_name);
	int new_name_len = (int)strlen(value);
	if (old_name_len==0 && new_name_len==0)
		return true;
	transport::MessageFixedPart &old_head = *(transport::MessageFixedPart*)mess;

	int old_mess_size = old_head.head_length + old_head.body_length + 1;
	int dif = new_name_len - old_name_len;
	int new_mess_size = old_mess_size + dif;
	unsigned char *new_mess = (unsigned char *)malloc(new_mess_size);

	int index = int(old_name - (const char *)mess - 1);
	memcpy(new_mess, mess, index);

	new_mess[index] = (unsigned char)new_name_len;		++index;
	strcpy((char *)&new_mess[index], value);	index += new_name_len + 1;

	memcpy(&new_mess[index], &mess[index-dif], new_mess_size - index);

	transport::MessageFixedPart &head = *(transport::MessageFixedPart*)new_mess;
	head.body_cksum = head.head_cksum = 0;
	head.head_length += dif;

	transport::UpdateMessageChecksums(head);
	Reset();
	VS_TransportMessage::mess = new_mess;
	isValid = true;
	setMsg = old_setMsg;

	return true;
}

void VS_TransportMessage::Reset( void )
{
	if (mess) {		free( (void *)mess );	mess = 0;	}
	isValid = setMsg = false;
}

void VS_TransportMessage::ReleaseMessage( void )
{
	mess = 0;
	isValid = setMsg = false;
}

bool VS_TransportMessage::Set( const unsigned char *mess )
{
	transport::MessageFixedPart &head = *(transport::MessageFixedPart*)mess;
	if (!head.mark1 || head.version < 1 || head.head_length < sizeof(transport::MessageFixedPart)
		|| !head.seq_number || head.head_cksum != transport::GetMessageHeaderChecksum(head)
		|| head.body_cksum != transport::GetMessageBodyChecksum(head))
	{
		return false;
	}
	Reset();
	VS_TransportMessage::mess = (unsigned char *)mess;
	return isValid = setMsg = true;
}

bool VS_TransportMessage::Set(const unsigned long request, const unsigned long seq_number,
							  const char *our_CID, const char *our_service,
							  const char *add_string,
							  const char *CID, const char *service,
							  const unsigned long ms_timelimit,
							  const void *body, const unsigned long size,
							  const char *to_user,
							  const char *from_user,
							  const char *to_server,
							  const char *from_server)
{
	if (!body || !size || size > VS_TRANSPORT_MAX_SIZE_BODY)
		return false;

	const int num = 9;
	const char * names[num] = {our_CID, our_service, add_string, CID, service, from_user, to_user, from_server, to_server};
	unsigned long lens[num];
	unsigned long size_mess = sizeof(transport::MessageFixedPart) + size + num*2;
	for (int i = 0; i<num; i++) {
		unsigned long len = 0;
		if (names[i]) {
			len = (unsigned long )strlen(names[i]);
			if (len > VS_TRANSPORT_MAX_SIZE_ADD_STRING)
				return false;
		}
		lens[i] = len;
		size_mess+= len;
	}

	unsigned char *mess = (unsigned char *)malloc(size_mess);
	if (!mess)
		return false;
	transport::MessageFixedPart &head = *(transport::MessageFixedPart*)mess;
	head.notify = 0;
	head.request = request;
	head.mark1 = 1;
	head.version = 1;
	head.body_cksum = head.head_cksum = 0;
	head.body_length = size - 1;
	head.seq_number = seq_number;
	head.ms_life_count = ms_timelimit;
	unsigned long   index = sizeof(transport::MessageFixedPart);

	for (int i = 0; i<num; i++) {
		mess[index] = (unsigned char)lens[i];
		index++;
		if (lens[i])
			strcpy((char*)mess + index, names[i]);
		else
			mess[index] = 0;
		index += lens[i]+1;
	}
	head.head_length = index;
	memcpy( (void *)&mess[index], body, size );
	transport::UpdateMessageChecksums(*(transport::MessageFixedPart*)mess);
	Reset();
	VS_TransportMessage::mess = mess;
	setMsg = true;
	return isValid = true;
}


bool VS_TransportMessage::FormReply(const VS_TransportMessage *request,
									const unsigned long ms_timelimit,
									const void *body, const unsigned long size,
									const char *add_string)
{
	if (!request)
		request = this;
	if (request->Type() != transport::MessageType::Request || !request->setMsg)
		return false;
	const char   *loc_our_CID = request->DstCID();
	const char   *loc_our_service = request->DstService();
	const char   *loc_our_user = request->DstUser();
	const char   *loc_add_string = add_string;
	const char   *loc_CID = request->SrcCID();
	const char   *loc_service = request->SrcService();
	const char   *loc_user = request->SrcUser();
	//	const char	 *loc_our_server = request->DstServer();
	//  instead use only our service in Reply!
	const char	 *loc_our_server = g_tr_endpoint_name.c_str();
	const char	 *loc_server = request->SrcServer();
	const unsigned long   loc_seq_number = request->Sequence();

	return Set( 0, loc_seq_number, loc_our_CID, loc_our_service,
		loc_add_string, loc_CID, loc_service, ms_timelimit, body,
		size,loc_user,loc_our_user,loc_server, loc_our_server );
}

bool VS_TransportMessage::CheckMessageIntegrity()
{
	if (!IsValid() || !mess) return false;
	const char *strings[] = {
		SrcCID(), SrcService(), AddString(),
		DstCID(), DstService(), SrcUser(), DstUser(),
		SrcServer(), DstServer()
	};

	const transport::MessageFixedPart* fp = reinterpret_cast<const transport::MessageFixedPart*>(mess);

	for (size_t i = 0; i < (sizeof(strings) / sizeof(const char *)); ++i) {
		const ptrdiff_t msgPos = strings[i] - reinterpret_cast<const char*>(mess);
		if ((msgPos < 0) || (strnlen(strings[i], fp->head_length - msgPos + 1) > (fp->head_length - msgPos))) return false;
	}

	return true;
}

#endif

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
/// \file VS_TransportMessage.h
/// \brief
/// \note
///

#ifndef VS_TRANSPORT_MESSAGE_H
#define VS_TRANSPORT_MESSAGE_H

#include "../Message.h"

class VS_TransportMessage
{
public:
	VS_TransportMessage( void );
	VS_TransportMessage(VS_TransportMessage&& src);
	VS_TransportMessage& operator=(VS_TransportMessage&&src);

	virtual ~VS_TransportMessage( void );
	bool			IsValid( void ) const;

	const char	*SrcCID() const;
	const char	*SrcService() const;
	const char	*AddString() const;
	const char	*DstCID() const;
	const char	*DstService() const;
	const char	*SrcUser() const;
	const char	*DstUser() const;
	const char	*SrcServer() const;
	const char	*DstServer() const;

	bool	SetSrcCID(const char *cid);
	bool	SetSrcService(const char *service);
	bool	SetString(const char *string);
	bool	SetDstCID(const char *cid);
	bool	SetDstService(const char *service);
	bool	SetSrcUser(const char *user);
	bool	SetDstUser(const char *user);
	bool	SetSrcServer(const char *server);
	bool	SetDstServer(const char *server);
	bool	SetTimeLimit(unsigned time_limit);

	bool				IsFromServer() const;
	unsigned long	TimeLimit() const;
	const unsigned char* Body() const;
	size_t BodySize() const;
	unsigned long	Sequence() const;
	unsigned char	Version() const;
	bool				IsRequest() const;
	bool				IsReply() const;
	bool				IsNotify() const;
	transport::MessageType	Type() const;

	size_t Size() const;
	const unsigned char* Data() const { return mess; }

protected:
	const char		*GetName(const unsigned, const unsigned char * = 0) const;
	bool			SetName(const unsigned, const char *);
	void			Reset();
	void			ReleaseMessage();	// ktrushnikov: don't call this, unless you know what you are doing!
	bool			Set(const unsigned long, const unsigned long, const char *, const char *, const char *, const char *, const char *, const unsigned long,
							const void *, const unsigned long,	const char*,const char*, const char*, const char *);
	bool			Set(const unsigned char *);
	bool			FormReply(const VS_TransportMessage *, const unsigned long, const void *, const unsigned long, const char * = 0);
	bool			CheckMessageIntegrity();

	bool			isValid, setMsg;
	unsigned char   *mess;
};

inline uint32_t VS_GetNewSequense()
{
	return transport::Message::GetNewSeqNumber();
}

#endif // VS_TRANSPORT_MESSAGE_H

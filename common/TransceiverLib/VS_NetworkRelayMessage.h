#pragma once
#include <boost/shared_ptr.hpp>
#include <vector>


class VS_NetworkRelayMessageBase
{
protected:
	boost::shared_ptr<std::vector<unsigned char>>	m_mess;
	unsigned long	m_mess_size;
	bool			m_isComplete;
public:
	VS_NetworkRelayMessageBase();
	virtual ~VS_NetworkRelayMessageBase()
	{
	};
	bool Empty() const;
	virtual bool IsComplete() const;
	virtual void Clear();
	virtual bool IsValid() const {return false;}

	virtual bool SetMessage(const boost::shared_ptr<std::vector<unsigned char>> &mess);
	bool SetMessage(const unsigned char *buf, const unsigned long sz);
	boost::shared_ptr<std::vector<unsigned char>>	GetMess() const;
	const unsigned char * GetMess(unsigned long &sz) const;

	virtual unsigned char *GetBufToRead(unsigned long &buf_sz)
	{
		buf_sz = 0;
		return 0;
	};
	virtual void SetReadBytes(const unsigned long received_bytes);
};


class VS_StartControlMess : public VS_NetworkRelayMessageBase
{
public:
	VS_StartControlMess(const unsigned char *auth_buf, const unsigned long sz);
	virtual ~VS_StartControlMess()
	{
	}
};

class VS_StartFrameTransmitterMess : public VS_NetworkRelayMessageBase
{
public:
	VS_StartFrameTransmitterMess(const unsigned char *auth_buf,const unsigned long sz, const char *conf_name);
	VS_StartFrameTransmitterMess(){}
	virtual ~VS_StartFrameTransmitterMess()
	{}
	const char *GetConferenceName() const;
	const unsigned char *GetAuthData(unsigned long &sz) const;
};
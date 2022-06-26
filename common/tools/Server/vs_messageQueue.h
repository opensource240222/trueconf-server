#pragma once
#include "CommonTypes.h"
#include <vector>
#include <queue>

struct VS_PerBuffer;

///////////////////////////////////////////////////////////////////////////////
class VS_GatekeeperMessage
{
public:
	unsigned char*	data;
	uint32_t	    data_sz;
	uint32_t	    ip;
	uint16_t    	port;
};

class VS_GatekeeperMessageQueue
{
public:
	bool GetGatekeeperMessage(	unsigned char* &data, uint32_t &data_sz,
		uint32_t &ip, uint16_t &port, bool isErase = true	);

	bool SetGatekeeperMessage(	const unsigned char* data, const uint32_t data_sz,
						const uint32_t ip, const uint16_t port	);

	~VS_GatekeeperMessageQueue();

protected:

private:
	std::queue<VS_GatekeeperMessage*>	m_queue;
};

///////////////////////////////////////////////////////////////////////////////
class VS_MessageQueue
{
public:
	VS_MessageQueue();
	virtual ~VS_MessageQueue();
	///////////////////////////////////////////////////////////////////////////////
	/// Буфера сообщений подаваемые на вход функции
	/// bool PutMessage(unsigned char *buf, uint32_t sz, uint32_t tear_sz);
	/// ДОЛЖНЫ быть выделены в куче оператором new unsigned char[].
	/// Эти данные в методе будут удалены посредством delete [].
	///////////////////////////////////////////////////////////////////////////////
	virtual bool PutMessage(unsigned char *buf, uint32_t sz, uint32_t channel_id);
	virtual std::unique_ptr<unsigned char[]> GetChannelMessage(uint32_t& sz, const uint32_t channel_id);
	virtual uint32_t GetChannelMessageSize(const uint32_t channel_id);
	virtual void EraseAll();
	bool IsData(uint32_t  channel_id);

	// Don't use this methods ! ! !
	virtual bool PutTearMessage(unsigned char* /*buf*/, uint32_t /*sz*/)
	{	return false;	}
	virtual bool IsExistEntireMessage()
	{	return false;	}
	virtual bool GetEntireMessage( unsigned char* /*buf*/, uint32_t& /*sz*/)
	{	return false;	}

protected:
	struct VS_Message
	{
		std::unique_ptr<unsigned char[]> buf;
		uint32_t sz;
		uint32_t channel_id;
		VS_Message(	std::unique_ptr<unsigned char[]> buf_,
					uint32_t sz_,
					uint32_t channel_id_)
					:buf(std::move(buf_)),sz(sz_),channel_id(channel_id_)
		{
		}
	};
	std::vector<VS_Message> m_Cont;
private:
	std::vector<VS_Message>::iterator m_It;
};
///////////////////////////////////////////////////////////////////////////////
class VS_SIPInputMessageQueue: public VS_MessageQueue
{
public:
	VS_SIPInputMessageQueue(uint32_t _message_size_limit/* = 0*/);
	~VS_SIPInputMessageQueue();

	bool PutMessage(unsigned char *buf, uint32_t sz, uint32_t channel_id) override;
	virtual std::unique_ptr<unsigned char[]> GetChannelMessage(uint32_t &sz, const uint32_t channel_id) override;
	uint32_t GetChannelMessageSize(const uint32_t channel_id) override;

	enum eFilter { FLT_NONE = 0, FLT_STUN = 1, FLT_ZERO_TERMINATED = 2 };
	const static int DEFALUT_FILTERS = FLT_STUN | FLT_ZERO_TERMINATED;
	bool PutMessageWithFilters(unsigned char *buf, uint32_t sz, uint32_t channel_id, int filters = DEFALUT_FILTERS);
	bool MoreThanSizeLimit(uint32_t)const;
protected:

private:
	static int32_t FindContentLenValInBuf(const char *buf, const uint32_t sz);// returns -1 if value was not found, and value in other case (method uses for little parts of big message in other cases uses VS_SIPField_ContentLength class )

	VS_MessageQueue				m_entire_out;
	uint32_t                    m_message_size_limit; // limit for size of massage in bytes. value of '0' means there is no limits, value >0 - max size of message
	int32_t                     m_data_for_delete_size;//data size in next packet, that we need delete. if its val < 0, we need delete after receiving, if val > 0, we need to delete after finding empty line
};
///////////////////////////////////////////////////////////////////////////////
class VS_TearMessageQueue : public VS_MessageQueue
{
public:
	VS_TearMessageQueue();
	virtual ~VS_TearMessageQueue();

	enum VS_TearMessageState{
		e_null,
		e_tearHeader,
		e_tearBody
	};
	///////////////////////////////////////////////////////////////////////////////
	/// Буфера сообщений подаваемые на вход функциям
	/// bool PutTearMessage(unsigned char *buf, uint32_t sz);
	/// bool PutMessage(unsigned char *buf, uint32_t sz, uint32_t tear_sz);
	/// ДОЛЖНЫ быть выделены в куче оператором new unsigned char[].
	/// Эти данные в методе будут удалены посредством delete [].
	///////////////////////////////////////////////////////////////////////////////
	virtual bool PutMessage(unsigned char *buf, uint32_t sz, uint32_t tear_sz);
	/// Для битых сообщений - используйте сборщик PutTearMessage. Все приняты сообщения из
	/// сети с заголовком, типа 0x03, ... 3bytes length..., Body - он будет собирать в целые.

	bool PutTearMessage(unsigned char *buf, uint32_t sz);
	bool GetLastMessageInfo(uint32_t &sz, uint32_t &tear_sz);
	bool IsExistEntireMessage();
	/// Для того, чтобы забрать целую мессагу - исползуйте GetEntireMessage.
	bool GetEntireMessage( unsigned char *buf,
					       uint32_t &sz);
	VS_TearMessageState GetState() const;
protected:
	uint32_t GetLength();
	bool MakeTearHeader(unsigned char *buf, uint32_t sz);
	VS_TearMessageState m_state;
	uint32_t m_tearHeaderSize;
	unsigned char m_tearHeader[4];
private:
	std::vector<VS_Message>::iterator m_it;
};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class VS_OutputMessageQueue: public VS_MessageQueue
{
public:
	VS_OutputMessageQueue();
	~VS_OutputMessageQueue();

	virtual std::unique_ptr<unsigned char[]> GetChannelMessage(uint32_t &sz, const uint32_t channel_id) override;
	uint32_t GetChannelMessageSize(const uint32_t channel_id) override;

	static void EncodeHeader(const unsigned char* in_buf, const unsigned int in_sz, VS_PerBuffer &out);

protected:

private:

};
///////////////////////////////////////////////////////////////////////////////
int TestTearMessageQueue();
///////////////////////////////////////////////////////////////////////////////

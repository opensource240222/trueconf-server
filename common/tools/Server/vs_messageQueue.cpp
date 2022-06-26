#include "vs_messageQueue.h"
#include "SIPParserLib/VS_SIPMessage.h"
#include "SIPParserLib/VS_SIPMetaField.h"
#include "SIPParserLib/VS_SIPField_Via.h"
#include "SIPParserLib/VS_SIPField_ContentLength.h"
#include "std-generic/cpplib/string_view.h"
#include "tools/H323Gateway/Lib/src/VS_AsnBuffers.h"
#include "std-generic/clib/strcasecmp.h"

///////////////////////////////////////////////////////////////////////////////
bool VS_GatekeeperMessageQueue::GetGatekeeperMessage(	unsigned char* &data, uint32_t &data_sz,
											uint32_t &ip, uint16_t &port, bool isErase	)
{
	if ( m_queue.empty() )
		return false;

	VS_GatekeeperMessage* msg = 0;

	msg = m_queue.front();

	if ( data_sz < msg->data_sz)
	{
		data_sz = msg->data_sz;
		return false;
	}
	data_sz = msg->data_sz;
	data = msg->data;
	ip = msg->ip;
	port = msg->port;

	if (isErase)
		m_queue.pop();

	return true;
}

bool VS_GatekeeperMessageQueue::SetGatekeeperMessage(	const unsigned char* data, const uint32_t data_sz,
											const uint32_t ip, const uint16_t port	)
{
	VS_GatekeeperMessage* msg = new VS_GatekeeperMessage;

	msg->data = (unsigned char*) data;
	msg->data_sz = data_sz;
	msg->ip = ip;
	msg->port = port;

	m_queue.push(msg);

	return true;
}

VS_GatekeeperMessageQueue::~VS_GatekeeperMessageQueue()
{
	if ( !m_queue.empty() )
	{
		for(unsigned int i = 0; i < m_queue.size(); i++)
		{
			VS_GatekeeperMessage* msg = m_queue.front();
			if (msg->data) { delete msg->data; msg->data = 0; }
			m_queue.pop();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
bool VS_MessageQueue::PutMessage( unsigned char *buf,
								  uint32_t sz,
								  uint32_t channel_id)
{
	m_Cont.emplace_back(std::unique_ptr<unsigned char[]>(buf), sz, channel_id);
	return true;
}

std::unique_ptr<unsigned char[]> VS_MessageQueue::GetChannelMessage(uint32_t& sz, const uint32_t channel_id)
{
	std::unique_ptr<unsigned char[]> returnValue;
	if (m_Cont.empty())
		return returnValue;
	for(m_It = m_Cont.begin(); m_It != m_Cont.end(); ++m_It)
	{
		if (channel_id == m_It->channel_id)
		{
			returnValue = std::move(m_It->buf);
			sz = m_It->sz;
			m_Cont.erase( m_It );
			return returnValue;
		}
	}
	return returnValue;
}

uint32_t VS_MessageQueue::GetChannelMessageSize(const uint32_t channel_id)
{
	if (m_Cont.empty())
		return false;

	for(m_It = m_Cont.begin(); m_It!=m_Cont.end(); ++m_It)
	{
		if (channel_id == (*m_It).channel_id)
			return (*m_It).sz;
	}

	return 0;
}

bool VS_MessageQueue::IsData(uint32_t  channel_id)
{
	if ( m_Cont.empty() )
		return false;

	for(auto it = m_Cont.begin(); it != m_Cont.end(); ++it)
	{
		if (channel_id == it->channel_id)
			return true;
	}

	return false;
}

void VS_MessageQueue::EraseAll()
{
	m_Cont.clear();
}

VS_MessageQueue::VS_MessageQueue()
{}

VS_MessageQueue::~VS_MessageQueue()
{
	EraseAll();
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
VS_TearMessageQueue::VS_TearMessageQueue():
m_state(e_null),m_tearHeaderSize(0)
{
}
VS_TearMessageQueue::~VS_TearMessageQueue()
{}
bool VS_TearMessageQueue::MakeTearHeader(unsigned char *buf, uint32_t sz)
{
	if (m_tearHeaderSize+sz<=4)
	{
		memcpy(m_tearHeader+m_tearHeaderSize,buf,sz);
		m_tearHeaderSize += sz;
		m_state = e_tearHeader;
	}
    return true;
}
bool VS_TearMessageQueue::PutTearMessage(unsigned char *buf_, uint32_t sz)
{
	unsigned char empty_tpkt[4] = { 3, 0, 0, 4 };
	if (sz==0 || buf_==0)
		return false;
	std::unique_ptr<unsigned char[]> buf(buf_);

	switch(m_state)
	{
	case e_null: ///Нулевая стадия. Нет никаких стартовых данных.
		{
			if (buf[0]!=3)
				return false;

			if (sz<=4)
			{
				if (sz == 4 && memcmp(empty_tpkt, buf.get(), 4) == 0) // empty TPKT packet
				{
					return true;
				}

				if (!MakeTearHeader(buf.get(),sz))
					return false;
			}else
			{
				memcpy(m_tearHeader,buf.get(),4);
				m_tearHeaderSize = 4;
				uint32_t entireSize = GetLength();

				uint32_t content_sz = sz - 4;

				if (entireSize == 4) // empty TPKT packet
				{
					size_t left_sz = sz - 4;
					auto left_data = new unsigned char[left_sz];

					memcpy(left_data, buf.get() + 4, left_sz);
					m_tearHeaderSize = 0;
					return PutTearMessage(left_data, left_sz);
				}

				unsigned char* content = new unsigned char[content_sz];
				memcpy(content, buf.get()+4, sz-4);

				/////
				if (entireSize==sz)
				{
					///entireSize -4 - потому, что длинна заголовков считается.
					///sz -4 - потому, что пакет содержит заголовок .
					if (!PutMessage(content, entireSize-4, sz-4))
						return false;
					m_state = e_null;
					m_tearHeaderSize = 0;
					return true;
				} else if (entireSize>sz)
				{
					///Принятый пакет меньше длинны всего контента.
					if (!PutMessage(content, entireSize-4, content_sz))
						return false;
					m_state =e_tearBody;
					return true;

				} else if (entireSize<sz)
				{
					///Принятый пакет больше длинны всего контента.
					///ВРОДЕ БАГ!!! В ЭТОЙ СЕКЦИИ!
					unsigned char * buffer = new unsigned char[sz-entireSize];
					memcpy(buffer,(content + entireSize-4),sz-entireSize);

					if (!PutMessage(content, entireSize-4, entireSize-4))
						return false;
					m_state = e_null;
					m_tearHeaderSize = 0;
					sz -= entireSize;
					if (sz>65535)
						return false;
					///Рекурсия
					return PutTearMessage( buffer, sz );
				}
			}
			return true;
		}
		break;
	case e_tearHeader:///Стадия - разорваный заголовок. Или полностью только заголовок.
		{
			if (m_tearHeaderSize==4)
			{
				///Заколовок в предидущий раз пришел полностью.
				///Но данных не было.
				uint32_t entireSize = GetLength();
				if (entireSize == 4) // empty TPKT packet
				{
					size_t left_sz = sz - 4;
					auto left_data = new unsigned char[left_sz];

					memcpy(left_data, buf.get() + 4, left_sz);
					m_tearHeaderSize = 0;
					m_state = e_null;
					return PutTearMessage(left_data, left_sz);
				}
				else if (entireSize==(sz+4))
				{
					///Принятый пакет соответствует длинне всего контента.
					///entireSize -4 - потому, что длинна заголовков считается.
					///sz  - потому, что пакет не содержит заголовок .
					if (!PutMessage(buf.release(),entireSize-4,sz))
						return false;
					m_state = e_null;
					m_tearHeaderSize = 0;
					return true;

				} else if (entireSize>(sz+4))
				{
					///Принятый пакет меньше длинны всего контента.
					///m_tearHeaderSize==4
					if (!PutMessage(buf.release(),entireSize-4,sz))
						return false;
					m_state = e_tearBody;
					return true;

				} else if (entireSize<(sz+4))
				{
					///Принятый пакет больше длинны всего контента.
					///m_tearHeaderSize==4
					///Значит за ним идет следующий пакет.
					unsigned char * buffer = new unsigned char[sz+4-entireSize];
					memcpy(buffer,buf.get()+entireSize-4,sz+4-entireSize);
					if (!PutMessage(buf.release(),entireSize-4,entireSize-4))
						return false;
					m_state = e_null;
					m_tearHeaderSize = 0;
					sz -= (entireSize-4);
					if (sz>65535)
						return false;
					///Рекурсия
					return PutTearMessage(buffer,sz);
				}
				///Больше случаев нет.
				return false;

			} else if ((m_tearHeaderSize+sz)<4)
			{
				///Принятый пакет содержит заголовок не полностью.
				if (!MakeTearHeader(buf.get(),sz))
					return false;
				return true;
			} else if ((m_tearHeaderSize+sz)> 4)
			{
				///Принятый пакет содержит заголовок и контент.
				uint32_t shift = m_tearHeaderSize;
				sz-= (4-shift);
				unsigned char * buffer = new unsigned char[sz];
				if (!buffer)
					return false;
				memcpy(buffer,buf.get()+(4-m_tearHeaderSize),sz);

				if (!MakeTearHeader(buf.get(),(4-shift)))
					return false;

				return PutTearMessage(buffer,sz);
			}
			///Больше случаев нет.
			return false;
		}
		break;
	case e_tearBody:
		{
			uint32_t entireSize = 0;
			uint32_t tearSize = 0;
			if (!GetLastMessageInfo(entireSize,tearSize))
				return false;
			if (tearSize==entireSize)
			{
				m_state = e_null;
				m_tearHeaderSize = 0;
				return PutTearMessage(buf.release(),sz);
			}
			if (tearSize+sz==entireSize)
			{
				///Пакет пришел полностью.
				if (!PutMessage(buf.release(),entireSize,sz))
					return false;
				m_state = e_null;
				m_tearHeaderSize = 0;
				return true;

			}else if (tearSize+sz>entireSize)
			{
				///Пакет пришел полностью.+ Следующий пакет.
				unsigned char * buffer = new unsigned char[sz - (entireSize-tearSize)];
				if (!buffer)
					return false;
				memcpy(buffer,buf.get() + (entireSize-tearSize),sz -(entireSize-tearSize));

				if (!PutMessage(buf.release(),entireSize,(entireSize-tearSize)))
					return false;
				sz-=(entireSize-tearSize);
				m_state = e_null;
				m_tearHeaderSize = 0;
				return PutTearMessage(buffer,sz);

			}else if (tearSize+sz<entireSize)
			{
				///Пакет пришел не полностью.
				if (!PutMessage(buf.release(),entireSize,sz))
					return false;
				return true;
			}
			///Больше случаев нет.
			return false;
		}
		break;
	default:
		break;
	}
	return false;
}
uint32_t VS_TearMessageQueue::GetLength()
{
	if ( m_tearHeaderSize==4)
	{
		uint32_t entireSize = m_tearHeader[1];
		entireSize = (entireSize<<8) + m_tearHeader[2];
		entireSize = (entireSize<<8) + m_tearHeader[3];
		return entireSize;
	}
	return 0;
}
bool VS_TearMessageQueue::IsExistEntireMessage()
{
	if (m_Cont.empty())
		return false;

	for (m_it  = m_Cont.begin();
		m_it != m_Cont.end();
		++m_it)
	{
		if (m_it->sz == m_it->channel_id)
		{
			return true;
		}
	}
	return false;
}
bool VS_TearMessageQueue::GetEntireMessage( unsigned char *buf,
					  uint32_t &sz)
{

	if (m_Cont.empty())
		return false;
	for (m_it  = m_Cont.begin();
		m_it != m_Cont.end();
		++m_it)
	{
		if (m_it->sz == m_it->channel_id)
		{
			if (sz < m_it->sz)
			{
				sz = m_it->sz;
				return false;
			}
			if (buf==0 || sz ==0)
			{
				sz = m_it->sz;
				return false;
			}
			memcpy(buf, m_it->buf.get(), m_it->sz);
			m_Cont.erase(m_it);
			return true;
		}
	}
	return false;
}
bool VS_TearMessageQueue::PutMessage(unsigned char *buf_, uint32_t sz, uint32_t tearSize)
{	////////////////////////////////////
	/// Метод добавляет новые мессаги.
	/// В случае, если предидущая мессага не дописана,
	/// то есть разорвана, он дописывает в нее пришедшие данные.
	/// sz - полный размер сообщения, без хидера.
	/// tearSize - пришедший размер.
	////////////////////////////////////
	std::unique_ptr<unsigned char[]> buf(buf_);

	if (sz == tearSize)
	{	/// Дают мессагу целиком.

		if (!VS_MessageQueue::PutMessage(buf.release(),sz,tearSize))
			return false;
		return true;
	} else if (sz > tearSize)
	{	/// Дают разорванную мессагу.
		uint32_t lastSz = 0;
		uint32_t lastTearSz = 0;
		if (m_Cont.empty())
		{
			///Мессаг нету.
			unsigned char * buffer = new unsigned char[sz];
			if (!buffer)
				return false;
			memcpy(buffer,buf.get(),tearSize);

			if (!VS_MessageQueue::PutMessage( buffer,sz,tearSize))
				return false;
			return true;

		} else
		{
			///Мессаги есть.
			if (!GetLastMessageInfo(lastSz,lastTearSz))
				return false;
			if (lastSz != sz)
			{
				/// Какой-то баг, мне пришла мессага, но предидущая еще
				/// не завершена.
				return false;
			}
			if (lastSz == lastTearSz)
			{	/// Предидущая мессага целиковая. Делаем следующую.
				unsigned char * buffer = new unsigned char[sz];
				if (!buffer)
					return false;
				memcpy(buffer,buf.get(),tearSize);

				if (!VS_MessageQueue::PutMessage( buffer,sz,tearSize))
					return false;
				return true;

			} else if (lastSz >= (lastTearSz+tearSize))
			{	/// Мессага не полна.
				m_it = m_Cont.end();
				--m_it;///добавляют же в конец
	            memcpy((m_it->buf.get()+lastTearSz),buf.get(),tearSize);
				m_it->channel_id += tearSize;

				return true;

			} else if (lastSz < (lastTearSz+tearSize))
			{	/// Очень странная ситуация. Такого быть не должно.
				return false;
			}
		}
		///<Больше вариантов нет.
		return false;

	} else if (sz < tearSize)
	{
		return false;
	}
	return false;
}
bool VS_TearMessageQueue::GetLastMessageInfo(uint32_t &sz, uint32_t &tear_sz)
{
	if (m_Cont.empty())
		return false;
	m_it = m_Cont.end();
	--m_it;///добавляют же в конец
	sz = m_it->sz;
	tear_sz = m_it->channel_id;
	return true;
}

VS_TearMessageQueue::VS_TearMessageState VS_TearMessageQueue::GetState() const
{
	return m_state;
}

int TestTearMessageQueue()
{
	///if ok - return 0
	///В каждой стадии несколько вариантов.
	///Их все надо проверить.
	/////////////////////////////////////////
	///Нулевая стадия
	///Разорванный хидер
	///Разорванное сообщение
	/////////////////////////////////////////
	{
	//////////ТЕСТ1///////////////////////////////
	///Нулевая стадия - Разорванный хидер
	///Разорванный хидер - еще раз, обрубка по 4.
	///Разорванное сообщение
	///Разорванное сообщение - оконечное

	VS_TearMessageQueue message;
	unsigned char mess0[] = { 0x03,0x00 };
	unsigned char mess1[] = { 0x00 };
	unsigned char mess2[] = { 0x07,0x01 };
	unsigned char mess3[] = { 0x02,0x03 };

	unsigned mess0_sz = sizeof(mess0);
	unsigned mess1_sz = sizeof(mess1);
	unsigned mess2_sz = sizeof(mess2);
	unsigned mess3_sz = sizeof(mess3);

	unsigned char * m0 = new unsigned char[mess0_sz];
	unsigned char * m1 = new unsigned char[mess1_sz];
	unsigned char * m2 = new unsigned char[mess2_sz];
	unsigned char * m3 = new unsigned char[mess3_sz];

	memcpy(m0,mess0,mess0_sz);
	memcpy(m1,mess1,mess1_sz);
	memcpy(m2,mess2,mess2_sz);
	memcpy(m3,mess3,mess3_sz);



	if (!message.PutTearMessage(m0,mess0_sz))
		return 1;
	if (!message.PutTearMessage(m1,mess1_sz))
		return 2;
	if (!message.PutTearMessage(m2,mess2_sz))
		return 3;
	if (!message.PutTearMessage(m3,mess3_sz))
		return 4;
	unsigned char *asd = new unsigned char[3];
	uint32_t asd_sz = 3;
	if (!message.GetEntireMessage(asd,asd_sz))
		return 101;
	printf("\n\t Test1 1=%d 2=%d 3=%d",asd[0],asd[1],asd[2]);
	delete [] asd;
	asd = 0;
	}
	{
	//////////ТЕСТ2///////////////////////////////
	///Нулевая стадия - Разорванный хидер
	///Разорванный хидер - еще раз, обрубка по 4.
	///Разорванное сообщение
	///Разорванное сообщение - оконечное

	VS_TearMessageQueue message;
	unsigned char mess0[] = { 0x03,0x00 ,0x00,0x07 };
	unsigned char mess1[] = { 0x01 };
	unsigned char mess2[] = { 0x02 };
	unsigned char mess3[] = { 0x03 };

	unsigned mess0_sz = sizeof(mess0);
	unsigned mess1_sz = sizeof(mess1);
	unsigned mess2_sz = sizeof(mess2);
	unsigned mess3_sz = sizeof(mess3);

	unsigned char * m0 = new unsigned char[mess0_sz];
	unsigned char * m1 = new unsigned char[mess1_sz];
	unsigned char * m2 = new unsigned char[mess2_sz];
	unsigned char * m3 = new unsigned char[mess3_sz];

	memcpy(m0,mess0,mess0_sz);
	memcpy(m1,mess1,mess1_sz);
	memcpy(m2,mess2,mess2_sz);
	memcpy(m3,mess3,mess3_sz);



	if (!message.PutTearMessage(m0,mess0_sz))
		return 5;
	if (!message.PutTearMessage(m1,mess1_sz))
		return 6;
	if (!message.PutTearMessage(m2,mess2_sz))
		return 7;
	if (!message.PutTearMessage(m3,mess3_sz))
		return 8;
	unsigned char *asd = new unsigned char[3];
	uint32_t asd_sz = 3;
	if (!message.GetEntireMessage(asd,asd_sz))
		return 102;
	printf("\n\t Test2 1=%d 2=%d 3=%d",asd[0],asd[1],asd[2]);
	delete [] asd;
	asd = 0;

	}
	{
	//////////ТЕСТ3///////////////////////////////
	/// Два целых сообщения.

	VS_TearMessageQueue message;
	unsigned char mess0[] = {0x03,0x00,0x00,0x0b,
		//0x01,0x00,0x32,0x80,0x70,0xd5,0x0e,0x03,0x00,0x00,0x07,0x21,0x80,0xcd,
		0x01,0x02,0x03,0x80,0x70,0xd5,0x0e,0x03,0x00,0x00,0x07,0x21,0x80,0xcd,
		0x03,0x00,0x00,0x06,
		0x20,0x80};

	unsigned mess0_sz = sizeof(mess0);

	unsigned char * m0 = new unsigned char[mess0_sz];

	memcpy(m0,mess0,mess0_sz);


	if (!message.PutTearMessage(m0,mess0_sz))
		return 9;
	uint32_t asd_sz = 30;
	unsigned char *asd = new unsigned char[asd_sz];

	if (!message.GetEntireMessage(asd,asd_sz))
		return 103;

	printf("\n\t Test3 1=%d 2=%d 3=%d",asd[0],asd[1],asd[2]);
	asd_sz = 30;
	if (!message.GetEntireMessage(asd,asd_sz))
		return 113;
	asd_sz = 30;
	if (!message.GetEntireMessage(asd,asd_sz))
		return 123;

	delete [] asd;
	asd = 0;

	}
	{
	//////////ТЕСТ4///////////////////////////////
	///Нулевая стадия - Разорванный хидер
	///Целое сообщение + еще одно

	VS_TearMessageQueue message;
	unsigned char mess0[] = { 0x03,0x00  };
	unsigned char mess1[] = {0x00,0x07, 0x01 ,0x02, 0x03,0x03,0x00,0x00,0x05,0x01};

	unsigned mess0_sz = sizeof(mess0);
	unsigned mess1_sz = sizeof(mess1);

	unsigned char * m0 = new unsigned char[mess0_sz];
	unsigned char * m1 = new unsigned char[mess1_sz];

	memcpy(m0,mess0,mess0_sz);
	memcpy(m1,mess1,mess1_sz);



	if (!message.PutTearMessage(m0,mess0_sz))
		return 10;
	if (!message.PutTearMessage(m1,mess1_sz))
		return 11;
	uint32_t asd_sz = 30;
	unsigned char *asd = new unsigned char[asd_sz];

	if (!message.GetEntireMessage(asd,asd_sz))
		return 104;
	printf("\n\t Test4 1=%d 2=%d 3=%d",asd[0],asd[1],asd[2]);
	asd_sz = 30;
	if (!message.GetEntireMessage(asd,asd_sz))
		return 114;

	delete [] asd;
	asd = 0;

	}
	return 0;
}
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
VS_OutputMessageQueue::VS_OutputMessageQueue()
{

}

VS_OutputMessageQueue::~VS_OutputMessageQueue()
{

}

std::unique_ptr<unsigned char[]> VS_OutputMessageQueue::GetChannelMessage(uint32_t &sz, const uint32_t channel_id)
{
	auto data = VS_MessageQueue::GetChannelMessage(sz, channel_id);
	if (!data)
		return data;

	VS_PerBuffer out;
	EncodeHeader(data.get(), sz, out);

	sz = out.ByteSize();
	data = std::make_unique<unsigned char[]>(sz);
	memcpy(data.get(), out.GetData(), sz);
	return data;
}

uint32_t VS_OutputMessageQueue::GetChannelMessageSize(const uint32_t channel_id)
{
	auto size = VS_MessageQueue::GetChannelMessageSize(channel_id);
	return size > 0 ? size + 4 : 0;
}

void VS_OutputMessageQueue::EncodeHeader(const unsigned char* in_buf, const unsigned int in_sz, VS_PerBuffer &out)
{
	const unsigned int sz = in_sz + 4;		// + header size
	unsigned char hdr[4] = { 3, 0, (unsigned char) ( sz >> 8 ), (unsigned char) sz };

	out.AddBits(hdr, 4 * 8);
	out.AddBits(in_buf, in_sz * 8);
}

/////////////////////////////////////////////////////////////////////////////
VS_SIPInputMessageQueue::VS_SIPInputMessageQueue(uint32_t _message_size_limit): m_message_size_limit(_message_size_limit), m_data_for_delete_size(0)
{	}
VS_SIPInputMessageQueue::~VS_SIPInputMessageQueue()
{	}
bool VS_SIPInputMessageQueue::PutMessage(unsigned char *buf, uint32_t sz, uint32_t channel_id)
{
	if ( (channel_id != e_SIP_CS) && (channel_id != e_SIP_Register) )
		return false;

	if ( !sz )
		return false;

	if (sz == 4 && strncasecmp("jaK", (const char*)buf, 3)==0)		// Linphone send this garbage as keep-alive (http://blog.gmane.org/gmane.comp.voip.sipx.devel/month=20120901)
		return false;												// http://www.sipfoundry.org/forum/-/message_boards/message/622210;jsessionid=6403521B9AE85D84311D57EE5B1FB628

	if (sz == 3 && !strncasecmp("\r\n", (const char*)buf, 2))			// GoIP4 device uses these \r\n\0 'pings' for unknown purpose
		return false;

	unsigned char *origBuf = buf;
	if (!m_Cont.size())
	{
		uint32_t pos(0);
		while(pos<sz)
		{
			if (buf[pos]!=' ' &&
				buf[pos]!='\t' &&
				buf[pos]!='\r' &&
				buf[pos]!='\n')
				break;
			pos++;
		}
		if (pos>0)
		{
			buf += pos;
			sz -= pos;
		}
	}

	// check for possible keep-alive ping
	if (origBuf != buf)
	{
		size_t size = buf - origBuf;
		char *inBetweenPart = new char[size + 1];
		memcpy(inBetweenPart, origBuf, size);
		inBetweenPart[size] = 0;

		char *pos = strstr(inBetweenPart, "\r\n\r\n");
		if (pos)
		{
			char *kaPing = new char[5];
			strcpy(kaPing, "\r\n\r\n");
			m_entire_out.PutMessage((unsigned char*)kaPing, 4, channel_id);
		}

		delete[] inBetweenPart;

		// no message left (only keep-alive ping or pong was received)
		if (!sz)
			return true;
	}

	if (m_data_for_delete_size < 0)
	{
		//ignoring start part of buf, because it is end part of content of previous message, that was deleted.
		//In case when m_data_for_delete_size < 0, m_Cont is guaranteed empty, then message is empty too and will bw consist only of buf
		int need_delete_now = sz < (-m_data_for_delete_size) ? sz : (-m_data_for_delete_size);
		buf += need_delete_now;
		sz -= need_delete_now;
		m_data_for_delete_size += need_delete_now;
		if (sz == 0)
		{
			return false;
		}
	}

	unsigned int size = 0;
	for (unsigned int i=0; i < m_Cont.size(); i++)
	{
		size += m_Cont[i].sz;
	}
	size += sz;

	auto message = std::make_unique<char[]>(size + 1);
	unsigned int index = 0;
	for (unsigned int i=0; i < m_Cont.size(); i++)
	{
		memcpy(message.get() + index, m_Cont[i].buf.get(), m_Cont[i].sz);
		index += m_Cont[i].sz;
	}

	memcpy(message.get()+index, buf, sz);
	unsigned int message_len = index + sz;
	message[message_len] = 0;

	char* pos = strstr(message.get(), "\r\n\r\n");

	if ( !pos )			// New line not found
	{
		bool ret_res(false);
		if (MoreThanSizeLimit(message_len))
		{
			int32_t content_len(FindContentLenValInBuf(message.get(), message_len));
			if (content_len > 0)
			{
				m_data_for_delete_size = content_len;
			}
			EraseAll();
			ret_res = false;
		}
		else
		{
			unsigned char* in_buf = new unsigned char[sz];
			memcpy(in_buf, buf, sz);
			ret_res = true;

			if (!VS_MessageQueue::PutMessage(in_buf, sz, channel_id))
			{
				delete[] in_buf; in_buf = 0;
				ret_res = false;
			}
		}
		return ret_res;
	}

	// > Есть новая строка
	pos += 4;					// Пропустим чистую строку "\r\n\r\n"
	size = pos - message.get();		// Размер SIP-сообщения вместе с чистой строкой, но без контента

	if (m_data_for_delete_size > 0) //in case when we expect end part of headers and content, that we need delete
	{
		//we found empty line, so end part of headers was received, so lets delete it
		unsigned char* other_data = (unsigned char*)message.get() + size;
		uint32_t other_data_sz = message_len - size;
		uint32_t need_delete_now = other_data_sz < m_data_for_delete_size ? other_data_sz : m_data_for_delete_size;
		m_data_for_delete_size -= need_delete_now;
		other_data += need_delete_now;
		other_data_sz -= need_delete_now;
		bool res(false);
		if (other_data_sz > 0)
			res = this->PutMessage((unsigned char*)other_data, other_data_sz, channel_id);

		return res;
	}

	if (MoreThanSizeLimit(size))// in case when SIPMessage size without content more than 65KB
	{
		int32_t content_len(FindContentLenValInBuf(message.get(), size));
		unsigned char* other_data = (unsigned char*)message.get()+ size;
		uint32_t other_data_sz = message_len - size;

		EraseAll();

		if (content_len > 0) //content need to be
		{
			if (other_data_sz >= content_len) //content was received
			{//ignore headers + content
				other_data += content_len;
				other_data_sz -= content_len;
			}
			else //content need to be, we have not received it yet
			{//delete message and save size of data for deleting
				m_data_for_delete_size = -(content_len - other_data_sz);//because other part of content will be at the start of next packet
				EraseAll();
				return false;
			}
		}

		bool res(false);
		if (other_data_sz > 0)
			res = this->PutMessage((unsigned char*)other_data, other_data_sz, channel_id);

		return res;
	}

	VS_SIPMessage msg;
	TSIPErrorCodes err = TSIPErrorCodes::e_null;

	err = msg.Decode(message.get(), size);
	if (TSIPErrorCodes::e_ok == err )					// > SIP-сообщение без контента
	{
		auto meta = msg.GetSIPMetaField();
		if ( !meta || meta->iContentLength && meta->iContentLength->Value() )
		{
			EraseAll();

			unsigned char* other_data = (unsigned char*) message.get()+ size;
			unsigned int other_data_sz = message_len - size;

			if ( other_data_sz > 0 )						// > Первое SIP-сообщение не верное, но есть ещё данные
				this->PutMessage(other_data, other_data_sz, channel_id);

			return false;
		}

		unsigned char* other_data = (unsigned char*) message.get()+ size;
		unsigned int other_data_sz = message_len - size;
		unsigned int entire_sz = size;

		unsigned char* mess = new unsigned char[entire_sz + 1];
		memcpy(mess, message.get(), entire_sz);
		mess[entire_sz] = 0;
		EraseAll();
		if ( !m_entire_out.PutMessage(mess, entire_sz, channel_id) )
		{
			delete[] mess;
			return false;
		}

		if ( other_data_sz > 0 )
			this->PutMessage((unsigned char*) other_data, other_data_sz, channel_id);

		return true;

	}else if ( err == TSIPErrorCodes::e_Content ){								// > SIP-сообщение + Контент

		auto meta = msg.GetSIPMetaField();

		int content_sz = (meta && meta->iContentLength) ? meta->iContentLength->Value() : sz - size;

		if ( !meta || (meta->iVia[0]->ConnectionType() == net::protocol::TCP && !meta->iContentLength) || !content_sz) // > Неверный контент
		{
			EraseAll();
			return false;
		}

		int in_data_sz = message_len - size;					// All Message length (that we recv) - SIPmessage

		if (MoreThanSizeLimit(size + content_sz)) //in case when SIPMessage size with content more than 65KB
		{
			uint32_t other_sz = in_data_sz - content_sz;
			uint32_t entire_sz = message_len - other_sz;
			EraseAll();
			bool res = false;

			if (other_sz > 0)
				res = this->PutMessage((unsigned char*)(message.get()+ entire_sz), other_sz, channel_id);

			return res;
		}

		if ( in_data_sz < content_sz)
		{
			unsigned char* in_buf = new unsigned char[sz];
			memcpy(in_buf, buf, sz);

			if ( !VS_MessageQueue::PutMessage(in_buf, sz, channel_id) )
			{
				delete[] in_buf; in_buf = 0;
				return false;
			}
			return true;
		}else if (in_data_sz == content_sz){
			unsigned char* mess = new unsigned char[message_len + 1];
			memcpy(mess, message.get(), message_len);
			mess[message_len] = 0;
			EraseAll();
			if ( !m_entire_out.PutMessage(mess, message_len, channel_id) )
			{
				delete[] mess;
				return false;
			}
			return true;
		}else{
			unsigned int other_sz = in_data_sz - content_sz;
			unsigned int entire_sz = message_len - other_sz;
			unsigned char* mess = new unsigned char[entire_sz + 1];
			memcpy(mess, message.get(), entire_sz);
			mess[entire_sz] = 0;
			EraseAll();
			if ( !m_entire_out.PutMessage(mess, entire_sz, channel_id) )
			{
				delete[] mess;
				return false;
			}

			// call recurcivly
			this->PutMessage( (unsigned char*)(message.get()+entire_sz), other_sz, channel_id);
			return true;
		}
	}else{

		unsigned char* other_data = (unsigned char*)message.get()+ size;
		unsigned int other_data_sz = message_len - size;

		EraseAll();
		bool res = false;

		if (other_data_sz > 0)
			res = this->PutMessage((unsigned char*)other_data, other_data_sz, channel_id);

		return res;

	}

	return false;
}

bool VS_SIPInputMessageQueue::PutMessageWithFilters(unsigned char *buf, uint32_t sz, uint32_t channel_id, int filters)
{
	if (filters & FLT_STUN) {
		// CLASSIC-STUN header (https://tools.ietf.org/html/rfc3489)
		if (sz >= 20) {
			uint16_t type = *(uint16_t *)&buf[0];
			if (type == 0x0100 || type == 0x0101 || type == 0x1101 || // predefined message types, 2-byte little-endian
				type == 0x0200 || type == 0x0201 || type == 0x1201) {
				return false;
			}
		}

		// STUN header (https://tools.ietf.org/html/rfc5389)
		if (sz >= 16 &&
			!(buf[0] & 0xC0) &&		// first 2 bits are zeroes (0xC0 == 0b11000000)
			!(buf[3] & 0x03)) {		// last 2 bits of length are zeroes (0x03 == 0b00000011)
			uint32_t m = *(uint32_t *)&buf[4];
			if (m == 0x42A41221) {	// Magic Cookie, 4-byte little-endian
				return false;
			}
		}
	}
	if (filters & FLT_ZERO_TERMINATED) {
		for (uint32_t i = 0; i < sz - 1; i++) {
			if (buf[i] == 0x00) {
				return false;
			}
		}
	}
	return PutMessage(buf, sz, channel_id);
}

std::unique_ptr<unsigned char[]> VS_SIPInputMessageQueue::GetChannelMessage(uint32_t &sz, const uint32_t channel_id)
{
	return m_entire_out.GetChannelMessage(sz, channel_id);
}

uint32_t VS_SIPInputMessageQueue::GetChannelMessageSize(const uint32_t channel_id)
{
	return m_entire_out.GetChannelMessageSize(channel_id);
}

bool VS_SIPInputMessageQueue::MoreThanSizeLimit(uint32_t _size)const
{
	return m_message_size_limit > 0 && m_message_size_limit < _size;

}

int32_t VS_SIPInputMessageQueue::FindContentLenValInBuf(const char *buf, const uint32_t sz)
{
	string_view mess_view(buf, sz);
	auto idx_start = mess_view.rfind("Content-Length:");//need find start element
	if (idx_start != mess_view.npos)
	{
		auto idx_end = mess_view.find("\r\n", idx_start);//need find finish element
		if (idx_end > idx_start)
		{
			idx_end += 2;//need add to buffer "\r\n" for correct decode
			VS_SIPBuffer c_l_buff;
			if (c_l_buff.AddData(&buf[idx_start], idx_end - idx_start) == TSIPErrorCodes::e_ok)//add only line with Content-Length
			{
				VS_SIPField_ContentLength sip_field_con_len;
				if (sip_field_con_len.Decode(c_l_buff) == TSIPErrorCodes::e_ok)
				{
					return sip_field_con_len.Value();
				}
			}
		}
	}
	return -1;
}
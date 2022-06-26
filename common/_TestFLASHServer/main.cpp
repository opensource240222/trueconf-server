#include <math.h>
#include <iostream>
#include <winsock2.h>
#include <vector>
#include <stdio.h>

#include "../acs/Connection/VS_ConnectionTCP.h"
#include "../acs/lib/VS_AcsLib.h"
#include "../FLASHParserLib/VS_AMFPacket.h"
#include "../FLASHParserLib/VS_AMFNumber.h"
#include "../FLASHParserLib/VS_AMFString.h"
#include "../FLASHParserLib/VS_AMFNull.h"
#include "../FLASHParserLib/VS_AMFHeader.h"
#include "../FLASHParserLib/VS_AMFObjectsContainer.h"
#include "../FLASHParserLib/VS_AMFVideoFrame.h"
#include "../FLASHParserLib/VS_FLASHSessionData.h"

#include "dump/flash_dump.h"


static VS_FLASHSessionData* g_sess = 0;


VS_ConnectionTCP tcp;

void TestParser();
void TestSpeed();
void TestInteger();

int main(int argc, char* argv[])
{
	//SpeedTest();
	//return 0;

	g_sess = new VS_FLASHSessionData;	

	// Инит сетевой части и прием одного клиента
	{
		if ( !VS_AcsLibInitial() )
		{
			std::cout << "[-] VS_AcsLibInitial()\n";
			return 0;
		}else{
			std::cout << "[+] VS_AcsLibInitial()\n";
		}

		const unsigned long host_sz = 256;
		char host[host_sz];
		if (!VS_GetDefaultHostName(host , 256))
		{
			std::cout << "[-] VS_GetDefaultHostName()\n";
			return 0;
		}else{
			std::cout << "[+] VS_GetDefaultHostName(" << host << ")\n";
		}

		if ( !tcp.Listen(host, 1935) )
		{
			std::cout << "[-] Bind(1935)\n";
			return 0;
		}else{
			std::cout << "[+] Bind(1935)\n";
		}

		unsigned long mills = 100000;
		if ( !tcp.Accept(host, 1935, mills) )
		{
			std::cout << "[-] Accept(1935)\n";
			return 0;
		}else{
			std::cout << "[+] Accept(1935)\n";
		}
	}

	// функция обработка одного пришедшего клиента
	TestParser();

	return 0;
}

// Хендшек пока кривой: сетевая часть вместе с парсером
// в будущем это решиться выносом в отдельной состояние паттерна "Стратегия" (как в гейтвее)
bool Handshake()
{
	unsigned long mills = 100000;

	unsigned long InBuff_sz = 1000000;
	void* InBuff = new char[InBuff_sz];
	memset(InBuff, 0, InBuff_sz);	

	int n_to_read = 1;
	int n_received = 0;

	// Client Request
	{
		n_received = tcp.Receive(InBuff, n_to_read, mills);
		if (n_received <= 0) { std::cout << "[-] Receive(" << n_received << ")\n"; return false; }

		if ( (((char*)InBuff)[0]) != 0x03)
			return false;

		// get 1536 dummy bytes
		n_to_read = 1536;

		n_received = tcp.Receive(InBuff, n_to_read, mills);
		if ((n_received <= 0) || (n_received != n_to_read) ) { std::cout << "[-] Receive(" << n_received << ")\n"; return false; }
	}

	unsigned long outBuff_sz = 1 + 1536;
	void* outBuff = new char[outBuff_sz];
	memset((char*)outBuff+1, 0, outBuff_sz-1);
	((char*)outBuff)[0] = 0x03;

	// Server Response
	{
		if (outBuff_sz != tcp.Send(outBuff, outBuff_sz, mills)) { std::cout << "[-] Send(" << outBuff_sz << ")\n"; return false; }
		if (n_received != tcp.Send(InBuff, n_received, mills)) { std::cout << "[-] Send(" << n_received << ")\n"; return false; }
	}

	// Client Response
	{
		// get 1536 dummy bytes
		n_to_read = 1536;

		n_received = tcp.Receive(InBuff, n_to_read, mills);
		if ((n_received <= 0) || (n_received != n_to_read) ) { std::cout << "[-] Receive(" << n_received << ")\n"; return false; }
	}

	return true;
}

std::vector<VS_AMFVideoFrame*> incom_video_frames;

bool Handshake();
void HandleConnect(VS_AMFPacket* decpkt);
void HandleCreateStream(VS_AMFPacket* decpkt);
void HandlePlay(VS_AMFPacket* decpkt);
void HandlePublish(VS_AMFPacket* decpkt);
void HandleIncomVideo(VS_AMFPacket* decpkt);

void TestParser()
{
	int n_received = 0;
	int n_to_read = 0;

	unsigned long InBuff_sz = 1000000;
	void* InBuff = new char[InBuff_sz];
	memset(InBuff, 0, InBuff_sz);

	if ( !Handshake() ) { std::cout << "[-] Handshake\n"; return ; }

	char tmp;

	// Основной цикл. Выйдет при ошибке в сети
	while (true)
	{
		VS_AMFPacket decpkt(g_sess);

		unsigned long mills = 200000;
		int n = 0;
		n_received = -1;

		// Цикл приема по одному байту (симуляции AsyncIO)
		//   будет заполнять буфер, пока сообщение не сможет раздекодиться
		//   или пришедших данных больше предполагаемого рамера сообщения
		while( !n )
		{			
			decpkt.Clean();
			decpkt.m_header.SetSession(g_sess);
			decpkt.m_body.SetSession(g_sess);

			// TODO: recv 1 bytes???
			int n_tmp = tcp.Receive((void*)&tmp, 1, mills);
			if (n_tmp <= 0) { std::cout << "[-] Receive(n_tmp) " << n_tmp << "\n"; return ; }

			((char*)InBuff)[++n_received] = tmp;

			unsigned int in_bytes = n_received + 1;		// cause it is index (not size)

			n = decpkt.Decode(InBuff, in_bytes);

			unsigned int packet_sz = decpkt.Size();

			if ( (packet_sz > 0) && (in_bytes >= packet_sz) )
			{
				// cleanup previous message
				memset(InBuff, 0, n_received );
				n_received = -1;				
			}
		}

		// сохраняем последний хедер для данного amf_num в данной сессии
		g_sess->AddHeader( &decpkt.m_header );

		if (decpkt.m_header.m_content_type == VIDEO_DATA)
			HandleIncomVideo(&decpkt);

		if (decpkt.m_body.m_cnt.size() > 0)
		{
			VS_AMFString* str = dynamic_cast<VS_AMFString*> (decpkt.m_body.m_cnt[0]);
			if (str != 0)
			{
				if (str->m_str == "connect")
					HandleConnect(&decpkt);
				else if (str->m_str == "createStream")
					HandleCreateStream(&decpkt);
				else if (str->m_str == "play")
					HandlePlay(&decpkt);
				else if (str->m_str == "publish")
					HandlePublish(&decpkt);
			}
		}

		// cleanup
		n = 0;
		memset(InBuff, 0, InBuff_sz);

		decpkt.Clean();
	}
}

void HandleCreateStream(VS_AMFPacket* decpkt)
{
	std::cout << "[+] HandleCreateStream()\n";

	unsigned long mills = 200000;
	int n = 0;
	char* o = new char[10000];


///////////////
	VS_AMFPacket pkt(g_sess);

	pkt.InitHeader(HEADER_12, 3, (content_types_e) 0x11);

	pkt.m_body.AddString("_result");
	pkt.m_body.AddNumber( ((VS_AMFNumber*) decpkt->m_body.m_cnt[1])->m_number.d );		// номер стрима из прошлого пакета (запроса)
	pkt.m_body.AddNull();
	pkt.m_body.AddNumber( ((VS_AMFNumber*) decpkt->m_body.m_cnt[1])->m_number.d );

	n = pkt.Encode((void*) o);

	n = tcp.Send(o, n, mills);
//////////////

	delete [] o; o = 0;
}

// Посылает 1 видео фрейм из очереди принятых пакетов по publish методу
bool SendVideoFrame2()
{
	if ( incom_video_frames.size() <= 0 )
		return false;						// release send

	VS_AMFVideoFrame* frame = incom_video_frames.front();
	if ( !frame )
		return false;

	char* o = new char[10000];
	memset(o, 0 , 10000);
	unsigned long mills = 200000;
	int n = 0;
	int n_total = 0;

	VS_AMFPacket pkt(g_sess);
	pkt.CreateVideoPacket(frame->GetDataPtr(), frame->GetDataSize(), frame->m_frame_type);
	n_total = pkt.Encode(o);

	n = tcp.Send(o, n_total, mills);
	if ( n <= 0 )
		return false;

	incom_video_frames.erase( incom_video_frames.begin() );

	return true;
}

// Посылает 1 видео фрейм из dump-пакетов (по кольцу)
bool SendVideoFrame()
{
	static unsigned int g_video_frame_idx = 0;

	std::cout << "V";

	void* ptr = 0;
	unsigned long mills = 200000;
	int n_total = 0;
	int n = 0;
	char* o = new char[10000];

	unsigned int i = g_video_frame_idx % MAX_VIDEO_FRAMES;

			// clear out buffer
			memset(o, 0 , 10000);
			n_total = 0;

			VS_AMFPacket pkt(g_sess);
			pkt.CreateVideoPacket(packets[i]+1, packets_sz[i]-1);
			n_total = pkt.Encode(o);

			n = tcp.Send(o, n_total, mills);
			if ( n <= 0 )
				return false;

	g_video_frame_idx++;
	delete [] o; o = 0;
	return true;
}

// Посылает 1 обычный аудио фрейм с синусоидой
bool SendAudioFrame()
{
	static double t = 0;
	static unsigned int A = 16000;

	std::cout << "A";

	void* ptr = 0;
	unsigned long mills = 200000;
	int n_total = 0;
	int n = 0;
	char* o = new char[10000];
	int y = 0;

			// clear out buffer
			memset(o, 0 , 10000);
			n_total = 0;

			unsigned int p_sz = 11025/50*2;

			char* data = new char[p_sz];

			ptr = (void*) data;

			for (unsigned f=0; f <= (p_sz/2); f++)
			{
				
				y = (int) (A*sin(t));
				t += 0.628;				

				((short int*)ptr)[f] = (short int) y;
			}

			VS_AMFPacket pkt(g_sess);
			pkt.CreateAudioPacket(data, p_sz/2);
			n_total = pkt.Encode(o);

			n = tcp.Send(o, n_total, mills);
			if ( n <= 0 )
				return false;

	delete [] o; o = 0;
	return true;
}

// Бесконечный цикл посылки фреймов. Выходит по ошибке в сети
void HandlePlay(VS_AMFPacket* decpkt)
{
	std::cout << "[+] HandlePlay()\n";
	std::cout << "Start sending Audio & Video..." << std::endl;

	g_sess->InitMedia();
	g_sess->SetCallerID(decpkt->m_header.m_caller);

	bool res = true;
	while( res )
	{
		if ( !SendVideoFrame2() )
			res = false;

		//if ( !SendAudioFrame() )
		//	res = false;

		Sleep(20);
	}

	std::cout << "Stop sending Audio & Video!" << std::endl;
}

void HandleConnect(VS_AMFPacket* decpkt)
{
	int n = 0;
	unsigned long mills = 200000;

	char* o = new char[10000];


	VS_AMFNumber* amf_num = 0;
	VS_AMFObjectsContainer* amf_cnt = 0;

// Client BW
	VS_AMFPacket pkt(g_sess);

	pkt.InitHeader(HEADER_12, 3, CLIENT);

	char c[] = { 0x26,0x25,0xa0,0x02,0x00,0x00,0x00,0x00 };
	amf_num = new VS_AMFNumber;		amf_num->Init(&c[0]);				pkt.m_body.Add(amf_num);

	n = pkt.Encode((void*) o);

	n = tcp.Send(o, n, mills);

	pkt.Clean();

// Chunk size
	pkt.m_body.SetSession(g_sess);

	pkt.InitHeader(HEADER_12, 2, CHUNK_SIZE);

	pkt.m_body.AddNumber(1000000);

	n = pkt.Encode((void*) o);

	n = tcp.Send(o, n, mills);

	pkt.Clean();

// _resut for connect
	pkt.m_body.SetSession(g_sess);

	pkt.InitHeader(HEADER_12, 3, INVOKE);

	pkt.m_body.AddString("_result");
	pkt.m_body.AddNumber(1);

	amf_cnt = new VS_AMFObjectsContainer;
		amf_cnt->Add("fmsVer", "FMS/3,0,1,123");
		amf_cnt->Add("capabilities", 1);
	pkt.m_body.Add(amf_cnt);

	amf_cnt = new VS_AMFObjectsContainer;
		amf_cnt->Add("level", "status");
		amf_cnt->Add("capabilities", 1);
		amf_cnt->Add("code", "NetConnection.Connect.Success");
		amf_cnt->Add("description", "Connection succeeded");
		amf_cnt->Add("clientid", 1467568292.0000000);
		amf_cnt->Add("objectEncoding", 3);
	pkt.m_body.Add(amf_cnt);	

	n = pkt.Encode((void*) o);

	n = tcp.Send(o, n, mills);

	pkt.Clean();
}

void HandlePublish(VS_AMFPacket* decpkt)
{
	std::cout << "[+] HandlePublish()\n";

	int n = 0;
	unsigned long mills = 200000;

	char* o = new char[10000];

	VS_AMFNumber* amf_num = 0;
	VS_AMFObjectsContainer* amf_cnt = 0;

	VS_AMFPacket pkt(g_sess);

// _resut for connect
	pkt.m_body.SetSession(g_sess);

	pkt.InitHeader(HEADER_12, 3, INVOKE);

	pkt.m_body.AddString("_result");
	pkt.m_body.AddNumber(1);

	amf_cnt = new VS_AMFObjectsContainer;
		amf_cnt->Add("fmsVer", "FMS/3,0,1,123");
		amf_cnt->Add("capabilities", 1);
	pkt.m_body.Add(amf_cnt);

	amf_cnt = new VS_AMFObjectsContainer;
		amf_cnt->Add("level", "status");
		amf_cnt->Add("capabilities", 1);
		amf_cnt->Add("code", "NetConnection.Connect.Success");
		amf_cnt->Add("description", "Connection succeeded");
		amf_cnt->Add("clientid", 1467568292.0000000);
		amf_cnt->Add("objectEncoding", 3);
	pkt.m_body.Add(amf_cnt);	

	n = pkt.Encode((void*) o);

	n = tcp.Send(o, n, mills);

	pkt.Clean();
}

void HandleIncomVideo(VS_AMFPacket* decpkt)
{
	std::vector<VS_AMFBase*>* map = 0;
	map = &(decpkt->m_body.m_cnt);
	if ( map->size() > 0 )
	{
		VS_AMFVideoFrame* frame = dynamic_cast<VS_AMFVideoFrame*> ( (*map)[0] );
		if ( frame )
		{
			VS_AMFVideoFrame* new_frame = new VS_AMFVideoFrame();
			new_frame->CopyFrom(frame);

			incom_video_frames.push_back(new_frame);
		}
	}
}

void TestSpeed()
{
	int n = 0;

	char* o = new char[10000];

	unsigned int NUM = 20000;

	VS_AMFPacket pkt(g_sess);

unsigned long t1 = GetTickCount();
	for(unsigned int i = 0; i < NUM; i++)
	{
	// Encode
		pkt.Clean();

		pkt.InitHeader(HEADER_12, 3, INVOKE);

		pkt.m_body.AddString("_result");
		pkt.m_body.AddNumber(1);
 
		VS_AMFObjectsContainer* amf_cnt = new VS_AMFObjectsContainer;
		amf_cnt->Add("fmsVer", "FMS/3,0,1,123");
		amf_cnt->Add("capabilities", 1);
		pkt.m_body.Add(amf_cnt);

		amf_cnt = new VS_AMFObjectsContainer;
		amf_cnt->Add("level", "status");
		amf_cnt->Add("capabilities", 1);
		amf_cnt->Add("code", "NetConnection.Connect.Success");
		amf_cnt->Add("description", "Connection succeeded");
		amf_cnt->Add("clientid", 1467568292.0000000);
		amf_cnt->Add("objectEncoding", 3);
		pkt.m_body.Add(amf_cnt);	

		n = pkt.Encode((void*) o);
		if ( !n )
			return ;

		pkt.Clean();

	// Decode
		n = pkt.Decode(o, n);

		if ( !n )
			return ;
	}
unsigned long t2 = GetTickCount();

if ((t2 - t1) > 0)
{
	std::cout << "Time: " << t2 - t1 << "\tPackets: " << NUM << "\n";
	std::cout << "Packets/sec: " << (NUM / ((t2 - t1)))*1000 << "\n";
} else {
	std::cout << "Use more packets!!!\n";
}

	delete [] o;
	o = 0;	
}

void TestInteger()
{
	// with start byte = 0x00
	char dump[9] = { 0x00, 0x40,0x08,0x00,0x00,0x00,0x00,0x00,0x00 };

	VS_AMFNumber num;
	num.Decode(&dump, 9);

	printf("Decodec int: %f\n", num.m_number.d);

	// audioCodecs = 1639
	// videoCodecs = 252
}
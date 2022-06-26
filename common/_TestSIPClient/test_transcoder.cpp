#include <queue>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "../Transcoder/VS_IppAudiCodec.h"
#include "../Transcoder/VideoCodec.h"
#include "../Transcoder/RTPPayload.h"
#include "../Transcoder/VS_RTP_Buffers.h"
#include "../Audio/WinApi/dsutil.h"

#include "../Transcoder/VS_Transcoder.h"

//#include <Msacm.h>

static HINSTANCE ippCodecExtLib = 0;
static bool ippCodecExtLibLoaded = false;
AudioCodec* VS_RetriveAudioCodec(int Id, bool isCodec)
{
	if		(Id==VS_ACODEC_GSM610)
		if (isCodec)return new VS_AudioCoderGSM610;
		else		return new VS_AudioDecoderGSM610;
	else {
		if (ippCodecExtLib==0 && !ippCodecExtLibLoaded) {
			ippCodecExtLib = LoadLibrary("CodecsDll");
			ippCodecExtLibLoaded = true;
		}
		if (ippCodecExtLib) {
			typedef void* (*GetCodecType)(int , bool);
			GetCodecType get_codec = (GetCodecType)GetProcAddress(ippCodecExtLib, "GetAudioCodec");
			if (get_codec) {
				return (AudioCodec *)get_codec(Id, isCodec);
			}
		}
		return 0;
	}
}

static HINSTANCE ippVCodecExtLib = 0;
static bool ippVCodecExtLibLoaded = false;
VideoCodec* VS_RetriveVideoCodec(int Id, bool isCodec, unsigned int typeHardware)
{
	if	(Id == VS_VCODEC_H261)
		if (isCodec) return new VS_VideoCoderH261;
		else return new VS_VideoDecoderH261;
	else if	(Id == VS_VCODEC_H263)
		if (isCodec) return new VS_VideoCoderH263;
		else return new VS_VideoDecoderH263;
	else if	(Id == VS_VCODEC_H263P)
		if (isCodec) return new VS_VideoCoderH263P;
		else return new VS_VideoDecoderH263P;
	else if (Id == VS_VCODEC_H264) {
		return new VS_H264VideoCodec(Id, isCodec);
	}
	return 0;	
}

struct queue_item
{
	unsigned char* p;
	unsigned int p_sz;
};



//int main()
//{
//	std::queue<queue_item> q;
//
//	queue_item it;
//	it.p = (unsigned char*) p1;	it.p_sz = p1_sz;	q.push(it);
//	it.p = (unsigned char*) p2;	it.p_sz = p2_sz;	q.push(it);
//	it.p = (unsigned char*) p3;	it.p_sz = p3_sz;	q.push(it);
//	it.p = (unsigned char*) p4;	it.p_sz = p4_sz;	q.push(it);
//	it.p = (unsigned char*) p5;	it.p_sz = p5_sz;	q.push(it);
//
//
//	it.p = (unsigned char*) p10;	it.p_sz = p10_sz;	q.push(it);
//
//
//
//	VS_RTP2VSTranscoder trans;
//	VS_MediaFormat mf_in;
//	mf_in.SetAudio(8000, VS_ACODEC_G728);
//	mf_in.SetVideo(352, 288, VS_VCODEC_H264);
//	VS_MediaFormat mf_out;
//	mf_out.SetAudio(8000, VS_ACODEC_GSM610);
//	mf_out.SetVideo(352,288, VS_VCODEC_XC02);
//
//	trans.Init(&mf_in, &mf_out);
//
//	while( !q.empty() )
//	{
//		queue_item it = q.front();
//		if ( !trans.Add(it.p, it.p_sz, 2) )
//		{
//			printf("if ( !trans.Add(it.p, it.p_sz, 2) )\n");
//			return 0;
//		}
//		q.pop();
//	}
//
//	return 0;
//}





//#include "../acs/Lib/VS_AcsLib.h"
//#include "../acs/connection/VS_ConnectionUDP.h"
//#include "../acs/connection/VS_ConnectionTCP.h"
//
//const char p1[] = { 0x80, 0x64, 0xc0, 0x44, 0x1c, 0xd1, 0x32, 0x46, 0xd6, 0x0f, 0xbe, 0x53, 0x27, 0x42, 0xe0, 0x0c, 0x95, 0xa0, 0x58, 0x25, 0xb0, 0x20, 0x08 };
//const char p2[] = { 0x80, 0x64, 0xc0, 0x45, 0x1c, 0xd1, 0x32, 0x46, 0xd6, 0x0f, 0xbe, 0x53, 0x28, 0xce, 0x0b, 0x88, 0xc0, 0xa8 };
//
//const char msg1_TCP[] = "SIP/2.0 200 OK\r\n"
//						"Via: SIP/2.0/TCP 192.168.0.253:1169;branch=z9hG4bK39AD0A89\r\n"
//						"Call-ID: A50D57078C974EAEE7489B6C6E0AA5E2\r\n"
//						"CSeq: 1 INVITE\r\n"
//						"Contact: <sip:tandberg@192.168.61.138:5060>\r\n"
//						"From: \"sip@pca.ru\" <sip:gw@pca.ru>;tag=45D4548ED3FB0D2B338653F4FE6B9D41\r\n"
//						"To: <sip:tandberg@pca.ru>;tag=bf4d16aaac861acd\r\n"
//						"Content-Type: application/sdp\r\n"
//						"Content-Length: 730\r\n"
//						"\r\n"
//						"v=0\r\n"
//						"o=tandberg 1 1 IN IP4 192.168.61.138\r\n"
//						"s=-\r\n"
//						"c=IN IP4 192.168.61.138\r\n"
//						"b=CT:256\r\n"
//						"t=0 0\r\n"
//						"m=audio 46276 RTP/AVP 15 8 0\r\n"
//						"b=TIAS:64000\r\n"
//						"a=rtpmap:15 G728/8000\r\n"
//						"a=rtpmap:8 PCMA/8000\r\n"
//						"a=rtpmap:0 PCMU/8000\r\n"
//						"a=sendrecv\r\n"
//						"a=maxprate:50.0\r\n"
//						"m=video 46278 RTP/AVP 100 101 34 31\r\n"
//						"b=TIAS:256000\r\n"
//						"a=rtpmap:100 H264/90000\r\n"
//						"a=fmtp:100 profile-level-id=428016;max-mbps=35000;max-fs=3600;max-smbps=323500\r\n"
//						"a=rtpmap:101 H263-1998/90000\r\n"
//						"a=fmtp:101 custom=1280,720,3;custom=1024,576,2;custom=512,288,1;custom=1024,768,4;custom=800,600,3;custom=640,480,2;cif=1;cif4=2;qcif=1;sqcif=1\r\n"
//						"a=rtpmap:34 H263/90000\r\n"
//						"a=fmtp:34 cif=1;cif4=2;qcif=1;sqcif=1\r\n"
//						"a=rtpmap:31 H261/90000\r\n"
//						"a=fmtp:31 cif=1;qcif=1\r\n"
//						"a=sendrecv\r\n"
//						"a=content:main\r\n"
//						"a=label:11\r\n"
//						"a=maxprate:24.0\r\n"
//						;
//unsigned long msg1_TCP_sz = (unsigned int) strlen(msg1_TCP);
//
//int main()
//{
//	
//	unsigned int p1_sz = (unsigned int) strlen(p1);
//	unsigned int p2_sz = (unsigned int) strlen(p2);
//
//	VS_ConnectionTCP TCP;
//	
//	if ( !VS_AcsLibInitial() )
//	{
//		printf("[-] VS_AcsLibInitial()\n");
//		return 0;
//	}else{
//		printf("[+] VS_AcsLibInitial()\n");
//	}
//
//	const unsigned long host_sz = 256;
//	char host[host_sz];
//	if (!VS_GetDefaultHostName(host , 256))
//	{
//		printf("[-] VS_GetDefaultHostName()\n");
//		return 0;
//	}else{
//		printf("[+] VS_GetDefaultHostName()\n");
//	}
//
//	//if ( !TCP.Bind(host, 5060) )
//	//{
//	//	printf("[-] Bind()\n");
//	//	return 0;
//	//}else{
//	//	printf("[+] Bind()\n");
//	//}
//
////	const unsigned long ip = 0xD43523DB;	// "sipnet.ru"
//	const unsigned long ip = 0xC0A83DAF;	// "192.168.61.175"
//
//	unsigned long mills = 20000;
//
//	if ( !TCP.Listen(5060) )
//	{
//		printf("[-] Listen()\n");
//		return 0;
//	}else{
//		printf("[+] Listen()\n");
//	}
//	
//	if ( !TCP.Accept("192.168.61.175", 5060, mills, false) )
//	{
//		printf("[-] Accept()\n");
//		return 0;
//	}else{
//		printf("[+] Accept()\n");
//	}
//
//
//	printf("Recive Inivite...");
//	char buff[65536];
//	int in_bytes = TCP.Receive(buff, 65536, mills);
//	buff[in_bytes] = 0;
//	printf("[OK]\n");
//
//	int out_bytes = 0;
//	out_bytes = TCP.Send((void*) msg1_TCP, msg1_TCP_sz, mills);
//	if (out_bytes  < 1)
//	{
//		TCP.Disconnect();
//		printf("[-] TCP.Send()\n");
//		return 0;
//	}else{
//		printf("[+] TCP.Send()\n");
//	}
//
//	printf("Recive ACK...");
//	memset(buff, 0, 65536);
//	in_bytes = TCP.Receive(buff, 65536, mills);
//	buff[in_bytes] = 0;
//	printf("[OK]\n");
//	//printf("---[ buff:" << in_bytes << " ]---------------\n" << buff << "-------------------------\n");
//
//
//	while(1)
//		Sleep(100);
//
//	return 0;
//}
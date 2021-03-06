#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <deque>

#include <boost/regex.hpp>


#include "../Transcoder/VS_IppAudiCodec.h"
#include "../Transcoder/VideoCodec.h"
#include "../Transcoder/RTPPayload.h"
#include "../Transcoder/VS_RTP_Buffers.h"
#include "../Audio/WinApi/dsutil.h"

#include "../Transcoder/VS_Transcoder.h"


bool str2hex(char* str, int &ret)
{
	int digit[2];

	for (int f = 0; f < 2; f++)
	{
		bool bValid = false;
		if (!bValid && (str[f] == '0')) { digit[f] = 0; bValid = true; }
		if (!bValid && (str[f] == '1')) { digit[f] = 1; bValid = true; }
		if (!bValid && (str[f] == '2')) { digit[f] = 2; bValid = true; }
		if (!bValid && (str[f] == '3')) { digit[f] = 3; bValid = true; }
		if (!bValid && (str[f] == '4')) { digit[f] = 4; bValid = true; }
		if (!bValid && (str[f] == '5')) { digit[f] = 5; bValid = true; }
		if (!bValid && (str[f] == '6')) { digit[f] = 6; bValid = true; }
		if (!bValid && (str[f] == '7')) { digit[f] = 7; bValid = true; }
		if (!bValid && (str[f] == '8')) { digit[f] = 8; bValid = true; }
		if (!bValid && (str[f] == '9')) { digit[f] = 9; bValid = true; }
		if (!bValid && (str[f] == 'a')) { digit[f] = 10; bValid = true; }
		if (!bValid && (str[f] == 'b')) { digit[f] = 11; bValid = true; }
		if (!bValid && (str[f] == 'c')) { digit[f] = 12; bValid = true; }
		if (!bValid && (str[f] == 'd')) { digit[f] = 13; bValid = true; }
		if (!bValid && (str[f] == 'e')) { digit[f] = 14; bValid = true; }
		if (!bValid && (str[f] == 'f')) { digit[f] = 15; bValid = true; }
		if (!bValid) { return false; }
	}

	ret = digit[0] * 16 + digit[1];
	return true;
}

CAviFile g_avi;

void parse_packet(char* packet, VS_RTP2VSTranscoder* &trans)
{
// skip empty line
//	VS_SimpleStr str = "\r\n" + packet;
	char* p = packet;
	//std::deque<int> v;
	char buf[10000];	memset(buf, 0, 10000);
	unsigned int index = 0;
	unsigned int index2 = 0;

	while( (p = strstr(p, "\r\n")) != 0 )
	{
		p+=2;		// skip \r\n
		p+=4;		// skip line number (4-hex values)
		p+=2;		// skip 2 spaces
		index2++;

		for (unsigned int i=0; i < 16; i++)
		{
			if (p[0] != 0x20)		// space
			{
				int x = 0;
				str2hex(p, x);
				buf[index] = x;
				index++;
				//v.push_back(x);
				p+=3;	// skip 2-hex digits & 1 space char
			}
		}
	}

// Skip 42 bytes to get RTP (ethernet+ip+udp)
	//for(unsigned int i=0; i < 42; i++)
	//	v.pop_front();

// Make one packet
	//unsigned int rtp_sz = (unsigned int) v.size();
	//char* rtp = new char[rtp_sz+10000];
	//for(unsigned int i=0; i < rtp_sz; i++)
	//{
	//	rtp[i] = v[i];
	//}

// Add to transcoder
	printf("rtp_sz = %5d, index2=%5d\n", index+1-42, index2);
	if ( !trans || !trans->Add((unsigned char*) buf+42, index-42+1, 2) )
	{
		printf("if ( !trans || !trans->Add(rtp, rtp_sz, 2) )\n");
		return ;
	}

//	if (rtp) { delete[] rtp; rtp = 0; }
}

/*
int main()
{
	VS_RTP2VSTranscoder* trans = new VS_RTP2VSTranscoder;

	VS_MediaFormat mf_in;
	mf_in.SetAudio(8000, VS_ACODEC_G728);
	mf_in.SetVideo(352, 288, VS_VCODEC_H264);
	trans->AddAutoFormat(2, 100, &mf_in);
	mf_in.SetVideo(704, 576, VS_VCODEC_H263P);
	trans->AddAutoFormat(2, 101, &mf_in);

	VS_MediaFormat mf_out;
	mf_out.SetAudio(11025, VS_ACODEC_G723);
	g_avi.Init("gavi.avi", true, false);
	BITMAPINFOHEADER bm;
	memset(&bm, 0, sizeof (BITMAPINFOHEADER));
	bm.biSize = sizeof (BITMAPINFOHEADER);
	bm.biCompression = mf_out.dwVideoCodecFCC;
	bm.biWidth = mf_out.dwVideoWidht;
	bm.biHeight = mf_out.dwVideoHeight;
	bm.biPlanes = 1;
	g_avi.m_fps = 30.;
	g_avi.SetFormat(&bm);

	mf_in.SetVideo(0, 0 ,0);

	trans->Init(&mf_in, &mf_out);

	FILE* fp = fopen("1.txt", "rb");
	if ( !fp )
		return 0;

	unsigned int size = 4000000;
	char* buf = new char[size];	memset(buf, 0xaf, size);

	fread(buf, size, 1, fp);


	char* p = 0;
	char* packet = buf;
	
	unsigned int num_packet = 0;
	char* new_packet = 0;
	VS_VS_InputBuffer in;
	while( (new_packet = strstr(packet, "\x0C")) != 0 )
	{
		unsigned int size = (unsigned int) (new_packet - packet);

		// delete last "\r\n"
		size -= 2;

		p = new char[size + 1];		 memset(p, 0xfa, size + 1);
		memcpy(p, packet, size);

// Parse Ethernet-packet
		parse_packet(p, trans);
		unsigned char aviBuff[65535];		memset(aviBuff, 0, 65535);
		unsigned long aviBuff_sz = 65535;
		int type = 2;
		while(trans->Get(aviBuff, aviBuff_sz, type))
		{
			in.Add(aviBuff, aviBuff_sz, 2);
			unsigned long VideoInterval = 0;
			bool key = false;
			memset(aviBuff, 0, 65535);
			if (in.Get(aviBuff, aviBuff_sz, type, VideoInterval, &key) >= 0)
				g_avi.WriteVideo(aviBuff, aviBuff_sz, key, VideoInterval);
		}
		num_packet++;

		if (p) { delete[] p; p = 0; }

		// skip 0x0C
		new_packet++;

		packet = new_packet;
	}


	g_avi.Release();

	if (trans) { delete trans; trans = 0; }
	if (buf) { delete[] buf; buf = 0; }

	return 0;
}
*/
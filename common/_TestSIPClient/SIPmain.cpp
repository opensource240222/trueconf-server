///*#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>*/
#include <boost/regex.hpp>
#include <string>
//
/////////// VideoPort -> ACS ///////////////////////
#include "../acs/connection/VS_ConnectionUDP.h"
#include "../acs/connection/VS_ConnectionTCP.h"
#include "../acs/Lib/VS_AcsLib.h"
////////////////////////////////////////////////////
//
//#include "VS_NetReactor.h"
//#include "VS_NetConnection.h"
//
#include "../SIPParserLib/VS_Const.h"
#include "../SIPParserLib/VS_SIPRequest.h"
#include "../SIPParserLib/VS_SIPResponse.h"
#include "../SIPParserLib/VS_SIPMessage.h"
//#include "../SIPParserLib/VS_SIPField_AAA_MD5.h"
#include "../SIPParserLib/VS_SIPObjectFactory.h"
#include "../SIPParserLib/VS_SDPObjectFactory.h"
//
#include "../Servers/SingleGatewayLib/VS_SIPServer.h"
#include "../Servers/SingleGatewayLib/VS_SIPCall.h"
#include "../Servers/SingleGatewayLib/VS_RegistrationMgr.h"
#include "../Servers/SingleGatewayLib/VS_CallSignallingMgr.h"
#include "../Servers/SingleGatewayLib/VS_RASRegistrationImpl.h"
#include "../Servers/SingleGatewayLib/VS_TerminalAbstractFactory.h"
////#include "../Servers/SingleGatewayLib/VS_SIPServer.h"
////#include "../SIPParserLib/VS_SDPObjectFactory_Ext.h"
//
//// SIP AAA
///*#include "../SIPParserLib/VS_SIPField_WWWAuthenticate.h"
//#include "../SIPParserLib/VS_SIPField_AAA_MD5.h"*/
//
//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//
//#include <windows.h>
//
//
//#include <iostream>
//#include <conio.h>
//
// char MY_SIP[] ="INVITE sip:192.168.61.57 SIP/2.0\r\n"
//				"Via: SIP/2.0/UDP 192.168.61.141:5060\r\n"
//				"Max-Forwards: 70\r\n"
//				"From: Alice <sip:alex@vp41.pca.ru>\r\n"
//				"To: Bob <sip:VP_2006@sipnet.ru>\r\n"
//				"Call-ID: 3848276298220188511@visicron.ru\r\n"
//				"CSeq: 1 INVITE\r\n"
//				"Contact: <sip:192.168.61.141:5060>\r\n"
//				"Content-Type: application/sdp\r\n"
//				"Content-Length: 142\r\n"
//				"\r\n"
//				"v=0\r\n"
//				"o=alice 2890844526 2890844526 IN IP4 192.168.61.141\r\n"
//				"s=-\r\n"
//				"c=IN IP4 192.168.61.141\r\n"
//				"t=0 0\r\n"
//				"m=audio 49172 RTP/AVP 0\r\n"
//				"a=rtpmap:0 PCMU/8000\r\n";
//unsigned int MY_SIP_sz = (unsigned int) strlen(MY_SIP);
//
//
//const char AAA[] = 
//					"SIP/2.0 401 Authentication required\r\n"
//					"Via: SIP/2.0/UDP 81.90.12.148:5060;rport=5060;branch=z9hG4bKF4406B759F7A4F3A9137CB9D15AA6D91\r\n"
//					"From: Fedor <sip:VP_2006@sipnet.ru>;tag=2231885316\r\n"
//					"To: Fedor <sip:VP_2006@sipnet.ru>;tag=62517321C4C25F41\r\n"
//					"Contact: Fedor <sip:VP_2006@81.90.12.148:5060>\r\n"
//					"Call-ID: F9B26D2F003F4F75A3856EEE59328587@sipnet.ru\r\n"
//					"CSeq: 35180 REGISTER\r\n"
///*					"WWW-Authenticate: Digest "
//									"realm=\"biloxi.com\","
//									"nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\","
//									"uri=\"sip:bob@biloxi.com\","
////									"username=\"bob\","
//									"opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"\n"
//*/					"WWW-Authenticate: Digest "
//									"realm=\"etc.tario.ru\","
//									"nonce=\"EAF9E8258687661EABD2\","
//									"cnonce=\"3007A48D72444B37AA30FE8A6C2FBAC5\","
//									"opaque=\"opaqueData\","
//									"qop=\"auth\","
//									"nc=00000001,"
//									"uri=\"sip:sipnet.ru\","
//									"username=\"VP_2006@sipnet.ru\","
//									"algorithm=MD5,stale=TRUe\r\n"
////					"WWW-Authenticate: NTLM realm=\"etc.tario.ru\",qop=\"auth\",targetname=\"etc.tario.ru\",opaque=\"ABE5DC9B\"\n"
//					"Server: CommuniGatePro/5.0.9\r\n"
//					"Content-Type: application/sdp\r\n"
//					"Content-Length: 0\r\n";
//
//unsigned int AAA_sz = (unsigned int) strlen(AAA);
//
//const char AAA_AUTH[] =
//					"SIP/2.0 401 Authentication required\r\n"
//					"Via: SIP/2.0/UDP 81.90.12.148:5060;rport=5060;branch=z9hG4bKF4406B759F7A4F3A9137CB9D15AA6D91\r\n"
//					"From: Fedor <sip:VP_2006@sipnet.ru>;tag=2231885316\r\n"
//					"To: Fedor <sip:VP_2006@sipnet.ru>;tag=62517321C4C25F41\r\n"
//					"Contact: Fedor <sip:VP_2006@81.90.12.148:5060>\r\n"
//					"Call-ID: F9B26D2F003F4F75A3856EEE59328587@sipnet.ru\r\n"
//					"CSeq: 35180 INVITE\r\n"
//					"WWW-Authenticate: Digest "
//									"realm=\"biloxi.com\","
//									"nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\","
//									"cnonce=\"0a4f113b\","
//									"opaque=\"opaqueData\","
//									"qop=\"auth\","
//									"nc=00000001,"
//									"uri=\"sip:bob@biloxi.com\","
//									"username=\"bob\","
//									"algorithm=MD5-sess\r\n"
//					"Server: CommuniGatePro/5.0.9\r\n"
//					"Content-Type: application/sdp\r\n"
//					"Content-Length: 0\r\n";
//				;
//
//unsigned int AAA_AUTH_sz = (unsigned int) strlen(AAA_AUTH);
//
//const char SIP[] = 
//				    "INVITE sip:192.168.61.110 SIP/2.0\r\n"
////					"SIP/2.0 180 Ringing\r\n"
//					"Via: SIP/2.0/UDP 192.168.61.113:1000;rport;branch=z9hG4bKc0a83d710000001a4355014200006c8f00000007\r\n" 
//					"Via: SIP/2.0/UDP 192.168.61.114:5060;rport;branch=z9hG4bK665\r\n"
//				    "Content-Length: 367\r\n" 
//					"To: <sip:192.168.61.110>\r\n"
//					"Max-Forwards: 70\r\n"
//
//					"Route: <sip:ABCEB8BF.signer.cgatepro;lr>\r\n"
//					"Route: <sip:81.200.20.20.1130.nat.cgatepro;lr>\r\n"
//					"Route: <sip:Matv@sipnet.ru:7675;maddr=10.10.251.12;transport=tcp>\r\n"
//
//				    "Contact: <sip:192.168.61.113:1000>\r\n"
//					"Call-ID: 308774BE-0DA0-4DF2-A4F3-DD5528BF0BE9@192.168.61.113\r\n" 
//					"Content-Type: application/sdp\r\n" 
//					"CSeq: 1 INVITE\r\n" 
//				    "From: \"Kostya Trushnikov\"<sip:192.168.61.113>;tag=20-F811719658\r\n" 
//					"User-Agent: SJphone/1.60.289a (SJ Labs)\r\n"
//					"\r\n"
//		// SDP
//					"v=0\r\n"
//					"o=- 3338633154 3338633154 IN IP4 192.168.61.113\r\n"
//					"s=SJphone\r\n"
//					"c=IN IP4 192.168.61.113\r\n"
//					"t=0 0\r\n"
//					"a=direction:active\r\n"
//					"m=audio 49154 RTP/AVP 18 3 97 98 8 0 101\r\n"
//					"a=rtpmap:18 G729/8000\r\n"
//					"a=rtpmap:3 GSM/8000\r\n"
//					"a=rtpmap:97 iLBC/8000\r\n"
//					"a=rtpmap:98 iLBC/8000\r\n"
//					"a=fmtp:98 mode=20\r\n"
//					"a=rtpmap:8 PCMA/8000\r\n"
//					"a=rtpmap:0 PCMU/8000\r\n"
//					"a=rtpmap:101 telephone-event/8000\r\n"
//					"a=fmtp:101 0-11,16\r\n"
//				;
//unsigned int SIP_sz = (unsigned int) strlen(SIP);
//
//const char msg1[] =
//					"REGISTER sip:sipnet.ru SIP/2.0\r\n"
//					"Via: SIP/2.0/UDP 81.90.12.148:5060;branch=z9hG4bKF4\r\n"
//					"From: Fedor<sip:VP_2006@sipnet.ru>;tag=2231885318\r\n"
//					"To: Fedor<sip:VP_2006@sipnet.ru>\r\n"
//					"Contact: \"Fedor\"<sip:VP_2006@81.90.12.148:5060>\r\n"
//					"Call-ID: F9B26D2F003F4F75A3856EE@sipnet.ru\r\n"
//					"CSeq: 351 REGISTER\r\n"
//					"Max-Forwards: 70\r\n"
//					"Content-Length: 0\r\n\r\n"
//				;
//unsigned int msg1_sz = (unsigned int) strlen(msg1);
//
//const char msg2[] =
//					"REGISTER sip:sipnet.ru SIP/2.0\r\n"
//					"Via: SIP/2.0/UDP 81.90.12.148:5060;branch=z9hG4bKF5\r\n"
//					"From: Fedor<sip:VP_2006@sipnet.ru>;tag=2231885318\r\n"
//					"To: Fedor<sip:VP_2006@sipnet.ru>\r\n"
//					"Contact: \"Fedor\"<sip:VP_2006@81.90.12.148:5060>\r\n"
//					"Call-ID: F9B26D2F003F4F75A3856EE@sipnet.ru\r\n"
//					"CSeq: 351 REGISTER\r\n"
//					"Max-Forwards: 70\r\n"
//					"Authorization: Digest "
//							"username=\"VP_2006@sipnet.ru\","
//							"realm=\"etc.tario.ru\","
//							"nonce=\"EAF9E8258687661EABD2\","
//							"response=\"efe815530019871b98c03e76b8bc44ee\","
//							"uri=\"sip:sipnet.ru\","
//							"algorithm=MD5,"
//							"opaque=\"opaqueData\","
//							"qop=auth,"
//							"cnonce=\"3007A48D72444B37AA30FE8A6C2FBAC5\","
//							"nc=00000001\r\n"
//					"Content-Length: 0\r\n\r\n"
//				;
//unsigned int msg2_sz = (unsigned int) strlen(msg2);
//
//const char msg1_OnDO[] =
//					"REGISTER sip:192.168.61.60 SIP/2.0\r\n"
//					"Via: SIP/2.0/UDP 192.168.61.53:5060;branch=z9hG4bKF4\r\n"
//					"From: Fedor<sip:vasya@192.168.61.60>;tag=2231885318\r\n"
//					"To: Fedor<sip:vasya@192.168.61.60>\r\n"
//					"Contact: \"Fedor\"<sip:vasya@192.168.61.60:5060>\r\n"
//					"Call-ID: F9B26D2F003F4F75A3856EE@sipnet.ru\r\n"
//					"CSeq: 351 REGISTER\r\n"
//					"Max-Forwards: 70\r\n"
//					"Content-Length: 0\r\n\r\n"
//				;
//unsigned int msg1_OnDO_sz = (unsigned int) strlen(msg1_OnDO);
//
//const char msg2_OnDO[] =
//					"REGISTER sip:192.168.61.60 SIP/2.0\r\n"
//					"Via: SIP/2.0/UDP 192.168.61.53:5060;branch=z9hG4bKF5\r\n"
//					"From: Fedor<sip:vasya@192.168.61.60>;tag=2231885318\r\n"
//					"To: Fedor<sip:vasya@192.168.61.60>\r\n"
//					"Contact: \"Fedor\"<sip:vasya@192.168.61.60:5060>\r\n"
//					"Call-ID: F9B26D2F003F4F75A3856EE@sipnet.ru\r\n"
//					"CSeq: 351 REGISTER\r\n"
//					"Max-Forwards: 70\r\n"
//					"Authorization: Digest "
//							"username=\"vasya\","
//							"realm=\"192.168.61.60\","
//							"nonce=\"EAF9E8258687661EABD2\","
//							"response=\"efe815530019871b98c03e76b8bc44ee\","
//							"uri=\"sip:192.168.61.60\","
//							"algorithm=MD5,"
//							"opaque=\"opaqueData\","
//							"qop=auth,"
//							"cnonce=\"3007A48D72444B37AA30FE8A6C2FBAC5\","
//							"nc=00000001\r\n"
//					"Content-Length: 0\r\n\r\n"
//				;
//unsigned int msg2_OnDO_sz = (unsigned int) strlen(msg2_OnDO);
//
//const char msg1_TCP[] =
//					"REGISTER sip:sipnet.ru SIP/2.0\r\n"
//					"Via: SIP/2.0/TCP 81.90.12.148:5060;branch=z9hG4bKF1\r\n"
//					"From: Fedor<sip:VP_2006@sipnet.ru>;tag=2231885318\r\n"
//					"To: Fedor<sip:VP_2006@sipnet.ru>\r\n"
//					"Contact: \"Fedor\"<sip:VP_2006@81.90.12.148:5060>\r\n"
//					"Call-ID: F9B26D2F003F4F75A3856EE@sipnet.ru\r\n"
//					"CSeq: 351 REGISTER\r\n"
//					"Max-Forwards: 70\r\n"
//					"Content-Length: 0\r\n\r\n"
//				;
//unsigned int msg1_TCP_sz = (unsigned int) strlen(msg1_TCP);
//
//const char msg2_TCP[] =
//					"REGISTER sip:sipnet.ru SIP/2.0\r\n"
//					"Via: SIP/2.0/TCP 81.90.12.148:5060;branch=z9hG4bKF5\r\n"
//					"From: Fedor<sip:VP_2006@sipnet.ru>;tag=2231885318\r\n"
//					"To: Fedor<sip:VP_2006@sipnet.ru>\r\n"
//					"Contact: \"Fedor\"<sip:VP_2006@81.90.12.148:5060>\r\n"
//					"Call-ID: F9B26D2F003F4F75A3856EE@sipnet.ru\r\n"
//					"CSeq: 351 REGISTER\r\n"
//					"Max-Forwards: 70\r\n"
//					"Authorization: Digest "
//							"username=\"VP_2006@sipnet.ru\","
//							"realm=\"etc.tario.ru\","
//							"nonce=\"EAF9E8258687661EABD2\","
//							"response=\"efe815530019871b98c03e76b8bc44ee\","
//							"uri=\"sip:sipnet.ru\","
//							"algorithm=MD5,"
//							"opaque=\"opaqueData\","
//							"qop=auth,"
//							"cnonce=\"3007A48D72444B37AA30FE8A6C2FBAC5\","
//							"nc=00000001\r\n"
//					"Content-Length: 0\r\n\r\n"
//				;
//unsigned int msg2_TCP_sz = (unsigned int) strlen(msg2_TCP);
//
///*int TestSIP(const char* aRawData, int aSize)
//{
//	std::cout << "\n\n==================[ SIP ]==================\n";
//
//	int err = e_null;
//
//	std::cout << "1). Decode: ";
//	VS_SIPMessage SIPMessage;
//	err = SIPMessage.Decode(aRawData, aSize);
//	if (err != e_ok)
//	{
//		std::cout << "[Error:" << err << "]\n";
//		return err;
//	}
//	std::cout << "[OK]\n";
//
//	std::cout << "2). Encode ";
//	VS_SIPBuffer SIPEncodeBuffer;
//	VS_SIPRequest* req;
//	VS_SIPResponse* resp;
//
//	req = (VS_SIPRequest*) SIPMessage;
//	resp = (VS_SIPResponse*) SIPMessage;
//    
//	if (req)
//	{
//		std::cout << "Request: ";
//		err = req->Encode(SIPEncodeBuffer);
//		if (err != e_ok)
//		{
//			std::cout << "[Error:" << err << "]\n";
//			return err;
//		}
//		std::cout << "[OK]\n";
//	}else{
//		if (resp)
//		{
//			std::cout << "Response: ";
//			err = resp->Encode(SIPEncodeBuffer);
//			if (err != e_ok)
//			{
//				std::cout << "[Error:" << err << "]\n";
//				return err;
//			}
//			std::cout << "[OK]\n";
//		}else{
//			std::cout << " [Non Req/Resp Message]\n";
//			return e_null;
//		}
//	}
//
//	VS_SIPMessage NewSIPmsg;
//    std::cout << "3). Decode: ";
//
//	char* ptr = 0;
//	unsigned int sz = SIPEncodeBuffer.GetWriteIndex();
//
//	SIPEncodeBuffer.GetDataAlloc(ptr, sz);
//
//	std::cout << "\n\n" << ptr << "\n\n";
//
//	err = NewSIPmsg.Decode(ptr, sz);
//
//	delete ptr;
//
//	if (err != e_ok)
//	{
//		std::cout << "[Error:" << err << "]\n";
//		return err;
//	}
//	std::cout << "[OK]\n";	
//
//	std::cout << "===========================================\n";
//
//	delete VS_SIPObjectFactory::Instance();
//	delete VS_SDPObjectFactory::Instance();
//	delete VS_SDPObjectFactory_Ext::Instance();
//
//	return e_ok;
//}*/
//
///*int TestAAA(const char* aRawData, int aSize)
//{
//	std::cout << "\n\n===[ SIP -> AAA ]===\n";
//
//	int err = e_null;
//
//	std::cout << "1). Decode ";
//	VS_SIPMessage SIPMessage;
//	err = SIPMessage.Decode(aRawData, aSize);
//	if (err != e_ok)
//	{
//		std::cout << "[Error:" << err << "]\n";
//		return err;
//	}
//	std::cout << "[OK]\n";
//
//	std::cout << "Test MD5: ";
//	{
//		if ( !SIPMessage.iSIPMeta->iCSeq || !SIPMessage.iSIPMeta->iWWWAuthenticate )
//		{
//			std::cout << "NO \"CSeq\" OR \"WWWAuthenticate\" HEADERS FOUND!!!\n";
//			return false;
//		}
//
//		int MessageType = SIPMessage.iSIPMeta->iCSeq->GetType();
//		VS_BaseField* aaa = SIPMessage.iSIPMeta->iWWWAuthenticate->GetAAA();
//
//		VS_SIPField_AAA_MD5* md5 = dynamic_cast<VS_SIPField_AAA_MD5*> (aaa);
//		if ( md5 )
//		{
//			unsigned char response[32 + 1];		// We add '\0' for std::cout
//
//			VS_SIPBuffer theContent;
//
//			md5->MD5("zanzibar", MessageType, &theContent, response);
//
// 			response[32] = 0;
//			std::cout << response << "\n";
//		}
//		std::cout << "\n";
//	}
//
//	std::cout << "2). Encode ";
//	VS_SIPBuffer SIPEncodeBuffer;
//	VS_SIPRequest* req;
//	VS_SIPResponse* resp;
//
//	req = (VS_SIPRequest*) SIPMessage;
//	resp = (VS_SIPResponse*) SIPMessage;
//    
//	if (req)
//	{
//		std::cout << "Request: ";
//		err = req->Encode(SIPEncodeBuffer);
//		if (err != e_ok)
//		{
//			std::cout << "[Error:" << err << "]\n";
//			return err;
//		}
//		std::cout << "[OK]\n";
//	}else{
//		if (resp)
//		{
//			std::cout << "Response: ";
//			err = resp->Encode(SIPEncodeBuffer);
//			if (err != e_ok)
//			{
//				std::cout << "[Error:" << err << "]\n";
//				return err;
//			}
//			std::cout << "[OK]\n";
//		}else{
//			std::cout << " [Non Req/Resp Message]\n";
//			return e_null;
//		}
//	}
//
//	VS_SIPMessage NewSIPmsg;
//    std::cout << "3). Decode: ";
//
//	char* ptr = 0;
//	unsigned int sz = SIPEncodeBuffer.GetWriteIndex();
//
//	SIPEncodeBuffer.GetDataAlloc(ptr, sz);
//
////	std::cout << "\n\n" << ptr << "\n\n";
//
//	err = NewSIPmsg.Decode(ptr, sz);
//
//	delete ptr;
//
//	if (err != e_ok)
//	{
//		std::cout << "[Error:" << err << "]\n";
//		return err;
//	}
//	std::cout << "[OK]\n";	
//
//	std::cout << "===============\n";
//
//	delete VS_SIPObjectFactory::Instance();
//	delete VS_SDPObjectFactory::Instance();
//	delete VS_SDPObjectFactory_Ext::Instance();
//
//	return 0;
//}*/
//
//int TestSlot();
//
///*int TestNetUDP()
//{
//	VS_ConnectionUDP UDP;
//	
//	if ( !VS_AcsLibInitial() )
//	{
//		std::cout << "[-] VS_AcsLibInitial()\n";
//		return 0;
//	}else{
//		std::cout << "[+] VS_AcsLibInitial()\n";
//	}
//
//	const unsigned long host_sz = 256;
//	char host[host_sz];
//	if (!VS_GetDefaultHostName(host , 256))
//	{
//		std::cout << "[-] VS_GetDefaultHostName()\n";
//		return 0;
//	}else{
//		std::cout << "[+] VS_GetDefaultHostName()\n";
//	}
//
//	if ( !UDP.Bind(host, 5060) )
//	{
//		std::cout << "[-] Bind()\n";
//		return 0;
//	}else{
//		std::cout << "[+] Bind()\n";
//	}
//
//	const unsigned long ip = 0xD43523DB;	// "sipnet.ru"
//	//const unsigned long ip = 0xC0A83D3C;	// "192.168.61.60"
//
//	int out_bytes = UDP.SendTo((void*) msg1, msg1_sz, ip, 5060);
//	if (out_bytes < 1)
//	{
//		std::cout << "[-] UDP.SendTo()\n";
//		return 0;
//	}else{
//		std::cout << "[+] UDP.SendTo()\n";
//	}
//
//	std::cout << "Recive...";
//	char buff[65536];
//	int in_bytes = UDP.Receive(buff, 65536);
//	buff[in_bytes] = 0;
//	std::cout << "[OK]\n";
//	std::cout << "---[ buff:" << in_bytes << " ]---------------\n" << buff << "-------------------------\n";
//
//// Only for OnDO SIP Server
//	//std::cout << "Recive 2...";
//	//in_bytes = UDP.Receive(buff, 65536);
//	//buff[in_bytes] = 0;
//	//std::cout << "[OK]\n";
//	//std::cout << "---[ buff:" << in_bytes << " ]---------------\n" << buff << "-------------------------\n";
//
//	VS_SIPMessage income1;
//	int err = income1.Decode(buff, in_bytes);
//	if ( err != e_ok )
//	{
//		std::cout << "[-] income1.Decode()\n";
//		return 0;
//	}else{
//		std::cout << "[+] income1.Decode()\n";
//	}
//
//	if ( !income1.iSIPMeta )
//	{
//		std::cout << "[!] !income1.iSIPMeta\n";
//		return 0;
//	}
//
//	if ( !income1.iSIPMeta->iWWWAuthenticate )
//	{
//		std::cout << "[!] !income1.iSIPMeta->iWWWAuthenticate\n";
//		return 0;
//	}
//
//	VS_SIPField_AAA_MD5* income1_md5 = dynamic_cast<VS_SIPField_AAA_MD5*> ( income1.iSIPMeta->iWWWAuthenticate->GetAAA() );
//	if ( !income1_md5 )
//	{
//		std::cout << "[!] !income1_md5\n";
//		return 0;
//	}
//
//	VS_SIPMessage out2;
//	err = out2.Decode(msg2, msg2_sz);
//	if ( err != e_ok )
//	{
//		std::cout << "[-] out2.Decode()\n";
//		return 0;
//	}else{
//		std::cout << "[+] out2.Decode()\n";
//	}
//
//	if ( !out2.iSIPMeta )
//	{
//		std::cout << "[!] !out2.iSIPMeta\n";
//		return 0;
//	}
//
//	if ( !out2.iSIPMeta->iAuthorization )
//	{
//		std::cout << "[!] !out2.iSIPMeta->iAuthorization\n";
//		return 0;
//	}
//
//	VS_SIPField_AAA_MD5* out2_md5 = dynamic_cast<VS_SIPField_AAA_MD5*> ( out2.iSIPMeta->iAuthorization->GetAAA() );
//	if ( !out2_md5 )
//	{
//		std::cout << "[!] !out2_md5\n";
//		return 0;
//	}
//	
//	char* nonce1 = income1_md5->Nonce();
//	unsigned int nonce1_sz = (unsigned int) strlen(nonce1);
//
//	out2_md5->Nonce(nonce1, nonce1_sz);
//
//	VS_SIPBuffer content;
//	//const char* con = "\r\n";
//	//content.AddData(con, (unsigned int) strlen(con));
//
//	unsigned char Resp[32 + 1];
//	out2_md5->MD5("vandam2006", TYPE_REGISTER, &content, Resp);
//	out2_md5->Response((char*) Resp, 32);
//
//	// --- Не надо увеличивать nonce-count
//	//unsigned int nc2 = mmm2->Nc();
//	//nc2++;
//	//mmm2->Nc(nc2);
//
//	VS_SIPField_CSeq* cseq = out2.iSIPMeta->iCSeq;
//	if ( !cseq )
//	{
//		std::cout << "[!] !out2.CSeq\n";
//		return 0;
//	}
//	unsigned int cseq_value = cseq->Value();
//	cseq_value++;
//	cseq->Value(cseq_value);
//
//	VS_SIPField_Via* via = out2.iSIPMeta->iVia;
//	if ( !via )
//	{
//		std::cout << "[!] !out2.Via\n";
//		return 0;
//	}
//	if ( !via->IsBranch() )
//	{
//		std::cout << "[!] !out2.CSeq\n";
//		return 0;
//	}
//	char* branch = new char[9 + 1];
//	memcpy((char*) branch, via->Branch(), 9);
//	branch[8] = 'A';
//	branch[9] = 0;
//	via->Branch(branch, (unsigned int) strlen(branch));
//
//	unsigned int sz = 65535;
//	char* ptr = new char[sz];		
//
//	err = out2.Encode(ptr, sz);
//	if ( err != e_ok )
//	{
//		std::cout << "[-] out2.Encode():" << err << "\n";
//		return 0;
//	}
//	
//	out_bytes = UDP.SendTo((void*) ptr, sz, ip, 5060);
//	if (out_bytes < 1)
//	{
//		std::cout << "[-] UDP.SendTo()\n";
//		return 0;
//	}else{
//		std::cout << "[+] UDP.SendTo()\n";
//	}
//
//	return 0;
//}*/
//
///*int TestNetTCP()
//{
//	VS_ConnectionTCP TCP;
//	
//	if ( !VS_AcsLibInitial() )
//	{
//		std::cout << "[-] VS_AcsLibInitial()\n";
//		return 0;
//	}else{
//		std::cout << "[+] VS_AcsLibInitial()\n";
//	}
//
//	const unsigned long host_sz = 256;
//	char host[host_sz];
//	if (!VS_GetDefaultHostName(host , 256))
//	{
//		std::cout << "[-] VS_GetDefaultHostName()\n";
//		return 0;
//	}else{
//		std::cout << "[+] VS_GetDefaultHostName()\n";
//	}
//
//	if ( !TCP.Bind(host, 5060) )
//	{
//		std::cout << "[-] Bind()\n";
//		return 0;
//	}else{
//		std::cout << "[+] Bind()\n";
//	}
//
//	const unsigned long ip = 0xD43523DB;	// "sipnet.ru"
//	//const unsigned long ip = 0xC0A83D3C;	// "192.168.61.60"
//
//	unsigned long mills = 20000;
//
//	if ( !TCP.Connect(ip, 5060, mills, false) )
//	{
//		std::cout << "[-] Connect()\n";
//		return 0;
//	}else{
//		std::cout << "[+] Connect()\n";
//	}
//
//	int out_bytes = 0;
//	out_bytes = TCP.Send((void*) msg1_TCP, msg1_TCP_sz, mills);
//	if (out_bytes  < 1)
//	{
//		TCP.Disconnect();
//		std::cout << "[-] TCP.Send()\n";
//		return 0;
//	}else{
//		std::cout << "[+] TCP.Send()\n";
//	}
//
//	std::cout << "Recive...";
//	char buff[65536];
//	int in_bytes = TCP.Receive(buff, 65536, mills);
//	buff[in_bytes] = 0;
//	std::cout << "[OK]\n";
//	std::cout << "---[ buff:" << in_bytes << " ]---------------\n" << buff << "-------------------------\n";
//
//	VS_SIPMessage income1;
//	int err = income1.Decode(buff, in_bytes);
//	if ( err != e_ok )
//	{
//		TCP.Disconnect();
//		std::cout << "[-] income1.Decode()\n";
//		return 0;
//	}else{
//		std::cout << "[+] income1.Decode()\n";
//	}
//
//	if ( !income1.iSIPMeta )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !income1.iSIPMeta\n";
//		return 0;
//	}
//
//	if ( !income1.iSIPMeta->iWWWAuthenticate )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !income1.iSIPMeta->iWWWAuthenticate\n";
//		return 0;
//	}
//
//	VS_SIPField_AAA_MD5* income1_md5 = dynamic_cast<VS_SIPField_AAA_MD5*> ( income1.iSIPMeta->iWWWAuthenticate->GetAAA() );
//	if ( !income1_md5 )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !income1_md5\n";
//		return 0;
//	}
//
//	VS_SIPMessage out2;
//	err = out2.Decode(msg2_TCP, msg2_TCP_sz);
//	if ( err != e_ok )
//	{
//		TCP.Disconnect();
//		std::cout << "[-] out2.Decode()\n";
//		return 0;
//	}else{
//		std::cout << "[+] out2.Decode()\n";
//	}
//
//	if ( !out2.iSIPMeta )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !out2.iSIPMeta\n";
//		return 0;
//	}
//
//	if ( !out2.iSIPMeta->iAuthorization )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !out2.iSIPMeta->iAuthorization\n";
//		return 0;
//	}
//
//	VS_SIPField_AAA_MD5* out2_md5 = dynamic_cast<VS_SIPField_AAA_MD5*> ( out2.iSIPMeta->iAuthorization->GetAAA() );
//	if ( !out2_md5 )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !out2_md5\n";
//		return 0;
//	}
//	
//	char* nonce1 = income1_md5->Nonce();
//	unsigned int nonce1_sz = (unsigned int) strlen(nonce1);
//
//	out2_md5->Nonce(nonce1, nonce1_sz);
//
//	VS_SIPBuffer content;
//
//	unsigned char Resp[32 + 1];
//	out2_md5->MD5("vandam2006", TYPE_REGISTER, &content, Resp);
//	out2_md5->Response((char*) Resp, 32);
//
//	VS_SIPField_CSeq* cseq = out2.iSIPMeta->iCSeq;
//	if ( !cseq )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !out2.CSeq\n";
//		return 0;
//	}
//	unsigned int cseq_value = cseq->Value();
//	cseq_value++;
//	cseq->Value(cseq_value);
//
//	VS_SIPField_Via* via = out2.iSIPMeta->iVia;
//	if ( !via )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !out2.Via\n";
//		return 0;
//	}
//	if ( !via->IsBranch() )
//	{
//		TCP.Disconnect();
//		std::cout << "[!] !out2.CSeq\n";
//		return 0;
//	}
//	char* branch = new char[9 + 1];
//	memcpy((char*) branch, via->Branch(), 9);
//	branch[8] = 'A';
//	branch[9] = 0;
//	via->Branch(branch, (unsigned int) strlen(branch));
//
//	unsigned int sz = 65535;
//	char* ptr = new char[sz];		
//
//	err = out2.Encode(ptr, sz);
//	if ( err != e_ok )
//	{
//		TCP.Disconnect();
//		std::cout << "[-] out2.Encode():" << err << "\n";
//		return 0;
//	}
//	
//	out_bytes = 0;
//	out_bytes = TCP.Send((void*) ptr, sz, mills);
//	if (out_bytes  < 1)
//	{
//		TCP.Disconnect();
//		std::cout << "[-] TCP.Send()\n";
//		return 0;
//	}else{
//		std::cout << "[+] TCP.Send()\n";
//	}
//
//	std::cout << "Recive...";
//	in_bytes = TCP.Receive(buff, 65536, mills);
//	buff[in_bytes] = 0;
//	std::cout << "[OK]\n";
//	std::cout << "---[ buff:" << in_bytes << " ]---------------\n" << buff << "-------------------------\n";
//
//	TCP.Disconnect();
//
//	return 0;
//}*/
//
//void Register__21_03_2007();
//void Parser_FastUpdatePicture__22_03_2007();
//
//void TestCalcDigest();
//void TestRegMgr();
//void Test_SIPServer();
//void Test_RegisterOnSIPNET();
//void TestParser();
//void TestSIPServer();
//int main(void)
//{
//	//Test_RegisterOnSIPNET();
//	//Test_SIPServer();
//	//TestRegMgr();
//	//TestCalcDigest();
//	//Register__21_03_2007();
//	//Parser_FastUpdatePicture__22_03_2007();
//	TestSIPServer();
//
//	return 0;
//
////	VS_SIPMessage msg;
////
////	if ( e_ok != msg.Decode(SIP, SIP_sz) )
////		return 0;
////
////	VS_SIPParserInfo info;
//////	msg.FillInfoByInviteMessage( &info );
////
////	VS_SIPRequest req;
////
////	info.SetMyCsAddress(0xC0A83D3C, 5060, CONNECTIONTYPE_TCP);
////
////	info.SetStreamAddress(SDP_MEDIATYPE_AUDIO, 0xC0A83D3C, 4000);
////	info.SetStreamAddress(SDP_MEDIATYPE_VIDEO, 0xC0A83D3C, 4002);
////
////	info.SetAlias_my("my@alias.ru");
////	//info.SetTag_my("my_tag123");
////	info.SetAlias_sip("sip@alias.ru");
////	//info.SetTag_sip("sip_tag321");
////	//info.SetSIPCallID("call_id123", 10);
////
////	if ( !req.MakeINVITE( &info ) )
////		return 0;
////
////	//if ( !req.MakeBYE( &info ) )
////	//	return 0;
////
////	char ptr[4096] = {0};
////	unsigned int sz = 4096;
////
////	req.Encode(ptr, sz);
////
////	VS_SIPResponse rsp;
////
////	if ( !rsp.MakeOnInviteResponseOK( msg, &info ) )
////		return 0;
////
////	sz = 4096;	memset(ptr, 0, sz);
////
////	rsp.Encode(ptr, sz);
////
////return 0;
//
//	//VS_SIPURI uri;
//	//VS_SIPBuffer buf;
//	//char * c(" \"ktrushnikov@visicron.ru\"<sip:192.168.61.164>;tag=1234567890\r\n\r\n\r\n");
//	//unsigned long csz = strlen(c);
//	//buf.AddData(c,csz);
//	//int b = uri.Decode(buf);
//	//printf("\n\t AAAA: %d",b);
//
///*	//VS_SIPURI uri;
//_CrtMemState s1, s2, s3;
//
//_CrtMemCheckpoint( &s1 );
//
//	TestNetTCP();
//
//	//TestAAA(AAA_AUTH, AAA_AUTH_sz);
//
//// TODO: UnComment
//	//TestSlot();
//
//	//return false;
//	//for (unsigned int i=0; i < 100; i++)
//	//{
//	//	std::cout << i << "). \n";		
//	//	TestSIP(SIP, (unsigned int) strlen(SIP));
//	//}
//
//_CrtMemCheckpoint( &s2 );
//
//
//_CrtDumpMemoryLeaks();
//
//if ( _CrtMemDifference( &s3, &s1, &s2) ) 
//	_CrtMemDumpStatistics( &s3 );
//*/
//}
//
//void Parser_FastUpdatePicture__22_03_2007()
//{
//
//	char* ptr =
//		"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n"
//		"<media_control>\r\n"
//		"<vc_primitive>\r\n"
//		"<to_encoder>\r\n"
////		"<picture_fast_update/>\r\n"
//		"<picture_fast_update>\r\n"
//		"</picture_fast_update>\r\n"
//		"</to_encoder>\r\n"
//		"</vc_primitive>\r\n"
//		"</media_control>\r\n";
//	unsigned int ptr_sz = (unsigned int) strlen(ptr);
//
//	VS_SIPMessage msg;
//
//	msg.Decode(ptr, ptr_sz);
//	return ;
//
//	boost::regex e(
//		"(?i)"
//			".*"
//			"< *\\? *xml *[^>]+>"
//				" *(?:\\r\\n)+"
//			"< *media_control *>"
//				" *(?:\\r\\n)+"
//			"< *vc_primitive *>"
//				" *(?:\\r\\n)+"
//			"< *to_encoder *>"
//				" *(?:\\r\\n)+"
//			"< *picture_fast_update */? *>"
//				" *(?:\\r\\n)+"
//			"(?:< */ *picture_fast_update *>"
//				" *(?:\\r\\n)+"
//			")?"
//			"< */ *to_encoder *>"
//				" *(?:\\r\\n)+"
//			"< */ *vc_primitive *>"
//				" *(?:\\r\\n)+"
//			"< */ *media_control *>"
//				" *(?:\\r\\n)+"
//			".*"
//		"(?-i) *"
//	);
//	boost::cmatch m;
//
//	if ( !boost::regex_match(ptr, m, e) )
//	{
//		printf("FUCK!!!!!!!!!!!!!!!\n");
//	}
//
//	std::string m1 = m[1];
//	std::string m2 = m[2];
//	std::string m3 = m[3];
//}
//
//void Register__21_03_2007()
//{
//// MD5(login:realm:pwd)
//	//const char* sbs_info = "572c73d631c81b05ad81755d3ff10467";
//	//unsigned int sbs_info_sz = (unsigned int) strlen(sbs_info);
//
//// MD5(bob:biloxi.com:zanzibar)
//	//const char* sbs_info = "12af60467a33e8518da5c68bbff12b11";
//	//unsigned int sbs_info_sz = (unsigned int) strlen(sbs_info);
//
//// MD5(bob:192.168.61.60:zanzibar)
//	//const char* sbs_info = "535ee00d4495668d3a82d58e3ef8a712";
//	//unsigned int sbs_info_sz = (unsigned int) strlen(sbs_info);	
//
//// MD5(1:192.168.61.60:1)
//	const char* sbs_info = "57f7becfe40dc032a618fa39b5276364";
//	unsigned int sbs_info_sz = (unsigned int) strlen(sbs_info);	
//
///////////////////////////////////////////////////////////////////////////////////
//	printf("\tTest VS_SIPRequest::MakeREGISTER()\n");
//
//	VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
//	if ( !factory )
//		return ;	
//
//	VS_SIPParserInfo parser_info;
//
//	parser_info.SetMyCsAddress(0xC0A83DA7, 12345, CONNECTIONTYPE_UDP);		// 192.168.61.60
//	parser_info.SetSIPCallID("CallID0000000");
//	parser_info.SetAlias_my("bob");
//	parser_info.SetAlias_sip("192.168.61.60");
//	parser_info.SetTag_my("fromtag");
//
//	VS_SIPField_Via via;
//	via.Init(&parser_info);
//
//	parser_info.SetSIPVia(&via);
//
//	VS_SIPRequest req;
//	req.MakeREGISTER(&parser_info);
//
//					char buf[1024] = {0};
//					unsigned int buf_len = 1024;
//
//					req.Encode(buf, buf_len);
//
//					VS_ConnectionUDP udp;
//
//					if ( !VS_AcsLibInitial() )
//						return ;
//
//					const unsigned long host_sz = 256;
//					char host[host_sz];
//					if (!VS_GetDefaultHostName(host , 256))
//						return ;
//
//					if ( !udp.Bind(host, 12345) )
//						return ;
//
//					udp.SendTo(buf, buf_len, 0xC0A83D3C, 5060);
//
//					char in_buff[65535] = {0};
//
//					// Trying
//					int in_bytes = udp.Receive(in_buff, 65535);
//					in_buff[in_bytes] = 0;
//
//					memset(in_buff, 0, 65535);
//
//					// UnAuthorized
//					in_bytes = udp.Receive(in_buff, 65535);
//					in_buff[in_bytes] = 0;
//
//	VS_SIPMessage msg;
//	if (e_ok != msg.Decode(in_buff, in_bytes) )
//	{
//		printf("Decode1\n");
//		return ;
//	}
//
//	VS_SIPMetaField* sip_meta = msg.GetSIPMetaField();
//	if ( !sip_meta )
//	{
//		printf("sip_meta\n");
//		return ;
//	}
//
//	if ( !sip_meta->iAuthHeader )
//	{
//		printf("!sip_meta->iAuthScheme\n");
//		return ;
//	}
//
//	VS_SIPAuthInfo* auth_info = sip_meta->iAuthHeader->GetAuthInfo();
//	VS_SIPAuthDigest* digest_info = (VS_SIPAuthDigest*) (sip_meta->iAuthHeader->GetAuthInfo());
//
//	digest_info->login("1");
//	digest_info->auth_digest( sbs_info );
////	sbs_auth_info->nonce( "a8c88adca1eff60c39bfa9a81455f4014ea21b4f" );
//	digest_info->uri( "sip:192.168.61.60" );
////	sbs_auth_info->realm( "192.168.61.60" );
//	digest_info->method( TYPE_REGISTER );
//	digest_info->algorithm( SIP_AAA_ALGORITHM_MD5 );
//	digest_info->nc(1);
//
////	digest_info->cnonce("cnoncevalue");
//
//	if ( !factory->CalcDigestResponse(auth_info) )
//	{
//		printf("!factory->CalcDigestResponse(digest_info)\n");
//		return ;
//	}
//
//	parser_info.SetAuthScheme(digest_info);
//
//	VS_SIPRequest req_with_auth_info;
//	if ( e_ok != req_with_auth_info.MakeREGISTER(&parser_info, VS_SIPObjectFactory::SIPHeader_Authorization) )
//	{
//		printf("e_ok != req_with_auth_info.MakeREGISTER\n");
//		return ;
//	}
//
//					buf_len = 1024;
//					memset(buf, 0, buf_len);	
//
//					if ( e_ok != req_with_auth_info.Encode(buf, buf_len) )
//					{
//						printf("e_ok != req_with_auth_info.Encode\n");
//						return ;
//					}
//
//					int out_bytes  = udp.SendTo(buf, buf_len, 0xC0A83D3C, 5060);
//
//					printf("Out Bytes: %d\n", out_bytes);
//
//					// UnAuthorized
//					in_bytes = udp.Receive(in_buff, 65535);
//					in_buff[in_bytes] = 0;
//
//	printf("The End\n");
//}
//
//void TestCalcDigest()
//{
//// MD5(1:192.168.61.60:1)
//	const char* sbs_info = "b3b8b5a8f126034a1f61cd08134bca67";
//	unsigned int sbs_info_sz = (unsigned int) strlen(sbs_info);	
//	
//	VS_SIPAuthDigest* _sbs_auth_info = new VS_SIPAuthDigest;	
//	VS_SIPAuthInfo* sbs_auth_info = dynamic_cast<VS_SIPAuthInfo*> (_sbs_auth_info);
//	if (!sbs_auth_info)
//		return;
//
//	sbs_auth_info->auth_digest( sbs_info );
//	sbs_auth_info->nonce( "6852D448CE84F0CA46B1" );
//	sbs_auth_info->cnonce( "16e1" );	
//	sbs_auth_info->uri( "sip:sipnet.ru" );
//	sbs_auth_info->realm( "etc.tario.ru" );
//	sbs_auth_info->method( TYPE_REGISTER );
//	sbs_auth_info->algorithm( SIP_AAA_ALGORITHM_MD5 );
//	sbs_auth_info->qop(SIP_AAA_QOP_AUTH);
//	sbs_auth_info->nc(1);
//
//	VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
//	if ( !factory )
//		return ;
//
//	if ( !factory->CalcDigestResponse(sbs_auth_info) )
//		return ;
//
//	return ;
//}
//
//void TestParser()
//{
//	const char* ptr1 = "SIP/2.0 200 OK\r\n"
//		"Via: SIP/2.0/TCP 10.34.16.104:2130;branch=z9hG4bK5FBB5C1132384C551387988F9CF9073E\r\n"
////		"Expires: asd\r\n"
//		"From: \"user2@msk.sviaz-bank.ru\"<sip:user2@msk.sviaz-bank.ru>;tag=2FBDB7E4F2CCA87365FD36C4EEE964A5\r\n"
//		"To: <sip:10.34.16.140>;tag=plcm_57668000-19055159\r\n"
//		"Call-ID: 30E637A1834D847D0BF0B1C0B05A4937\r\n"
////		"CSeq: 1 INVITE\r\n"
//		"CSeq: asd INVITE\r\n"
//		"Contact: <sip:(null)@10.34.16.140:5060;transport=tcp>\r\n"
//		"Allow: INVITE,BYE,CANCEL,ACK,INFO,PRACK,COMET,OPTIONS,SUBSCRIBE,NOTIFY,REFER,REGISTER,UPDATE\r\n"
//		"Supported: ms-forking,100rel\r\n"
//		"User-Agent: Polycom VSX 6000A (Release 8.5 - 08May2006 13:09)\r\n"
//		"Content-Type: application/sdp\r\n"
//		"Content-Length: 1092\r\n"
//		"\r\n"
//		"v=0\r\n"
//		"o=user2@msk.sviaz-bank.ru 2082170087 0 IN IP4 10.34.16.140\r\n"
//		"s=-\r\n"
//		"c=IN IP4 10.34.16.140\r\n"
//		"b=AS:256\r\n"
//		"t=0 0\r\n"
//		"m=audio 49154 RTP/AVP 15 18 8 0 115 114 113 99 98 97 102 101 103 9\r\n"
//		"a=rtpmap:15 G728/8000\r\n"
//		"a=rtpmap:18 G729/8000\r\n"
//		"a=fmtp:18 annexb=no\r\n"
//		"a=rtpmap:8 PCMA/8000\r\n"
//		"a=rtpmap:0 PCMU/8000\r\n"
//		"a=rtpmap:115 G7221/32000\r\n"
//		"a=fmtp:115 bitrate=48000\r\n"
//		"a=rtpmap:114 G7221/32000\r\n"
//		"a=fmtp:114 bitrate=32000\r\n"
//		"a=rtpmap:113 G7221/32000\r\n"
//		"a=fmtp:113 bitrate=24000\r\n"
//		"a=rtpmap:99 SIREN14/16000\r\n"
//		"a=fmtp:99 bitrate=48000\r\n"
//		"a=rtpmap:98 SIREN14/16000\r\n"
//		"a=fmtp:98 bitrate=32000\r\n"
//		"a=rtpmap:97 SIREN14/16000\r\n"
//		"a=fmtp:97 bitrate=24000\r\n"
//		"a=rtpmap:102 G7221/16000\r\n"
//		"a=fmtp:102 bitrate=32000\r\n"
//		"a=rtpmap:101 G7221/16000\r\n"
//		"a=fmtp:101 bitrate=24000\r\n"
//		"a=rtpmap:103 G7221/16000\r\n"
//		"a=fmtp:103 bitrate=16000\r\n"
//		"a=rtpmap:9 G722/8000\r\n"
//		"a=sendrecv\r\n"
//		"m=video 49156 RTP/AVP 109 34 96 31\r\n"
//		"b=TIAS:262000\r\n"
//		"a=rtpmap:109 H264/90000\r\n"
//		;
//	const unsigned int ptr1_sz = (unsigned int) strlen(ptr1);
//
//	const char* ptr2 = "a=fmtp:109 profile-level-id=42800c; max-mbps=10000; max-br=625\r\n"
//		"a=rtpmap:34 H263/90000\r\n"
//		"a=fmtp:34 CIF4=2;CIF=1;QCIF=1;F\r\n"
//		"a=rtpmap:96 H263-1998/90000\r\n"
//		"a=fmtp:96 CIF4=2;CIF=1;QCIF=1;F;J;T\r\n"
//		"a=rtpmap:31 H261/90000\r\n"
//		"a=fmtp:31 CIF=1;QCIF=1\r\n"
//		"a=sendrecv\r\n"
//		;
//	const unsigned int ptr2_sz = (unsigned int) strlen(ptr2);
//
//
//	VS_TerminalAbstractFactory::FactoryInit(2);
//
//				//VS_SIPParserInfo parser_info;
//
//				//parser_info.SetStreamAddress(SDP_MEDIATYPE_AUDIO, 0xC0A83DA7, 10000);
//				//parser_info.SetStreamAddress(SDP_MEDIATYPE_VIDEO, 0xC0A83DA7, 20000);
//
//				//parser_info.SetMyCsAddress(0xC0A83DA7, 12345, CONNECTIONTYPE_UDP);		// 192.168.61.60
//				//parser_info.SetSIPCallID("CallID0000000");
//				//parser_info.SetAlias_my("bob");
//				//parser_info.SetAlias_sip("192.168.61.60");
//				//parser_info.SetTag_my("fromtag");
//
//				//VS_SIPField_Via via;
//				//via.Init(&parser_info);
//
//				//parser_info.SetSIPVia(&via);
//
//	VS_SIPCall call;
//
//	VS_CallSignallingMgr mgr;
//	mgr.Init(&call);
//
//	mgr.SetRecvBuf((unsigned char*) ptr1, ptr1_sz, e_SIP_CS);
//
//	if ( e_ok != mgr.SetRecvBuf((unsigned char*) ptr2, ptr2_sz, e_SIP_CS) )
//		return ;
//}
//
//void TestSIPServer()
//{
//	const char msg1[] = "REGISTER sip:sipnet.ru SIP/2.0\r\n"
//		"Via: SIP/2.0/TCP 192.168.61.60:12062\r\n"
//		"Max-Forwards: 70\r\n"
//		"From: <sip:forcastnewer@sipnet.ru>;tag=914c7585a89e49c3b868ee04b768ea93;epid=bde33910b4\r\n"
//		"To: <sip:forcastnewer@sipnet.ru>\r\n"
//		"Call-ID: 3867bcdb27414b47bd15a0304d620831\r\n"
//		"CSeq: 1 REGISTER\r\n"
//		"Contact: <sip:192.168.61.60:12062;transport=tcp>;methods=\"INVITE, MESSAGE, INFO, SUBSCRIBE, OPTIONS, BYE, CANCEL, NOTIFY, ACK, REFER, BENOTIFY\";proxy=replace\r\n"
//		"User-Agent: RTC/1.3.5369 (Sippoint v1.5.1)\r\n"
//		"Supported: com.microsoft.msrtc.presence, adhoclist\r\n"
//		"ms-keep-alive: UAC;hop-hop=yes\r\n"
//		"Event: registration\r\n"
//		"Allow-Events: presence\r\n"
//		"Content-Length: 0\r\n"
//		"\r\n";
//	unsigned int msg1_sz = (unsigned int) strlen(msg1);
//
//	VS_SIPServer server;
//	server.Init(0xC0A83B3C, 5065);
//
//	server.SetRecvBuf((unsigned char*) msg1, msg1_sz, 0xC0A83B33 , 5060, CONNECTIONTYPE_TCP);
//
//	char buf[65535] = {0};
//	unsigned long buf_sz = 65535;
//	unsigned long out_ip = 0;
//	unsigned short out_port = 0;
//
//	server.GetBufForSend((unsigned char*) buf, buf_sz, out_ip, out_port, CONNECTIONTYPE_TCP);
//
//	return ;
//}
//
////void TestRegMgr()
////{
////// MD5(1:192.168.61.60:1)
////	const char* sbs_info = "57f7becfe40dc032a618fa39b5276364";
////	unsigned int sbs_info_sz = (unsigned int) strlen(sbs_info);	
////
////	VS_TerminalAbstractFactory::FactoryInit(2);
////
////				VS_SIPParserInfo parser_info;
////
////				parser_info.SetStreamAddress(SDP_MEDIATYPE_AUDIO, 0xC0A83DA7, 10000);
////				parser_info.SetStreamAddress(SDP_MEDIATYPE_VIDEO, 0xC0A83DA7, 20000);
////
////				parser_info.SetMyCsAddress(0xC0A83DA7, 12345, CONNECTIONTYPE_UDP);		// 192.168.61.60
////				parser_info.SetSIPCallID("CallID0000000");
////				parser_info.SetAlias_my("bob");
////				parser_info.SetAlias_sip("192.168.61.60");
////				parser_info.SetTag_my("fromtag");
////
////				VS_SIPField_Via via;
////				via.Init(&parser_info);
////
////				parser_info.SetSIPVia(&via);
////
////	VS_SIPAuthDigest* digest_info = new VS_SIPAuthDigest;
////
////	digest_info->login("1");
////	digest_info->auth_digest( sbs_info );
//////	sbs_auth_info->nonce( "a8c88adca1eff60c39bfa9a81455f4014ea21b4f" );
////	digest_info->uri( "sip:192.168.61.60" );
//////	sbs_auth_info->realm( "192.168.61.60" );
////	digest_info->method( TYPE_REGISTER );
////	digest_info->algorithm( SIP_AAA_ALGORITHM_MD5 );
////	digest_info->nc(1);
////
////	parser_info.SetAuthScheme(digest_info);
////
////	VS_RASRegistrationImpl reg_info;
////	reg_info.SetSIPParserInfo(&parser_info);
////
////	VS_RegistrationMgr mgr;
////	mgr.Init(&reg_info);
////
////	VS_ConnectionUDP udp;
////
////	if ( !VS_AcsLibInitial() )
////		return ;
////
////	const unsigned long host_sz = 256;
////	char host[host_sz];
////	if (!VS_GetDefaultHostName(host , 256))
////		return ;
////
////	if ( !udp.Bind(host, 12345) )
////		return ;
////
////	unsigned char in_buf[65535] = {0};
////	const unsigned long in_buf_sz = 65535;
////
////	unsigned char out_buf[65535] = {0};
////	unsigned long out_buf_sz = 65535;
////
////	int hop = 0;
////	while(1)
////	{
////		if ( mgr.GetBufForSend(out_buf, out_buf_sz, e_SIP_Register) )
////			udp.SendTo(out_buf, out_buf_sz, 0xC0A83D3C, 5060);
////
////		out_buf_sz = 65535;
////		memset(out_buf, 0, out_buf_sz);		
////
////		int in_bytes = udp.Receive(in_buf, in_buf_sz);
////		in_buf[in_bytes] = 0;
////
////		mgr.SetRecvBuf(in_buf, in_bytes, e_SIP_Register);
////		hop++;
////		if (hop == 3)
////		{
////			parser_info.SetAlias_my("bob");
////			parser_info.SetAlias_sip("1@192.168.61.202");
////
////			VS_SIPRequest r;
////			if ( r.MakeINVITE( &parser_info ) )
////			{
////				char* out = 0;
////				unsigned int out_sz = 0;
////
////				if ( (e_buffer == r.Encode(out, out_sz)) && (out_sz > 0) )
////				{
////					out = new char[out_sz + 1];
////					if ( !out || (e_ok != r.Encode(out, out_sz)) )
////					{
////						if (out) { delete out; out = 0; }
////						return ;
////					}
////				}else{
////					if (out) { delete out; out = 0; }
////					return ;
////				}
////
////				int out_bytes = udp.SendTo(out, out_sz, 0xC0A83D3C, 5060);	
////				printf("MakeINVITE(): ok\n");
////			}
////		}
////
////		if (hop == 4)
////		{
////			VS_SIPRequest r;
////			if ( r.MakeCANCEL( &parser_info ) )
////			{
////				char* out = 0;
////				unsigned int out_sz = 0;
////
////				if ( (e_buffer == r.Encode(out, out_sz)) && (out_sz > 0) )
////				{
////					out = new char[out_sz + 1];
////					if ( !out || (e_ok != r.Encode(out, out_sz)) )
////					{
////						if (out) { delete out; out = 0; }
////						return ;
////					}
////				}else{
////					if (out) { delete out; out = 0; }
////					return ;
////				}
////
////				int out_bytes = udp.SendTo(out, out_sz, 0xC0A83D3C, 5060);	
////				printf("MakeCANCEL(): ok\n");
////			}
////		}
////	}	
////}
//
////void Test_SIPServer()
////{
////	VS_SIPServer srv;
////	srv.Init(0xC0A83DCA, 5060);
////
////	//srv.SetRecvBuf((unsigned char*) msg1, msg1_sz, 0xC0A83DA7, 5060);	
////	//srv.SetRecvBuf((unsigned char*) MY_SIP, MY_SIP_sz, 0xC0A83DA7, 5060);
////
////	//unsigned char buf[65535] = {0};
////	//unsigned long buf_sz = 65535;
////	//unsigned long ip = 0;
////	//unsigned short port = 0;
////
////	//srv.GetBufForSend(buf, buf_sz, ip, port);
////
////	//memset(buf, 0, 65535);
////	//buf_sz = 65535;
////	//ip = 0;
////	//port = 0;
////
////	//srv.GetBufForSend(buf, buf_sz, ip, port);
////
////
////	VS_ConnectionUDP udp;
////
////	if ( !VS_AcsLibInitial() )
////		return ;
////
////	const unsigned long host_sz = 256;
////	char host[host_sz];
////	if (!VS_GetDefaultHostName(host , 256))
////		return ;
////
////	if ( !udp.Bind(host, 5060) )
////		return ;
////
////	unsigned char in_buf[65535] = {0};
////	const unsigned long in_buf_sz = 65535;
////
////	unsigned char out_buf[65535] = {0};
////	unsigned long out_buf_sz = 65535;
////
////	int hop = 0;
////	unsigned long ip_to = 0;
////	unsigned short port_to = 0;
////	unsigned long ip_from = 0;
////	unsigned short port_from = 0;
////	unsigned long mills = 5*60*1000;
////
////	for(;;)
////	{
////		while ( srv.GetBufForSend(out_buf, out_buf_sz, ip_to, port_to) )
////			udp.SendTo(out_buf, out_buf_sz, ip_to, port_to);
////
////		out_buf_sz = 65535;
////		memset(out_buf, 0, out_buf_sz);		
////
////		int in_bytes = udp.ReceiveFrom(in_buf, in_buf_sz, &ip_from, &port_from, mills);
////		in_buf[in_bytes] = 0;
////
////		srv.SetRecvBuf(in_buf, in_bytes, ip_from, port_from);
////	}
////}
////
void Test_RegisterOnSIPNET()
{
// MD5(1:192.168.61.60:1)
//	const char* sbs_info = "b3b8b5a8f126034a1f61cd08134bca67";			// goofydog:etc.tario.ru:goof456
	const char* sbs_info = "dc2e8054ddf54597ee43d75df37130aa";			// bbs:videoport.ru:ktktkt777
	unsigned int sbs_info_sz = (unsigned int) strlen(sbs_info);	

	VS_TerminalAbstractFactory::FactoryInit(2);

				VS_SIPParserInfo parser_info;

				parser_info.SetStreamAddress(SDP_MEDIATYPE_AUDIO, 0xC0A83D3C, 4000);
				parser_info.SetStreamAddress(SDP_MEDIATYPE_VIDEO, 0xC0A83D3C, 4002);

				parser_info.SetMyCsAddress(0xC0A8280E, 5060, CONNECTIONTYPE_UDP);		// 192.168.61.60:5060 UDP
				parser_info.SetAlias_my("bbs@asterisk.visicron.ru");
				parser_info.SetAlias_sip("bbs@asterisk.visicron.ru");

				VS_SIPField_Via via;
				via.Init(&parser_info);

				parser_info.SetSIPVia(&via);

	VS_SIPAuthDigest* digest_info = new VS_SIPAuthDigest;

	digest_info->login("bbs");
	digest_info->auth_digest( sbs_info );
	digest_info->cnonce( "hjg32jgjhbxch" );
	digest_info->uri( "sip:videoport.ru" );
//	digest_info->realm( "etc.tario.ru" );
	digest_info->method( TYPE_REGISTER );
	digest_info->algorithm( SIP_AAA_ALGORITHM_MD5 );
	digest_info->nc(1);

	parser_info.SetAuthScheme(digest_info);

	VS_RASRegistrationImpl reg_info;
	reg_info.SetSIPParserInfo(&parser_info);

	VS_RegistrationMgr mgr;
	mgr.Init(&reg_info);

	VS_ConnectionUDP udp;

	if ( !VS_AcsLibInitial() )
		return ;

	const unsigned long host_sz = 256;
	char host[host_sz];
	if (!VS_GetDefaultHostName(host , 256))
		return ;

	if ( !udp.Bind("192.168.2.2", 5060) )
		return ;

	unsigned char in_buf[65535] = {0};
	const unsigned long in_buf_sz = 65535;

	unsigned char out_buf[65535] = {0};
	unsigned long out_buf_sz = 65535;

	int hop = 0;
	while(1)
	{
		if ( mgr.GetBufForSend(out_buf, out_buf_sz, e_SIP_Register) )
			udp.SendTo(out_buf, out_buf_sz, 0xD586D0A2, 5060);			 // was 0xD43523DB,5060
					// 213.134.208.162	asterisk.visicron.ru

		out_buf_sz = 65535;
		memset(out_buf, 0, out_buf_sz);		

		int in_bytes = udp.Receive(in_buf, in_buf_sz);
		in_buf[in_bytes] = 0;

		mgr.SetRecvBuf(in_buf, in_bytes, e_SIP_Register);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
///// Test of SIPSlot
///////////////////////////////////////////////////////////////////////////////////////////
///*int TestConnections()
//{
//	VS_NetReactor reactor;
//	VS_NetConnection conn;
//	{
//		int code;
//		if (e_ok!=(code=reactor.Init()))
//		{
//			printf("\n\t Error code : %d",code);
//			getch();
//			return code;
//		}
//		else
//		{
//			int code;
//
//			if (e_ok!=(code=conn.Init(false,&reactor)))
//			{
//				printf("\n\t Init Error code : %d",code);
//				getch();
//				return code;
//			}
//			if (e_ok!=(code=conn.Bind(5060)))
//			{
//				printf("\n\t Init Error code : %d",code);
//				getch();
//				return code;
//			}
//
//			unsigned long ip =  0xC0A83D39;//0x7F000001; //127.0.0.1
//								//0xD5B4CC08//Ya.Ru
//								//0xC0A83D39 //Polycom IP 192.168.61.57
//			unsigned short port= 0x13C4; //50//80
//								//13C4//5060//
//			if (e_ok!=(code=conn.Connect(ip,port)))
//			{
//				printf("\n\t Connect Error code : %d",code);
//				getch();
//				return code;
//			}
//			
//			char *a = MY_SIP;
//			unsigned int l = (unsigned )strlen(MY_SIP);
//			if (e_ok!=(code=conn.AddWriteMessage(a,l)))
//			{
//				printf("\n\t Write Error code : %d",code);
//				getch();
//				return code;
//			}
//			if (e_ok!=(code=conn.Read()))
//			{
//				printf("\n\t Read Error code : %d",code);
//				getch();
//				return code;
//			}
//			if (e_ok!=(code=conn.Write()))
//			{
//				printf("\n\t Write Error code : %d",code);
//
//				return code;
//			}
//			printf("\n\t .....");
//			Sleep(5000);
//			char * Hbuffer = 0;
//			unsigned int Hlength = 0;
//			unsigned int messages = conn.GetReadMessage(Hbuffer,Hlength);
//			if (messages>=1)
//			{
//				Hbuffer[Hlength] = 0;
//				printf("\n\t %s",Hbuffer);
//				printf("\n\t Length = %u",Hlength);
//			}
//
//		}
//	}
//	printf("\n\t press any...");
//	getch();
//	return e_ok;
//
//}
//int TestSlot()
//{
//	return TestConnections();
//}*/

int main(void)
{
	Test_RegisterOnSIPNET();
	//TestRegMgr();

	return 0;
}
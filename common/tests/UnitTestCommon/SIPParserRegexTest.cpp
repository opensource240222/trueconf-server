#if defined(_WIN32) // Not ported yet

#include "../../SIPParserLib/VS_SIPObjectFactory.h"
#include "../../SIPParserLib/VS_SIPURI.h"
#include "../../SIPParserLib/VS_SIPField_UserAgent.h"
#include "../../SIPParserLib/VS_SIPField_Via.h"
#include "../../SIPParserLib/VS_SIPField_To.h"
#include "../../SIPParserLib/VS_SIPField_Accept.h"
#include "../../SIPParserLib/VS_SIPField_Allow.h"
#include "../../SIPParserLib/VS_SIPField_Auth.h"
#include "../../SIPParserLib/VS_SIPField_CallID.h"
#include "../../SIPParserLib/VS_SIPField_Contact.h"
#include "../../SIPParserLib/VS_SIPField_ContentLength.h"
#include "../../SIPParserLib/VS_SIPField_ContentType.h"
#include "../../SIPParserLib/VS_SIPField_CSeq.h"
#include "../../SIPParserLib/VS_SIPField_Event.h"
#include "../../SIPParserLib/VS_SIPField_Expires.h"
#include "../../SIPParserLib/VS_SIPField_From.h"
#include "../../SIPParserLib/VS_SIPField_MaxForwards.h"
#include "../../SIPParserLib/VS_SIPField_MinExpires.h"
#include "../../SIPParserLib/VS_SIPField_MinSE.h"
#include "../../SIPParserLib/VS_SIPField_RecordRoute.h"
#include "../../SIPParserLib/VS_SIPField_Route.h"
#include "../../SIPParserLib/VS_SIPField_SessionExpires.h"
#include "../../SIPParserLib/VS_SIPField_StartLine.h"
#include "../../SIPParserLib/VS_SIPField_Supported.h"
#include "../../SIPParserLib/VS_SIPField_Unsupported.h"
#include "../../SIPParserLib/VS_SIPField_RetryAfter.h"
#include "../../SIPParserLib/VS_SDPObjectFactory.h"
#include "../../SIPParserLib/VS_SDPField_Attribute.h"
#include "../../SIPParserLib/VS_SDPField_Bandwidth.h"
#include "../../SIPParserLib/VS_SDPField_Connection.h"
#include "../../SIPParserLib/VS_SDPField_MediaStream.h"
#include "../../SIPParserLib/VS_SDPField_Origin.h"
#include "../../SIPParserLib/VS_SDPField_SessionName.h"
#include "../../SIPParserLib/VS_SDPField_Time.h"
#include "../../SIPParserLib/VS_SDPField_Version.h"
#include "../../SIPParserLib/VS_SDPConnect.h"
#include "../../SIPParserLib/VS_SDPCodecH264.h"
#include "../../SIPParserLib/VS_SDPCodecH263.h"
#include "../../SIPParserLib/VS_SDPCodec.h"
#include "../../SIPParserLib/VS_SDPCodecG729A.h"
#include "../../SIPParserLib/VS_SDPCodecH261.h"
#include <tuple>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace sip_regex_test
{
	/**
	params
		functor that return boost::regex,
		test_string,
		vector of matchers
	*/
	using TestParamType = std::tuple<std::function<const boost::regex&()>, bool, std::string, std::vector<std::string>>;
	/**
	params
		functor that return boost::regex,
		test_string,
		vector of matchers
	*/
	using IterateTestParamType = std::tuple<std::function<const boost::regex&()>, std::string, std::vector<std::string>>;

	/**
	params
	functor that return boost::regex,
	test_string,
	string that must be found
	*/
	using SearchTestParamType = std::tuple<std::function<const boost::regex&()>, std::string, std::string>;;

#define make_param(f, ...) \
	TestParamType([]() -> const boost::regex& { return (f); }, true, __VA_ARGS__)

#define make_param_nomatch(f, s) \
	TestParamType([]() -> const boost::regex& { return (f); }, false, s, {})

#define make_iterate_param(f, ...) \
	IterateTestParamType([]() -> const boost::regex& { return (f); }, __VA_ARGS__)

#define make_search_param(f, ...) \
	SearchTestParamType([]() -> const boost::regex& { return (f); }, __VA_ARGS__)

	/*TestParamType test_case_array[] = {
		std::make_tuple([]() -> const boost::regex&
		{return VS_SDPField_MediaStream::e1; },
										"m=audio 49184 RTP/AVP 115 102 9 15 0 8 18",
			std::vector<std::string>({ "m=audio 49184 RTP/AVP 115 102 9 15 0 8 18", "audio", "49184", "", "RTP/AVP", " 115 102 9 15 0 8 18"}))
	};*/

	TestParamType test_case_array[] = {
		make_param(VS_SIPObjectFactory::e, "VIA", { "VIA" }),
		make_param(VS_SIPObjectFactory::e, "CALL-ID", { "CALL-ID" }),

		make_param(VS_SIPURI::e1, ";maddr=11.22.33.44; transport=tcp;lr;tag=plcm_3741300540-812;epid=8213020FD0F0CV", { "11.22.33.44" }),
		make_param(VS_SIPURI::e2, ";maddr=11.22.33.44; transport=tcp;lr;tag=plcm_3741300540-812;epid=8213020FD0F0CV", { "tcp" }),
		make_param(VS_SIPURI::e3, ";maddr=11.22.33.44; transport=tcp;lr;tag=plcm_3741300540-812;epid=8213020FD0F0CV", {}),
		make_param(VS_SIPURI::e4, ";maddr=11.22.33.44; transport=tcp;lr;tag=plcm_3741300540-812;epid=8213020FD0F0CV", { "8213020FD0F0CV" }),
		make_param(VS_SIPURI::e5, ";maddr=11.22.33.44; transport=tcp;lr;tag=plcm_3741300540-812;epid=8213020FD0F0CV", { "plcm_3741300540-812" }),
		make_param(VS_SIPURI::e5, ";tag=5 bb86910-babc80a-13c4-40030-8a5145-40552802-8a5145", { "5 bb86910-babc80a-13c4-40030-8a5145-40552802-8a5145" }),
		make_param(VS_SIPURI::e7, ";gr=urn:uuid:998e8de6-7d02-5d23-abb3-7e070e1deb8c", { "urn:uuid:998e8de6-7d02-5d23-abb3-7e070e1deb8c" }),
		make_param(VS_SIPURI::e6, "sip:213.133.168.206",
		{ "", "", "", "", "", "", "", "", "sip", "213.133.168.206", "", "", "", "" }),
		make_param(VS_SIPURI::e6, "sip:test@213.133.168.206",
		{ "", "", "", "", "", "", "", "", "sip", "", "test", "213.133.168.206", "", "" }),
		make_param(VS_SIPURI::e6, " sip:213.133.168.206 SIP/2.0",
		{ "", "", "", "", "", "", "", "", "sip", "213.133.168.206", "", "", "", "SIP/2.0" }),
		make_param(VS_SIPURI::e6, " <sip:test@213.133.168.206>;tag=2E211B8E5554F826ACDFDC4ACE68DE09",
		{ "", "sip", "", "test", "213.133.168.206", "", "", ";tag=2E211B8E5554F826ACDFDC4ACE68DE09", "", "", "", "", "", "" }),
		make_param(VS_SIPURI::e6, " <sip:test@0.0.0.1:5060;transport=udp>;expires=60",
		{ "", "sip", "", "test", "0.0.0.1", "5060", ";transport=udp", ";expires=60", "", "", "", "", "", "" }),
		make_param(VS_SIPURI::e6, "<sip:79499;phone-context=cdp.moldgres@interrao.ru;user=phone>;tag=5 bb86910-babc80a-13c4-40030-8a5145-40552802-8a5145",
		{ "", "sip", "", "79499", "interrao.ru", "", ";user=phone", ";tag=5 bb86910-babc80a-13c4-40030-8a5145-40552802-8a5145", "", "", "", "", "", "" }),
		make_param(VS_SIPURI::e6, "\"ATS OVK1\" <sip:79499;phone-context=cdp.moldgres@interrao.ru;user=phone>;tag=5 bb86910-babc80a-13c4-40030-8a5145-40552802-8a5145",
		{ "ATS OVK1", "sip", "", "79499", "interrao.ru", "", ";user=phone", ";tag=5 bb86910-babc80a-13c4-40030-8a5145-40552802-8a5145", "", "", "", "", "", "" }),
		make_param(VS_SIPURI::e6, " <sip:192.168.66.57:21669;transport=tls;ms-opaque=a3b656b803;ms-received-cid=127B00>;expires=7200;+sip.instance=\"<urn:uuid:11ec8985-494a-5186-9cd1-3fb177f23bbc>\";gruu=\"sip:user1@lync2010.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu\"",
		{ "", "sip", "192.168.66.57", "", "", "21669", ";transport=tls;ms-opaque=a3b656b803;ms-received-cid=127B00", ";expires=7200;+sip.instance=\"<urn:uuid:11ec8985-494a-5186-9cd1-3fb177f23bbc>\";gruu=\"sip:user1@lync2010.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu\"", "", "", "", "", "", "" }),
		make_param(VS_SIPURI::opaque_e, "opaque=state:F:Ci.R15f2ec00",{ "state:F:Ci.R15f2ec00" }),
		make_param(VS_SIPURI::ms_opaque_e, "ms-opaque=state:F:Ci.R15f2ec00",{ "state:F:Ci.R15f2ec00" }),
		make_param(VS_SIPURI::ms_route_sig_e, "MS-route-sig=bwKixcm4asz5ZICKKoFkkPY4vkp_tFNwPtWg3E4_Gx7U0hNuj3XdhgiQAA",{ "bwKixcm4asz5ZICKKoFkkPY4vkp_tFNwPtWg3E4_Gx7U0hNuj3XdhgiQAA" }),
		make_param(VS_SIPURI::ms_key_info_e, "ms-key-info=bwKixcm4asz5ZICKKoFkkPY4vkp",{ "bwKixcm4asz5ZICKKoFkkPY4vkp" }),
		make_param(VS_SIPURI::ms_identity_e, "ms-identity=val",{ "val" }),
		make_param(VS_SIPURI::ms_fe_e, "ms-fe=T2RU-SKYPE-01.corp.tele2.ru",{ "T2RU-SKYPE-01.corp.tele2.ru" }),
		make_param(VS_SIPURI::ms_role_rs_to_e, "ms-role-rs-to",{}),
		make_param(VS_SIPURI::ms_role_rs_from_e, "ms-role-rs-from",{}),
		make_param(VS_SIPURI::ms_ent_dest_e, "ms-ent-dest",{}),


		make_param(VS_SIPField_UserAgent::e1, "User-Agent: Blink 1.4.2 (Windows)", { "Blink 1.4.2 (Windows)" }),

		make_param(VS_SIPField_Via::e1, ";rport;branch=z9hG4bKPj09a89d2d549f47aaa1359a3ecc272abf",
		{ "z9hG4bKPj09a89d2d549f47aaa1359a3ecc272abf" }),
		make_param(VS_SIPField_Via::e2, "Via: SIP/2.0/UDP 192.168.66.68:56416;rport;branch=z9hG4bKPjc694fb1fc2ea45cd9387653eabc9d935",
		{ "UDP", "192.168.66.68", "56416", ";rport;branch=z9hG4bKPjc694fb1fc2ea45cd9387653eabc9d935" }),

		// ru-ticket#6439: slash at via-params
		make_param(VS_SIPField_Via::e2, "Via: SIP/2.0/UDP 10.62.255.1:5060;x-ds0num=\"ISDN 0 / 0 / 1:15 0 / 0 / 1 : DS1 9 : DS0\";branch=z9hG4bK1281B9C",
		{ "UDP", "10.62.255.1", "5060", ";x-ds0num=\"ISDN 0 / 0 / 1:15 0 / 0 / 1 : DS1 9 : DS0\";branch=z9hG4bK1281B9C" }),
		make_param(VS_SIPField_Via::e2, "Via: SIP/2.0/UDP 10.62.255.1:5060;x-ds0num=\"ISDN 0 / 0 / 1:15 0 / 0 / 1 : DS1 9 : DS0\";branch=z9hG4bK1281B9C\r\n",
		{ "UDP", "10.62.255.1", "5060", ";x-ds0num=\"ISDN 0 / 0 / 1:15 0 / 0 / 1 : DS1 9 : DS0\";branch=z9hG4bK1281B9C" }),

		make_param(VS_SIPField_Via::e3, "Via: SIP/2.0/UDP 192.168.66.68:56416;keep=360;branch=z9hG4bKPjc694fb1fc2ea45cd9387653eabc9d935",
		{ "360" }),

		make_param(VS_SIPField_To::e, "To: \"serg\" <sip:qwe@192.168.66.68>",
		{ "To", "" }),
		make_param(VS_SIPField_To::e, "To:  <sip:hdx8000@192.168.62.42> ;tag=plcm_4273200408-287505551",
		{ "To", "" }),

		make_param(VS_SIPField_Accept::e, "Accept: application/simple-message-summary",
		{ "application/simple-message-summary" }),
		make_param(VS_SIPField_Accept::e, "Accept: application/sdp",
		{ "application/sdp" }),

		make_param(VS_SIPField_Allow::e, "Allow: INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY",
		{ "INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY" }),

		make_param(VS_SIPField_Auth::e, "WWW-Authenticate: DIGEST realm=\"trueconf\", nonce=\"cd2c552637981902d81775e5cff45ce3\"",
		{ "WWW-Authenticate: DIGEST ", "WWW-Authenticate", "DIGEST" }),
		make_param(VS_SIPField_Auth::e, "Authorization: Digest username=\"qwe\", realm=\"trueconf\", nonce=\"f7d34317e1d46495d5020c0c8fbdb5d3\", uri=\"sip:192.168.66.68\", response=\"fe83a2e507bcc818e8e85a9af814f639\"",
		{ "Authorization: Digest ", "Authorization", "Digest" }),
		make_param(VS_SIPField_Auth::e, "WWW-Authenticate: TLS-DSK realm=\"SIP Communications Service\", targetname=\"LYNC2013-SERVER.lync.loc\", version=4, sts-uri=\"https://LYNC2013-SERVER.lync.loc:443/CertProv/CertProvisioningService.svc\"\r\n",
		{ "WWW-Authenticate: TLS-DSK ", "WWW-Authenticate", "TLS-DSK" }),

		make_param(VS_SIPField_CallID::e, "Call-ID: 6537df3c49b1c298@192.168.1.8", { "6537df3c49b1c298@192.168.1.8" }),
		make_param(VS_SIPField_CallID::e, "Call-ID: 3285718264-287341511", { "3285718264-287341511" }),

		make_param(VS_SIPField_Contact::e, "Contact : hdx8000 <sip:hdx8000@85.202.225.51:5060;transport=tcp> ;proxy=replace", { "Contact", " " }),
		make_param(VS_SIPField_Contact::e, "m : hdx8000 <sip:hdx8000@85.202.225.51:5060;transport=tcp> ;proxy=replace", { "m", " " }),
		make_param(VS_SIPField_Contact::e, "Contact : <sip:192.168.62.43:5060;transport=tcp>;+sip.instance=\"<urn:uuid : 02ec0634 - 751b - 5259 - b1aa - 156a7ca2b6c6>\"", { "Contact", " " }),
		make_param(VS_SIPField_Contact::e1, "Contact: hdx8000 <sip:hdx8000@85.202.225.51:5060;transport=tcp> ;expires=3600", { "3600" }),
		make_param(VS_SIPField_Contact::e2, "Contact: <sip:192.168.66.57:21669;transport=tls;ms-opaque=a3b656b803;ms-received-cid=127B00>;expires=7200;+sip.instance=\"<urn:uuid:11ec8985-494a-5186-9cd1-3fb177f23bbc>\";gruu=\"sip:user1@lync2010.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu\"", { "sip:user1@lync2010.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu" }),

		make_param(VS_SIPField_ContentLength::e, "Content-Length: 100", { "100" }),
		make_param(VS_SIPField_ContentLength::e, "l:100", { "100" }),

		make_param(VS_SIPField_ContentType::e, "Content-Type: application/sdp; charset=UTF-8", { "application/sdp", "; charset=UTF-8" }),
		make_param(VS_SIPField_ContentType::e, "c:application/sdp;charset=UTF-8", { "application/sdp", ";charset=UTF-8" }),
		make_param(VS_SIPField_ContentType::e, "Content-Type: application/sdp", { "application/sdp", "" }),
		make_param(VS_SIPField_ContentType::e, "c:application/sdp", { "application/sdp", "" }),
		make_param(VS_SIPField_ContentType::e, "Content-Type: multipart/alternative;boundary=\"----=_NextPart_000_001C_01D282D0.D2055920\"", { "multipart/alternative", ";boundary=\"----=_NextPart_000_001C_01D282D0.D2055920\"" }),

		make_param(VS_SIPField_CSeq::e, "CSeq: 1 INVITE", { "1", "INVITE" }),

		make_param(VS_SIPField_Event::e, "Event:registration", { "registration" }),
		make_param(VS_SIPField_Event::e, "Event: registration", { "registration" }),

		make_param(VS_SIPField_Expires::e, "Expires: 300", { "300" }),
		make_param(VS_SIPField_Expires::e, "Expires:300", { "300" }),

		make_param(VS_SIPField_From::e, "From : 185.22.184.14 <sip:185.22.184.14@185.22.184.14>;tag=plcm_3741300540-812;epid=8213020FD0F0CV",
		{ "From", " " }),
		make_param(VS_SIPField_From::e, "f:185.22.184.14 <sip:185.22.184.14@185.22.184.14>;tag=plcm_3741300540-812;epid=8213020FD0F0CV",
		{ "f", "" }),

		make_param(VS_SIPField_MaxForwards::e, "Max-Forwards: 70", { "70" }),

		make_param(VS_SIPField_MinExpires::e, "Min-Expires: 3200", { "3200" }),

		make_param(VS_SIPField_MinSE::e, "Min-SE: 90", { "90" }),

		make_param(VS_SIPField_RecordRoute::e, "Record-Route : <sip:proxy-call-id=df5ea190-8f41-11e5-9a74-0010f31fd402@203.188.221.150:5060;transport=tcp;lr>",
		{ " " }),

		make_param(VS_SIPField_Route::e, "Route : <sip:192.168.61.91;lr>",
		{ " " }),

		make_param(VS_SIPField_SessionExpires::e, "Session-Expires: 1800;refresher=uas",
		{ "1800", ";refresher=uas" }),
		make_param(VS_SIPField_SessionExpires::e1, ";refresher=uas",
		{ "uas" }),

		make_param(VS_SIPField_StartLine::e, "INVITE sip:10.174.0.70:60610;transport=tcp SIP/2.0",
		{ "INVITE", "", "" }),
		make_param(VS_SIPField_StartLine::e, "SIP/2.0 100 Trying",
		{ "", "100", "Trying" }),

		make_param(VS_SIPField_Supported::e1, "Supported: replaces,100rel,timer,gruu,path,outbound",
		{ "replaces,100rel,timer,gruu,path,outbound" }),
		make_param(VS_SIPField_Supported::e1, "k:replaces,100rel,timer,gruu,path,outbound",
		{ "replaces,100rel,timer,gruu,path,outbound" }),
		make_param(VS_SIPField_Supported::e2, "qwer,qw,timer,qwe,fgh,sdf", {}),
		make_param(VS_SIPField_Supported::e3, "sdf,100rel,fgh,sdf,ewr,rty", {}),
		make_param(VS_SIPField_Supported::e4, "replaces,cxv,dsf,fgh,sdf,ewr", {}),

		make_param(VS_SIPField_Unsupported::e1, "Unsupported: rel100", { "rel100" }),
		make_param(VS_SIPField_Unsupported::e2, "Unsupported: timer,100rel", {}),
		make_param_nomatch(VS_SIPField_Unsupported::e2, "Unsupported: replaces,100rel"),
		make_param(VS_SIPField_Unsupported::e3, "Unsupported: 100rel,replaces", {}),
		make_param_nomatch(VS_SIPField_Unsupported::e3, "Unsupported: replaces,rel100"),
		make_param(VS_SIPField_Unsupported::e4, "Unsupported: replaces,100rel", {}),
		make_param_nomatch(VS_SIPField_Unsupported::e4, "Unsupported: timer,100rel"),

		make_param(VS_SDPObjectFactory::e, "m", { "m" }),

		make_param(VS_SDPField_Attribute::e1, "a=sendonly", { "sendonly" }),

		make_param(VS_SDPField_Bandwidth::e, "b=TIAS:384000", { "TIAS", "384000" }),
		make_param(VS_SDPField_Bandwidth::e, "b=AS:384", { "AS", "384" }),

		make_param(VS_SDPField_Connection::e, " c  =IN IP4 192.168.62.113", // it's counting spaces around 'c'
		{ " ", "  " }),
		make_param(VS_SDPField_Connection::e, "  c =IN IP4 192.168.62.113",
		{ "  ", " " }),

		make_param(VS_SDPField_MediaStream::e1, "m=audio 49184 RTP/AVP 115 102 9 15 0 8 18",
		{ "audio", "49184", "", "RTP/AVP", " 115 102 9 15 0 8 18" }),
		make_param(VS_SDPField_MediaStream::e1, "m=video 5064 RTP/AVP 109 34 96 31",
		{ "video", "5064", "", "RTP/AVP", " 109 34 96 31"}),
		make_param(VS_SDPField_MediaStream::e1, "m=video 5064/2 RTP/AVP 109 34 96 31",
		{ "video", "5064", "2", "RTP/AVP", " 109 34 96 31" }),
		make_param(VS_SDPField_MediaStream::e2, "a=sendonly",
		{ "a", "sendonly", "", "" }),
		make_param(VS_SDPField_MediaStream::e2, "a=rtpmap:98 L16/11025/2",
		{ "a", "rtpmap", "98 L16/11025/2", "" }),
		make_param(VS_SDPField_MediaStream::e3, "1 AES_CM_128_HMAC_SHA1_80 inline:9+s+/+675CaW+4oOF+C4ViBZc2nIjsFEVVoJbL0J|2^31",
		{ "AES_CM_128_HMAC_SHA1_80", "9+s+/+675CaW+4oOF+C4ViBZc2nIjsFEVVoJbL0J" }),

		make_param(VS_SDPField_Origin::e, " o  =   hdx8000 2024860525 0 IN IP4 192.168.62.113",
		{" ", "  ", "   ", "hdx8000", " ", "2024860525", " ", "0", " ", "IN IP4 192.168.62.113"}),
		make_param(VS_SDPField_Origin::e, "o=TrueConf 459656167 0 IN IP4 192.168.62.41",
		{ "", "", "", "TrueConf", " ", "459656167", " ", "0", " ", "IN IP4 192.168.62.41" }),
		make_param(VS_SDPField_Origin::e, "o=huawei 1 0 IN IP4 192.168.62.46",
		{ "", "", "", "huawei", " ", "1", " ", "0", " ", "IN IP4 192.168.62.46" }),

		make_param(VS_SDPField_SessionName::e, "s=-", { "-" }),
		make_param(VS_SDPField_SessionName::e, "s=noname", { "noname" }),
		make_param(VS_SDPField_SessionName::e, "s= ", { "" }),

		make_param(VS_SDPField_Time::e, "t=0 0", { "0", "0" }),

		make_param(VS_SDPField_Version::e, "v=0", { "0" }),

		make_param(VS_SDPConnect::e, "IN IP4 192.168.62.113/127/3", { "192.168.62.113", "", "127", "3" }),
		make_param(VS_SDPConnect::e, "IN IP6 FF15::101/3", { "", "FF15::101", "3", "" } ),

		make_param(VS_SDPCodecH264::e, "a=fmtp:109 profile-level-id=42800d; max-mbps=40000; max-fs=1792; max-br=1600",
		{ "42", "80", "0d" }),
		make_param(VS_SDPCodecH264::e1, "a=fmtp:109 profile-level-id=42800d; max-mbps=40000; max-fs=1792; max-br=1600",
		{ "40000" }),
		make_param(VS_SDPCodecH264::e2, "a=fmtp:109 profile-level-id=42800d; max-mbps=40000; max-fs=1792; max-br=1600",
		{ "1792" }),
		make_param(VS_SDPCodecH264::e3, "a=fmtp:109 profile-level-id=42800d; max-mbps=40000; max-fs=1792; max-br=1600",
		{ "1600" }),

		make_param(VS_SDPCodecH263::e1, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F", { "1" }),
		make_param(VS_SDPCodecH263::e2, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F", { "1" }),
		make_param(VS_SDPCodecH263::e3, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F", { "1" }),
		make_param_nomatch(VS_SDPCodecH263::e4, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F"),
		make_param(VS_SDPCodecH263::e5, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F", { "1" }),
		make_param(VS_SDPCodecH263::e6, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F", { "F", "" }),
		make_param_nomatch(VS_SDPCodecH263::e6, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1"),
		make_param_nomatch(VS_SDPCodecH263::e7, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1"),
		make_param_nomatch(VS_SDPCodecH263::e8, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1"),
		make_param_nomatch(VS_SDPCodecH263::e9, "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1"),

		make_param(VS_SDPCodecH263::e1, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T", { "1" }),
		make_param(VS_SDPCodecH263::e2, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T", { "1" }),
		make_param(VS_SDPCodecH263::e3, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T", { "1" }),
		make_param_nomatch(VS_SDPCodecH263::e4, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T"),
		make_param(VS_SDPCodecH263::e5, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T", { "1" }),
		make_param(VS_SDPCodecH263::e6, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T", { "F", "" }),
		make_param_nomatch(VS_SDPCodecH263::e7, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T"),
		make_param(VS_SDPCodecH263::e8, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T", { "J", "" }),
		make_param(VS_SDPCodecH263::e9, "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T", { "T", "" }),

		make_param(VS_SDPCodec::e, " G722/8000",
		{ "G722", "8000", "" }),
		make_param(VS_SDPCodec::e, " H264/90000",
		{ "H264", "90000", "" }),
		make_param(VS_SDPCodec::e, " H263-1998/90000",
		{ "H263-1998", "90000", "" }),
		make_param(VS_SDPCodec::e, " OPUS/48000/2",
		{ "OPUS", "48000", "2" }),
		make_param(VS_SDPCodec::e2, " bitrate=24000", { "24000" }),
		make_param(VS_SDPCodec::e2, " profile-level-id=24;object=23;bitrate=64000", { "64000" }),


		make_param(VS_SDPCodecG729A::e, "a=fmtp:18 annexb=no", { "no" }),

		make_param(VS_SDPCodecH261::e1, "a=fmtp:31 CIF=1;QCIF=1", { "1" }),
		make_param_nomatch(VS_SDPCodecH261::e1, "a=fmtp:31 QCIF=1"),
		make_param(VS_SDPCodecH261::e2, "a=fmtp:31 CIF=1;QCIF=1", { "1" }),
		make_param_nomatch(VS_SDPCodecH261::e2, "a=fmtp:31 CIF=1"),
		make_param_nomatch(VS_SDPCodecH261::e3, "a=fmtp:31 CIF=1;QCIF=1"),

		make_param(VS_SIPField_RetryAfter::e, "Retry-After: 5", { "5" ,"","","",""}),
		make_param(VS_SIPField_RetryAfter::e, "Retry-After: 18000;duration=3600", { "18000","","",";duration=3600","3600" }),
		make_param(VS_SIPField_RetryAfter::e, "Retry-After: 120 (I'm in a meeting)", { "120", "(I'm in a meeting)", "I'm in a meeting","",""}),
		make_param(VS_SIPField_RetryAfter::e, "Retry-After: 120 (I'm in a meeting);duration=3600", { "120", "(I'm in a meeting)", "I'm in a meeting", ";duration=3600", "3600" }),
	};

	IterateTestParamType iterate_test_case_array[] = {
		make_iterate_param(VS_SIPField_Contact::base_uri_e, "\"Mr.Watson\" <sip:watson@worcester.bell-telephone.com>;q = 0.7;expires = 3600,\"Mr. Watson\" <mailto:watson@bell-telephone.com>;q = 0.1",
							{ "\"Mr.Watson\" <sip:watson@worcester.bell-telephone.com>;q = 0.7;expires = 3600",
							  "\"Mr. Watson\" <mailto:watson@bell-telephone.com>;q = 0.1"}),
	};

	SearchTestParamType searchTestCaseArray[] = {
		make_search_param(
			VS_SIPField_Contact::name_addr_param_e
			, "<sip:test2@192.168.41.111:5070>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\""
			, "<sip:test2@192.168.41.111:5070>"),
		make_search_param(
			VS_SIPField_Contact::name_addr_param_e
			, "sip:test2@192.168.41.111:5070;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\""
			, "sip:test2@192.168.41.111:5070")
	};

#undef make_param
#undef make_iterate_param
#undef make_search_param

	class SIPRegexTest : public ::testing::TestWithParam<TestParamType>
	{};

	TEST_P(SIPRegexTest, MatchTest)
	{
		boost::cmatch m;
		const boost::regex &e = std::get<0>(GetParam())();
		bool res = std::get<1>(GetParam());
		auto &vector_result = std::get<3>(GetParam());
		const std::string &str = std::get<2>(GetParam());
		if (res) {
			ASSERT_TRUE(boost::regex_match(str.c_str(), m, e)) << "(string is '" << str << "')";
			ASSERT_EQ(m.size(), vector_result.size() + 1) << "(string is '" << str << "')";
			for (size_t i = 0; i < vector_result.size(); i++)
			{
				ASSERT_EQ(vector_result[i], static_cast<std::string>((m[i + 1]))) << "(i = " << i << ")" << std::endl << "(string is '" << str << "')";;
			}
		} else {
			ASSERT_FALSE(boost::regex_match(str.c_str(), m, e)) << "(string is '" << str << "')";
		}
	}

	INSTANTIATE_TEST_CASE_P(MatchProbes,
		SIPRegexTest,
		::testing::ValuesIn(test_case_array));

	class SIPIterateRegexTest : public ::testing::TestWithParam<IterateTestParamType>
	{};

	TEST_P(SIPIterateRegexTest, MatchTest)
	{

		const boost::regex &e = std::get<0>(GetParam())();
		auto &vector_result = std::get<2>(GetParam());
		const std::string &str = std::get<1>(GetParam());

		boost::sregex_token_iterator iter(str.begin(), str.end(), e, 0);
		boost::sregex_token_iterator end;

		for (unsigned i = 0; iter != end; ++iter,++i) {
			std::string s(*iter);
			ASSERT_TRUE(i < vector_result.size());
			ASSERT_TRUE(s == vector_result[i]);
		}
	}

	INSTANTIATE_TEST_CASE_P(IterateProbes,
		SIPIterateRegexTest,
		::testing::ValuesIn(iterate_test_case_array));

	class SIPSearchRegexTest : public ::testing::TestWithParam<SearchTestParamType>
	{};

	TEST_P(SIPSearchRegexTest, SearchTest)
	{

		const boost::regex &e = std::get<0>(GetParam())();
		auto &toSearch = std::get<2>(GetParam());
		const std::string &str = std::get<1>(GetParam());
		boost::smatch m;

		EXPECT_TRUE(boost::regex_search(str, m, e));
		ASSERT_TRUE(!m.empty());
		std::string foundStr(m[0].first, m[0].second);
		EXPECT_STREQ(foundStr.c_str(), toSearch.c_str());
	}

	INSTANTIATE_TEST_CASE_P(SearchProbes,
		SIPSearchRegexTest,
		::testing::ValuesIn(searchTestCaseArray));
}

#endif

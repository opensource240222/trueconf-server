#ifndef SIP_RAW_H
#define SIP_RAW_H

namespace sipraw{
	const char _200ok_contact_with_gruu[] =
R"(SIP/2.0 200 OK
Authentication-Info: NTLM qop="auth", opaque="0999F355", srand="AE4D44F8", snum="1", rspauth="010000006f006900a5960bc32d0f5e40", targetname="sarco04.skype2015.loc", realm="SIP Communications Service", version=4
From: "user6"<sip:user6@skype2015.loc>;tag=7DFF5C53A16C40AF26B60AF0062567AF;epid=451e9252b6
To: <sip:user6@skype2015.loc>;tag=C5FE2F33CD23A451AA1D653DB6C6859C
Call-ID: 501ED1D3EE6E7872588F67ADC5194AFA
CSeq: 3 REGISTER
Via: SIP/2.0/TLS 192.168.41.128:17925;branch=z9hG4bK8680AE1AB33D105937088D3C32BF86D8-2;ms-received-port=17925;ms-received-cid=87400
Contact: <sip:user6@192.168.41.128:17925;transport=tls>;expires=300;+sip.instance="<urn:uuid:e0de506b-a705-59cc-9bfa-d37593141231>";gruu="sip:user6@skype2015.loc;opaque=user:epid:a1De4AWnzFmb-tN1kxQSMQAA;gruu"
Expires: 300
Allow-Events: vnd-microsoft-provisioning,vnd-microsoft-roaming-contacts,vnd-microsoft-roaming-ACL,presence,presence.wpending,vnd-microsoft-roaming-self,vnd-microsoft-provisioning-v2
Supported: adhoclist
Server: RTC/6.0
Supported: msrtc-event-categories
Supported: ms-keepalive-deregister
Supported: ms-userservices-state-notification
Content-Length: 0

)";
	const char ack_to_invite_with_two_records_route[] =
R"(ACK sip:service.trueconf@10.77.252.13:62755;transport=tls SIP/2.0
Via: SIP/2.0/TLS 10.78.223.115:5061;branch=z9hG4bKEB97EFF1.6C3FC27AAF2E1C33;branched=FALSE
Authentication-Info: NTLM qop="auth", opaque="B73E6169", srand="B202EB08", snum="390", rspauth="01000000000000003302f004b1ee6ab8", targetname="T2RU-SKYPE-04.corp.tele2.ru", realm="SIP Communications Service", version=4
Max-Forwards: 68
Via: SIP/2.0/TLS 10.78.223.112:61619;branch=z9hG4bK35E37E7D.B7C01EEFB1362C33;branched=FALSE;ms-received-port=61619;ms-received-cid=A2EA700
From: "Р”РµРІР°РµРІ РљРёСЂРёР»Р» РђР»РµРєСЃР°РЅРґСЂРѕРІРёС‡"<sip:kirill.devaev@tele2.ru>;tag=753cf74dcb;epid=48a4a25bb1
Via: SIP/2.0/TLS 10.78.251.56:64100;ms-received-port=64100;ms-received-cid=A58B700
To: <sip:service.trueconf@tele2.ru>;tag=B1A549804CB8AA17CCC689E77DDE74BE;epid=0647bd7908
Call-ID: 3eb03c53a8f247bb891eb243e860ff7b
CSeq: 1 ACK
User-Agent: UCCAPI/15.0.4971.1000 OC/15.0.4971.1000 (Skype for Business)
Content-Length: 0

)";
	const char invite_with_two_records_route[] =
R"(INVITE sip:service.trueconf@10.77.252.13:62755;transport=tls SIP/2.0
Record-Route: <sip:skypepool.tele2.ru:5061;transport=tls;ms-fe=T2RU-SKYPE-04.corp.tele2.ru;opaque=state:F:Ci.Rabf6d00;lr;ms-route-sig=fiowNaa3xSVBbFSb_oBnHO9mMUVPRQLxAZwOsB-TedzYiBhirha3BNxAAA>;ms-rrsig=fi6ZIFfGFBvrQTZKnNWlu8UeerJpccOgu9LTyAH5OnF-yBhirha3BNxAAA;tag=A990A1D3DF287188230E96F588D87AD1
Via: SIP/2.0/TLS 10.78.223.115:5061;branch=z9hG4bK8422CB10.E12A8681AF226C33;branched=FALSE;ms-internal-info="cjL3EjunLQK1F8yeGqqjHCibIBLqW7CD8AdeAF_OaXHMyBhirhV5Q87wAA"
Authentication-Info: NTLM qop="auth", opaque="B73E6169", srand="50FDFFB9", snum="389", rspauth="0100000000000000c96295c7b1ee6ab8", targetname="T2RU-SKYPE-04.corp.tele2.ru", realm="SIP Communications Service", version=4
Max-Forwards: 68
Content-Length: 234
Via: SIP/2.0/TLS 10.78.223.112:61619;branch=z9hG4bK452C7551.70CA5D20B1297C33;branched=FALSE;ms-received-port=61619;ms-received-cid=A2EA700
Via: SIP/2.0/TLS 10.78.251.56:64100;ms-received-port=64100;ms-received-cid=A58B700
Record-Route: <sip:skypepool.tele2.ru:5061;transport=tls;ms-fe=T2RU-SKYPE-01.corp.tele2.ru;opaque=state:T;lr>;tag=3A78FB75E877D0A0B16EB39156A3983A
From: "Р”РµРІР°РµРІ РљРёСЂРёР»Р» РђР»РµРєСЃР°РЅРґСЂРѕРІРёС‡"<sip:kirill.devaev@tele2.ru>;tag=753cf74dcb;epid=48a4a25bb1
To: <sip:service.trueconf@tele2.ru>;epid=0647bd7908
Call-ID: 3eb03c53a8f247bb891eb243e860ff7b
CSeq: 1 INVITE
Contact: <sip:kirill.devaev@tele2.ru;opaque=user:epid:4BYPlX2Wv1mCQSyyYpXsJQAA;gruu>
Supported: ms-dialog-route-set-update
ms-text-format: text/plain; charset=UTF-8; ms-body=L2NhbGwgYW5uYS5seWtvdmFAcnVtMTgudHJ1ZWNvbmYubmFtZSAgIA0K
ms-im-format: text/rtf; charset=UTF-8; ms-body=e1xydGYxXGZiaWRpc1xhbnNpXGFuc2ljcGcxMjUxXGRlZmYwXG5vdWljb21wYXRcZGVmbGFuZzEwNDl7XGZvbnR0Ymx7XGYwXGZuaWwgU2Vnb2UgVUk7fXtcZjFcZm5pbFxmY2hhcnNldDIwNCBTZWdvZSBVSTt9e1xmMlxmbmlsXGZjaGFyc2V0MCBTZWdvZSBVSTt9fQ0Ke1xjb2xvcnRibCA7XHJlZDBcZ3JlZW4wXGJsdWUwO1xyZWQwXGdyZWVuMFxibHVlMjU1O30NCntcKlxnZW5lcmF0b3IgUmljaGVkMjAgMTUuMC41MDEzfXtcKlxtbWF0aFByXG13cmFwSW5kZW50MTQ0MCB9XHZpZXdraW5kNFx1YzEgDQpccGFyZFxjZjFccHJvdGVjdFxvdXRsXGYwXGZzMjAgL2NhbGwge1xjZjB7XGZpZWxke1wqXGZsZGluc3R7SFlQRVJMSU5LIGFubmEubHlrb3ZhQHJ1bTE4LnRydWVjb25mLm5hbWUgfX17XGZsZHJzbHR7YW5uYS5seWtvdmFAcnVtMTgudHJ1ZWNvbmYubmFtZVx1bDBcY2YwfX19fVxmMVxmczIwICBcZjJcbGFuZzEwMzMgIFxwcm90ZWN0MFxvdXRsMFxmMVxsYW5nMTA0OSAgXGYwXHBhcg0Ke1wqXGx5bmNmbGFnczxydGY9MT59fQ0K
Supported: ms-embedded-first-message
Supported: ms-delayed-accept
Supported: ms-renders-isf
Supported: ms-renders-gif
Supported: ms-renders-mime-alternative
Ms-Conversation-ID: AdPQwQ+QHyGgNm5cnnI5Sv0cCiqWRgACaRaw
Subject: РџСЂРѕРїСѓС‰РµРЅРЅР°СЏ Р±РµСЃРµРґР° СЃ service.trueconf@tele2.ru
Supported: timer
Supported: histinfo
Supported: ms-safe-transfer
Supported: ms-sender
Supported: ms-early-media
Roster-Manager: sip:kirill.devaev@tele2.ru
EndPoints: <sip:kirill.devaev@tele2.ru>, <sip:service.trueconf@tele2.ru>
Supported: com.microsoft.rtc-multiparty
ms-keep-alive: UAC;hop-hop=yes
Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY
ms-subnet: 10.78.251.0
Supported: ms-conf-invite
Content-Type: application/sdp
history-info: <sip:service.trueconf@tele2.ru>;index=1

v=0
o=- 0 0 IN IP4 10.78.251.56
s=session
c=IN IP4 10.78.251.56
t=0 0
m=message 5060 sip null
a=accept-types:text/plain multipart/alternative image/gif text/rtf application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite
)";

	const char call_with_conference_CID[] =
R"(MESSAGE sip:user6@192.168.41.195:50954;transport=tls SIP/2.0
Via: SIP/2.0/TLS 192.168.74.4:5061;branch=z9hG4bKC62D82DB.F5BED7F75178DA15;branched=FALSE;ms-internal-info="ax-vn4kDyS26rJIjTn99e5yDZKY-JAEfvnZAaq0VajR1L317712b2EGQAA"
Authentication-Info: NTLM qop="auth", opaque="BA815744", srand="457F9338", snum="6", rspauth="0100000000000000f84d5ec2c5990dbe", targetname="sarco04.skype2015.loc", realm="SIP Communications Service", version=4
Max-Forwards: 69
From: "user3"<sip:user3@skype2015.loc>;tag=9b6864536d;epid=72f0d99ead
Via: SIP/2.0/TLS 192.168.41.195:50092;ms-received-port=50092;ms-received-cid=5C3600
To: <sip:user6@skype2015.loc>;tag=0BE8301FCB8775D09EA0DBE73B1C430B;epid=61d55e0379
Call-ID: e6354278851f44198ac5d2e43be60681
CSeq: 2 MESSAGE
User-Agent: UCCAPI/16.0.4405.1000 OC/16.0.4405.1000 (Skype for Business)
Supported: ms-dialog-route-set-update
Supported: timer
Content-Type: text/plain; charset=UTF-8
Content-Length: 13

/call \c\1234)";
	const char unautorized_ntlm_kerberros_tlsdsk[] =
R"(SIP/2.0 401 Unauthorized
Date: Tue, 24 Oct 2017 13:28:50 GMT
WWW-Authenticate: NTLM realm="SIP Communications Service", targetname="LYNC2013-SERVER.lync.loc", version=4
WWW-Authenticate: Kerberos realm="SIP Communications Service", targetname="sip/LYNC2013-SERVER.lync.loc", version=4
WWW-Authenticate: TLS-DSK realm="SIP Communications Service", targetname="LYNC2013-SERVER.lync.loc", version=4, sts-uri="https://LYNC2013-SERVER.lync.loc:443/CertProv/CertProvisioningService.svc"
From: <sip:user6@192.168.74.4>;tag=D3E42EE35E472F759A5A88AE6A214BB8
To: <sip:user6@192.168.74.4>;tag=DCB82A4DA5F4BCE3FD28D1FA0FC1CA94
Call-ID: %s
CSeq: %d REGISTER
Via: SIP/2.0/TLS 192.168.41.195:65100;branch=%s;ms-received-port=65100;ms-received-cid=5D3F00
Server: RTC/5.0
Content-Length: 0

)";

	const char Invite_with_session_expires[] =
		"INVITE sip:192.168.0.100 SIP/2.0\r\n"
		"Via: SIP/2.0/UDP 192.168.0.106;branch=z9hG4bKEA770792F1669A1210C6B2D685CCEC2E-1\r\n"
		"Max-Forwards: 70\r\n"
		"From: \"alexpavl@ub22n.trueconf.name\"<sip:alexpavl@ub22n.trueconf.name>;tag=C021D691B6AFFEDD9A6D8B7CE8AAAB0D\r\n"
		"To: <sip:192.168.0.100>\r\n"
		"Call-ID: 6851A7C96E406796BA379C82A1651128\r\n"
		"CSeq: 1 INVITE\r\n"
		"Contact: <sip:alexpavl@192.168.0.106:5060;transport=udp>\r\n"
		"Content-Type: application/sdp\r\n"
		"Supported: timer, gruu-10\r\n"
		"Allow: INVITE, UPDATE, ACK, CANCEL, INFO, OPTIONS, BYE, MESSAGE\r\n"
		"Session-Expires: 100;refresher=uas\r\n"
		"Min-SE: 90\r\n"
		"Content-Length: 1750\r\n\r\n"
		"v=0\r\n"
		"o=222 5020 8 IN IP4 192.168.62.26\r\n"
		"s=Mapping\r\n"
		"c=IN IP4 192.168.62.26\r\n"
		"t=0 0\r\n"
		"m=audio 5020 RTP/AVP 9 8\r\n"
		"a=rtpmap:9 G722/8000\r\n"
		"a=rtpmap:8 PCMA/8000\r\n";


	const char unautorized_ntlm_with_gss[] =
R"(SIP/2.0 401 Unauthorized
Date: Tue, 24 Oct 2017 13:28:50 GMT
WWW-Authenticate: NTLM opaque="85185AE4", gssapi-data="TlRMTVNTUAACAAAAAAAAADgAAADzgpji+qDppPyr9XwAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgANuVZB8xM0wEAAAAA", targetname="LYNC2013-SERVER.lync.loc", realm="SIP Communications Service", version=4
From: <sip:user6@192.168.74.4>;tag=D3E42EE35E472F759A5A88AE6A214BB8;epid=8cfb320206
To: <sip:user6@192.168.74.4>;tag=DCB82A4DA5F4BCE3FD28D1FA0FC1CA94
Call-ID: %s
CSeq: %d REGISTER
Via: SIP/2.0/TLS 192.168.41.195:65100;branch=%s;ms-received-port=65100;ms-received-cid=5D3F00
Server: RTC/5.0
Content-Length: 0

)";

	const char redirect_to_home_server[] =
R"(SIP/2.0 301 Redirect request to Home Server
Authentication-Info: NTLM qop="auth", opaque="DD23910A", srand="8DD8F2A8", snum="1", rspauth="0100000065000000890295bc234ad41a", targetname="T2RU-SKYPE-02.corp.tele2.ru", realm="SIP Communications Service", version=4
From: <sip:user6@192.168.74.4>;tag=D3E42EE35E472F759A5A88AE6A214BB8;epid=d0b1e08d57
To: <sip:user6@192.168.74.4>;tag=DCB82A4DA5F4BCE3FD28D1FA0FC1CA94
Call-ID: %s
CSeq: 3 REGISTER
Via: SIP/2.0/TLS 192.168.41.195:65100;branch=%s;ms-received-port=65100;ms-received-cid=5D3F00
Contact: <sip:192.168.74.5:5061;transport=tls>
Expires: 2592000
Content-Length: 0

)";

	const char call_command[] =
R"(MESSAGE sip:user6@192.168.41.195:65101;transport=tls SIP/2.0
Via: SIP/2.0/TLS 192.168.74.4:5061;branch=z9hG4bK9F49D5E7.907A8955D1A8D80E;branched=FALSE;ms-internal-info="dhlAywTfmyYa-Jq2V1-7LMJRHYXhmCYlxOxnJ3NbRm7bZViXqQ2b2EGQAA"
Authentication-Info: NTLM qop="auth", opaque="BB512227", srand="D496DDC4", snum="7", rspauth="0100000000000000254ea4835ddf2da6", targetname="sarco04.skype2015.loc", realm="SIP Communications Service", version=4
Max-Forwards: 69
From: "user3"<sip:user3@skype2015.loc>;tag=ff4a012cf7;epid=72f0d99ead
Via: SIP/2.0/TLS 192.168.41.195:57648;ms-received-port=57648;ms-received-cid=447000
To: <sip:user6@skype2015.loc>;tag=03E3B033409902635D9B14DFC47E4F45;epid=61d55e0379
Call-ID: dbf99552542642af8b0a4a222dc3a5aa
CSeq: 2 MESSAGE
User-Agent: UCCAPI/16.0.4405.1000 OC/16.0.4405.1000 (Skype for Business)
Supported: ms-dialog-route-set-update
Supported: timer
Content-Type: text/plain; charset=UTF-8
Content-Length: 9

/call us2)";
	const char busy_here_packet_prototype[] =
		"SIP/2.0 486 Busy Here\r\n"
		"Via: SIP/2.0/TCP 212.13.98.242;branch=%s\r\n"	// sprintf branch here
		"From: \"vialalekseev@rugjr.trueconf.name\""
		"<sip:vialalekseev@rugjr.trueconf.name>;tag=43017E66489962BE2AA6F07035F82193\r\n"
		"To: <sip:212.13.98.248>;tag=plcm_2866295577-3293\r\n"
		"Call-ID: %s\r\n"	// sprintf call_id here
		"CSeq: 1 INVITE\r\n"
		"Supported: timer\r\n"
		"Contact: <sip : (null)@212.13.98.248:5060; transport = udp>; proxy = replace; +sip.instance = \"<urn:uuid:6d692eaa-529f-5751-b8a6-873ae7f982bb>\"\r\n"
		"Retry-After: 5\r\n"
		"Content-Length: 0\r\n\r\n";
	const char MovedTemporarily_packet[] =
		"SIP/2.0 302 Moved Temporarily\r\n"
		"Via: SIP/2.0/UDP 192.168.41.195;branch=%s\r\n"
		"From: \"us2_local@ub1kb.trueconf.name\" <sip:us2_local@ub1kb.trueconf.name>;tag=FA28813281731BA3B126B8E94BBF4E01\r\n"
		"To: <sip:192.168.62.170>;tag=1669806016\r\n"
		"Call-ID: %s\r\n"
		"CSeq: 1 INVITE\r\n"
		"Contact: <sip:hdx8000@192.168.62.42>\r\n"
		"Supported: replaces, path, timer, eventlist\r\n"
		"User-Agent: Grandstream GVC3200 1.0.3.8\r\n"
		"X-IPVT-Client-Info: uid = 8202445; alias = \"Grandstream GVC3200 1.0.3.8\"\r\n"
		"Diversion: <sip:gvc3200@192.168.62.170:5060>;reason=unconditional\r\n"
		"Warning: 399 192.168.62.170 \"Detected NAT type is Port Restricted Cone NAT\"\r\n"
		"Allow: INVITE, ACK, OPTIONS, CANCEL, BYE, SUBSCRIBE, NOTIFY, INFO, REFER, UPDATE, MESSAGE\r\n"
		"Content-Length: 0\r\n\r\n";
	const char MovedTemporarilySeveralContacts_packet[] =
		"SIP/2.0 302 Moved Temporarily\r\n"
		"Via: SIP/2.0/UDP 192.168.11.1;branch=%s\r\n"
		"From: \"us2_local@ub1kb.trueconf.name\" <sip:us2_local@ub1kb.trueconf.name>;tag=FECA3901511DFC7FFA7375AD4AED57B7\r\n"
		"To: <sip:192.168.11.150>\r\n"
		"Call-ID: %s\r\n"
		"CSeq: 1 INVITE\r\n"
		"Contact: <sip:192.168.11.150:5060;transport=UDP>, <sip:hdx8000@192.168.62.42:5060>\r\n"
		"Content-Length: 0\r\n\r\n";
	const char NotFound_packet[] =
		"SIP/2.0 404 Not Found\r\n"
		"Via: SIP/2.0/UDP 192.168.11.1;branch=%s\r\n"
		"From: \"us2_local@ub1kb.trueconf.name\"<sip:us2_local@ub1kb.trueconf.name>;tag=FECA3901511DFC7FFA7375AD4AED57B7\r\n"
		"To: <sip:192.168.11.150:5060;transport=udp>\r\n"
		"Call-ID: %s\r\n"
		"CSeq: 2 INVITE\r\n"
		"Content-Length: 0\r\n\r\n";
	const char MovedTemporarilySeveralContacts1_packet[] =
		"SIP/2.0 302 Moved Temporarily\r\n"
		"Via: SIP/2.0/UDP 192.168.11.1;branch=z9hG4bK8F26BC054A5A566D6F3541EE939B1C65-1\r\n"
		"From: \"us2_local@ub1kb.trueconf.name\" <sip:us2_local@ub1kb.trueconf.name>;tag=FECA3901511DFC7FFA7375AD4AED57B7\r\n"
		"To: <sip:192.168.11.150>\r\n"
		"Call-ID: 648AF3540D00B794908902750A466C1D\r\n"
		"CSeq: 1 INVITE\r\n"
		"Contact: <sip:192.168.11.150:5060;transport=UDP>, <sip:hdx8000@192.168.62.42:5060>\r\n"
		"Contact: <sip:1.2.3.4:5060;transport=UDP>, <sip:gvc3200@192.168.62.170:5060>\r\n"
		"Content-Length: 0\r\n\r\n";
	const char InviteWithExtendedRecordRoute[] =
R"(INVITE sip:service.trueconf@10.77.252.13:53374;transport=tls SIP/2.0
Record-Route: <sip:skypepool.tele2.ru:5061;transport=tls;ms-identity=identity;ms-key-info=info;ms-ent-dest;ms-role-rs-from;ms-role-rs-to;ms-fe=T2RU-SKYPE-01.corp.tele2.ru;ms-opaque=state:F:Ci.R15f2ec00;lr;ms-route-sig=bwvGnrdqFKQO4UnF2Q9LygIdCx-PqWkBQL4Ixnxhuhv_loczcdXdhgiQAA>;ms-rrsig=bwfiGBAet52xft9h3WyrCvDCbjP81OemPnX2mywcCFCd1oczcdXdhgiQAA;tag=C552F8BD89A377955223AA7CAC4A8E00
Via: SIP/2.0/TLS 10.78.223.112:5061;branch=z9hG4bKB9020F8C.1D37736849858B30;branched=TRUE;ms-internal-info="cqAODqTS1kykxWNN0agNoAghseaLCj8uFzrS3iaS-jFIdoczcd98cvUQAA"
Authentication-Info: NTLM qop="auth", opaque="E857C185", srand="5F43AD63", snum="154", rspauth="01000000d40000008e0bbcaab99251bb", targetname="T2RU-SKYPE-01.corp.tele2.ru", realm="SIP Communications Service", version=4
Max-Forwards: 68
Content-Length: 2292
Via: SIP/2.0/TLS 10.78.223.115:49673;branch=z9hG4bK64222A46.7C28527A0DA0DB30;branched=FALSE;ms-received-port=49673;ms-received-cid=1C224C00
Via: SIP/2.0/TLS 10.77.44.43:65529;ms-received-port=65529;ms-received-cid=19F21100
Record-Route: <sip:skypepool.tele2.ru:5061;transport=tls;ms-fe=T2RU-SKYPE-04.corp.tele2.ru;opaque=state:T;lr;ms-identity=identity;ms-key-info=info;ms-ent-dest;ms-role-rs-from;ms-role-rs-to;ms-opaque=state:F:Ci.R15f2ec00;lr;ms-route-sig=bwvGnrdqFKQO4UnF2Q9LygIdCx-PqWkBQL4Ixnxhuhv_loczcdXdhgiQAA>;tag=ABE3F76F8257ED2EE384F72BB140EB8B
P-Asserted-Identity: "Лыкова Анна Николаевна"<sip:anna.lykova@tele2.ru>,<tel:+78442>
From: "Лыкова Анна Николаевна"<sip:anna.lykova@tele2.ru>;tag=cd9bbbf50f;epid=69eb994c65
To: <sip:service.trueconf@tele2.ru>;epid=0647bd7908
Call-ID: d980b0b933c9477ab6668c90f90ff4ef
CSeq: 1 INVITE
Contact: <sip:anna.lykova@tele2.ru;opaque=user:epid:yrmRjH4GQVqJor-JE9ciowAA;gruu>
User-Agent: UCCAPI/16.0.4591.1000 OC/16.0.4600.1000 (Skype for Business)
Supported: ms-dialog-route-set-update
Ms-Conversation-ID: AdPHUWJH0stCMXggSpG6tMex88orbg==
Supported: timer
Supported: histinfo
Supported: ms-safe-transfer
Supported: ms-sender
Supported: ms-early-media
Supported: 100rel
ms-keep-alive: UAC;hop-hop=yes
Allow: INVITE, BYE, ACK, CANCEL, INFO, UPDATE, REFER, NOTIFY, BENOTIFY, OPTIONS
ms-subnet: 10.77.44.0
Accept-Language: ru-RU
ms-endpoint-location-data: NetworkScope;ms-media-location-type=Intranet
Supported: replaces
Supported: ms-conf-invite
Content-Type: application/sdp
history-info: <sip:service.trueconf@tele2.ru>;index=1;ms-target-phone="tel:+79192"

v=0
o=- 0 0 IN IP4 10.77.44.43
s=session
c=IN IP4 10.77.44.43
b=CT:99980
t=0 0
a=x-devicecaps:audio:send,recv;video:recv
m=audio 50052 RTP/AVP 117 104 114 9 112 111 0 8 103 116 115 97 13 118 119 101
a=x-ssrc-range:2029455532-2029455532
a=rtcp-fb:* x-message app send:dsh recv:dsh
a=rtcp-rsize
a=label:main-audio
a=x-source:main-audio
a=ice-ufrag:tlMR
a=ice-pwd:FuwpNRsfT4UaOKa0ivlnB0n4
a=candidate:1 1 UDP 2130706431 10.77.44.43 50052 typ host
a=candidate:1 2 UDP 2130705918 10.77.44.43 50053 typ host
a=candidate:2 1 TCP-PASS 174456319 194.176.96.67 56148 typ relay raddr 10.77.44.43 rport 50024
a=candidate:2 2 TCP-PASS 174455806 194.176.96.67 56148 typ relay raddr 10.77.44.43 rport 50024
a=candidate:3 1 UDP 184548351 194.176.96.67 42037 typ relay raddr 10.77.44.43 rport 50048
a=candidate:3 2 UDP 184547838 194.176.96.67 49460 typ relay raddr 10.77.44.43 rport 50049
a=candidate:4 1 TCP-ACT 174848511 194.176.96.67 56148 typ relay raddr 10.77.44.43 rport 50024
a=candidate:4 2 TCP-ACT 174847998 194.176.96.67 56148 typ relay raddr 10.77.44.43 rport 50024
a=candidate:5 1 TCP-ACT 1684797439 10.77.44.43 50024 typ srflx raddr 10.77.44.43 rport 50024
a=candidate:5 2 TCP-ACT 1684796926 10.77.44.43 50024 typ srflx raddr 10.77.44.43 rport 50024
a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:IQ+Q4YcN8irlbEovMTkF8LL/uMV0JIGFWrAxrrpY|2^31|1:1
a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:IQ+Q4YcN8irlbEovMTkF8LL/uMV0JIGFWrAxrrpY|2^31|1:1
a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:IQ+Q4YcN8irlbEovMTkF8LL/uMV0JIGFWrAxrrpY|2^31
a=maxptime:200
a=rtcp:50053
a=rtpmap:117 G722/8000/2
a=rtpmap:104 SILK/16000
a=fmtp:104 useinbandfec=1; usedtx=0
a=rtpmap:114 x-msrta/16000
a=fmtp:114 bitrate=29000
a=rtpmap:9 G722/8000
a=rtpmap:112 G7221/16000
a=fmtp:112 bitrate=24000
a=rtpmap:111 SIREN/16000
a=fmtp:111 bitrate=16000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:103 SILK/8000
a=fmtp:103 useinbandfec=1; usedtx=0
a=rtpmap:116 AAL2-G726-32/8000
a=rtpmap:115 x-msrta/8000
a=fmtp:115 bitrate=11800
a=rtpmap:97 RED/8000
a=rtpmap:13 CN/8000
a=rtpmap:118 CN/16000
a=rtpmap:119 CN/24000
a=rtpmap:101 telephone-event/8000
a=fmtp:101 0-16
a=rtcp-mux
a=ptime:20
a=extmap:1 http:\\www.webrtc.org\experiments\rtp-hdrext\abs-send-time
)";

	const char WebinarCommand[] =
R"(MESSAGE sip:user6@192.168.41.195:49932;transport=tls SIP/2.0
Via: SIP/2.0/TLS 192.168.74.4:5061;branch=z9hG4bK7FE02052.1FBB8A05077FB342;branched=FALSE;ms-internal-info="du3m6BkBR7wD9iGtILDBkl15wPWbY8QXDztqYtR3nYAB8Firsf2b2EGQAA"
Authentication-Info: NTLM qop="auth", opaque="15BC1FAB", srand="1733CC1C", snum="137", rspauth="0100000000000000be5b76ce41998214", targetname="sarco04.skype2015.loc", realm="SIP Communications Service", version=4
Max-Forwards: 69
From: "user12"<sip:user12@skype2015.loc>;tag=d5f5402643;epid=72f0d99ead
Via: SIP/2.0/TLS 192.168.41.195:49556;ms-received-port=49556;ms-received-cid=37D400
To: <sip:user6@skype2015.loc>;tag=86C03CC95C88D786D1FE634D71091448;epid=61d55e0379
Call-ID: 7c08edfc64074ec880fd8a546be21f61
CSeq: 5 MESSAGE
User-Agent: UCCAPI/16.0.4405.1000 OC/16.0.4405.1000 (Skype for Business)
Supported: ms-dialog-route-set-update
Supported: timer
Content-Type: text/plain; charset=UTF-8
Content-Length: 23

/webinar us2 "Internal")";

	const char InvitePacket[] =
R"(INVITE sip:user2@192.168.41.195:1289;transport=tls SIP/2.0
Record-Route: <sip:LYNC2013-SERVER.lync.loc:5061;transport=tls;opaque=state:T:F:Ci.R64ef00;lr;ms-route-sig=ebVJJ-S35gvasUjOab5HXdn2UdCPA_8C0MeFDLLlXRYtLkFKIw-VaIIwAA>;tag=292144B2A02EF37152F543F63B226EF7
Via: SIP/2.0/TLS 192.168.74.12:5061;branch=z9hG4bKFC734704.30A214E4CBC708DC;branched=FALSE;ms-internal-info="ceS7-P9UWXzgsEVwuPuq3wuLT9xg93kbePLkUk68vQMNfkFKIwWGS1mgAA"
Authentication-Info: NTLM qop="auth", opaque="0EBA6584", srand="7882F039", snum="14", rspauth="0100000061673d46656fdaf8971e3788", targetname="LYNC2013-SERVER.lync.loc", realm="SIP Communications Service", version=4
Max-Forwards: 69
Content-Length: 4335
Via: SIP/2.0/TLS 192.168.41.195:1221;ms-received-port=1221;ms-received-cid=64E000
P-Asserted-Identity: "user1"<sip:user1@lync.loc>
From: "user1"<sip:user1@lync.loc>;tag=076fb7eacc;epid=4eb952ba52
To: <sip:user2@lync.loc>;epid=09a58443b8
Call-ID: a23ba67b2b0b4db5b10db2b1e7b5ca7c
CSeq: 1 INVITE
Contact: <sip:user1@lync.loc;opaque=user:epid:yCrEABeo2F-TVJ-yRKDmIQAA;gruu>
User-Agent: UCCAPI/16.0.4522.1000 OC/16.0.4522.1000 (Skype for Business)
Supported: ms-dialog-route-set-update
Ms-Conversation-ID: AdM4HIvgt2NHijMqSfaP72eXUqwraA==
Supported: timer
Supported: histinfo
Supported: ms-safe-transfer
Supported: ms-sender
Supported: ms-early-media
Supported: 100rel
ms-keep-alive: UAC;hop-hop=yes
Allow: INVITE, BYE, ACK, CANCEL, INFO, UPDATE, REFER, NOTIFY, BENOTIFY, OPTIONS
ms-subnet: 192.168.41.0
Accept-Language: ru-RU
ms-endpoint-location-data: NetworkScope;ms-media-location-type=Intranet
Supported: replaces
Supported: ms-conf-invite
Content-Type: application/sdp
history-info: <sip:user2@lync.loc>;index=1

v=0
o=- 0 0 IN IP4 192.168.56.1
s=session
c=IN IP4 192.168.56.1
b=CT:99980
t=0 0
a=x-mediabw:main-video send=12000;recv=12000
a=x-devicecaps:audio:send,recv;video:send,recv
m=audio 27100 RTP/SAVP 117 104 114 9 112 111 0 8 103 116 115 97 13 118 119 101
a=x-ssrc-range:1215169024-1215169024
a=rtcp-fb:* x-message app send:dsh recv:dsh
a=rtcp-rsize
a=label:main-audio
a=x-source:main-audio
a=ice-ufrag:lMRF
a=ice-pwd:oeNM9Rf6IT4BtUaOKa60ivx2
a=candidate:1 1 UDP 2130706431 192.168.56.1 27100 typ host
a=candidate:1 2 UDP 2130705918 192.168.56.1 27101 typ host
a=candidate:2 1 UDP 2130705919 192.168.88.1 21074 typ host
a=candidate:2 2 UDP 2130705406 192.168.88.1 21075 typ host
a=candidate:3 1 UDP 2130705407 192.168.49.2 2104 typ host
a=candidate:3 2 UDP 2130704894 192.168.49.2 2105 typ host
a=candidate:4 1 UDP 2130704895 192.168.11.1 2346 typ host
a=candidate:4 2 UDP 2130704382 192.168.11.1 2347 typ host
a=candidate:5 1 UDP 2130704383 192.168.41.195 24002 typ host
a=candidate:5 2 UDP 2130703870 192.168.41.195 24003 typ host
a=x-candidate-ipv6:6 1 UDP 2130703871 fd00:7:495:1::4c 14530 typ host
a=x-candidate-ipv6:6 2 UDP 2130703358 fd00:7:495:1::4c 14531 typ host
a=candidate:7 1 TCP-ACT 1684796415 192.168.56.1 27100 typ srflx raddr 192.168.56.1 rport 27100
a=candidate:7 2 TCP-ACT 1684795902 192.168.56.1 27100 typ srflx raddr 192.168.56.1 rport 27100
a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:3t+Jui/n92LdFt7lBWOYUC1wUg52ub4HAW4PzdDl|2^31|1:1
a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:3t+Jui/n92LdFt7lBWOYUC1wUg52ub4HAW4PzdDl|2^31|1:1
a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:3t+Jui/n92LdFt7lBWOYUC1wUg52ub4HAW4PzdDl|2^31
a=maxptime:200
a=rtcp:27101
a=rtpmap:117 G722/8000/2
a=rtpmap:104 SILK/16000
a=fmtp:104 useinbandfec=1; usedtx=0
a=rtpmap:114 x-msrta/16000
a=fmtp:114 bitrate=29000
a=rtpmap:9 G722/8000
a=rtpmap:112 G7221/16000
a=fmtp:112 bitrate=24000
a=rtpmap:111 SIREN/16000
a=fmtp:111 bitrate=16000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:103 SILK/8000
a=fmtp:103 useinbandfec=1; usedtx=0
a=rtpmap:116 AAL2-G726-32/8000
a=rtpmap:115 x-msrta/8000
a=fmtp:115 bitrate=11800
a=rtpmap:97 RED/8000
a=rtpmap:13 CN/8000
a=rtpmap:118 CN/16000
a=rtpmap:119 CN/24000
a=rtpmap:101 telephone-event/8000
a=fmtp:101 0-16
a=rtcp-mux
a=ptime:20
a=extmap:1 http:\\www.webrtc.org\experiments\rtp-hdrext\abs-send-time
m=video 16312 RTP/SAVP 122 121 123
a=x-ssrc-range:1215169025-1215169124
a=rtcp-fb:* x-message app send:src,x-pli recv:src,x-pli
a=rtcp-rsize
a=label:main-video
a=x-source:main-video
a=ice-ufrag:R3PS
a=ice-pwd:MQlXI6fnw5qgMejVCKOTps5O
a=x-caps:121 263:1920:1080:30.0:2000000:1;4359:1280:720:30.0:1500000:1;8455:640:480:30.0:600000:1;12551:640:360:30.0:600000:1;16647:352:288:15.0:250000:1;20743:424:240:15.0:250000:1;24839:176:144:15.0:180000:1
a=candidate:1 1 UDP 2130706431 192.168.56.1 16312 typ host
a=candidate:1 2 UDP 2130705918 192.168.56.1 16313 typ host
a=candidate:2 1 UDP 2130705919 192.168.88.1 23632 typ host
a=candidate:2 2 UDP 2130705406 192.168.88.1 23633 typ host
a=candidate:3 1 UDP 2130705407 192.168.49.2 4700 typ host
a=candidate:3 2 UDP 2130704894 192.168.49.2 4701 typ host
a=candidate:4 1 UDP 2130704895 192.168.11.1 4588 typ host
a=candidate:4 2 UDP 2130704382 192.168.11.1 4589 typ host
a=candidate:5 1 UDP 2130704383 192.168.41.195 21334 typ host
a=candidate:5 2 UDP 2130703870 192.168.41.195 21335 typ host
a=x-candidate-ipv6:6 1 UDP 2130703871 fd00:7:495:1::4c 10802 typ host
a=x-candidate-ipv6:6 2 UDP 2130703358 fd00:7:495:1::4c 10803 typ host
a=candidate:7 1 TCP-ACT 1684796415 192.168.56.1 16312 typ srflx raddr 192.168.56.1 rport 16312
a=candidate:7 2 TCP-ACT 1684795902 192.168.56.1 16312 typ srflx raddr 192.168.56.1 rport 16312
a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:3t+Jui/n92LdFt7lBWOYUC1wUg52ub4HAW4PzdDl|2^31|1:1
a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:3t+Jui/n92LdFt7lBWOYUC1wUg52ub4HAW4PzdDl|2^31|1:1
a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:3t+Jui/n92LdFt7lBWOYUC1wUg52ub4HAW4PzdDl|2^31
a=rtcp:16313
a=rtpmap:122 X-H264UC/90000
a=fmtp:122 packetization-mode=1;mst-mode=NI-TC
a=rtpmap:121 x-rtvc1/90000
a=rtpmap:123 x-ulpfecuc/90000
a=rtcp-mux
a=extmap:1 http:\\www.webrtc.org\experiments\rtp-hdrext\abs-send-time)"
"\r\n\r\n";

const char registerSIP[] =
R"(REGISTER sip:192.168.56.1;transport=tcp SIP/2.0
Via: SIP/2.0/TCP 10.0.2.15:50869;rport;branch=z9hG4bKPj934325b544794c8890829b9194bcc1ea;alias
Route: <sip:192.168.56.1;transport=tcp;lr>
Max-Forwards: 70
From: <sip:us1@ub1kb.trueconf.name>;tag=504829697c6b47d2b3351af5a19f4d43
To: <sip:us1@ub1kb.trueconf.name>
Call-ID: 9d9e89325be34fc2868bcab4d9562bb9
CSeq: 43060 REGISTER
User-Agent: MicroSIP/3.19.10
Supported: outbound, path
Contact: <sip:us1@10.0.2.15:50869;transport=TCP;ob>;reg-id=1;+sip.instance="<urn:uuid:00000000-0000-0000-0000-0000bedb1486>"
Expires: 300
Allow: PRACK, INVITE, ACK, BYE, CANCEL, UPDATE, INFO, SUBSCRIBE, NOTIFY, REFER, MESSAGE, OPTIONS
Authorization: Digest username="us1", realm="trueconf", nonce="05e19bc460f3a22f7fb3b6cd603841a1", uri="sip:192.168.56.1;transport=tcp", response="bc4b0b3592426715ce2e4bcbc2eade7d"
Content-Length:  0)""\r\n\r\n";

const char registerSIP_Nonce[] =
R"(REGISTER sip:192.168.56.1;transport=tcp SIP/2.0
Via: SIP/2.0/TCP 10.0.2.15:50869;rport;branch=z9hG4bKPj934325b544794c8890829b9194bcc1ea;alias
Route: <sip:192.168.56.1;transport=tcp;lr>
Max-Forwards: 70
From: <sip:us1@ub1kb.trueconf.name>;tag=504829697c6b47d2b3351af5a19f4d43
To: <sip:us1@ub1kb.trueconf.name>
Call-ID: 9d9e89325be34fc2868bcab4d9562bb9
CSeq: 43060 REGISTER
User-Agent: MicroSIP/3.19.10
Supported: outbound, path
Contact: <sip:us1@10.0.2.15:50869;transport=TCP;ob>;reg-id=1;+sip.instance="<urn:uuid:00000000-0000-0000-0000-0000bedb1486>"
Expires: 300
Allow: PRACK, INVITE, ACK, BYE, CANCEL, UPDATE, INFO, SUBSCRIBE, NOTIFY, REFER, MESSAGE, OPTIONS
Authorization: Digest username="us1", realm="trueconf", nonce="%s", uri="sip:192.168.56.1;transport=tcp", response="bc4b0b3592426715ce2e4bcbc2eade7d"
Content-Length:  0)""\r\n\r\n";

} /* namespace sipraw */

#endif /* SIP_RAW_H */
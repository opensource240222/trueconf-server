
char raw_sip_message_Bug42320_Invite[] =
"INVITE sip:__invite__ SIP/2.0\r\n"
"Via: SIP/2.0/UDP 192.168.62.26:5060;branch=z9hG4bK32d399346511e2f9473ae2cbf12f31b3;rport\r\n"
"From: \"222\" <sip:222@192.168.62.26>;tag=821229615\r\n"
"To: <sip:__invite__>\r\n"
"Call-ID: 3326795752@192_168_62_26\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:222@192.168.62.26:5060>\r\n"
"Max-Forwards: 70\r\n"
"User-Agent: C610A IP/42.076.00.000.000\r\n"
"Supported: replaces\r\n"
"Allow-Events: message-summary, refer, ua-profile\r\n"
"Allow: INVITE, ACK, CANCEL, BYE, OPTIONS, INFO, REFER, SUBSCRIBE, NOTIFY\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: __contentlength__\r\n"
;

char raw_sip_message_423_Interval_Too_Brief[] =

"SIP/2.0 423 Interval Too Brief\r\n"
"From: <sip:7758@213.133.168.206>;tag=F49A82AD3702EA84B08B47CE6167FF0A\r\n"
"To: <sip:7758@213.133.168.206>;tag=464de48-ac100a0b-13c4-50022-278c6c-1b6ef38a-278c6c\r\n"
"Call-ID: __callid__\r\n"
"CSeq: __cseq__ REGISTER\r\n"
"Via: SIP/2.0/UDP 192.168.62.141:64169;received=85.202.225.51;branch=__branch__\r\n"
"User-Agent: OfficeServ 7400\r\n"
"Min-Expires: 3200\r\n"
"Content-Length: 0\r\n\r\n"
;


char raw_sip_message_CommonInvite[] =
"INVITE sip:111@192.168.62.141 SIP/2.0\r\n"
"Via: SIP/2.0/UDP 192.168.62.26:5060;branch=z9hG4bK32d399346511e2f9473ae2cbf12f31b3;rport\r\n"
"From: \"222\" <sip:222@192.168.62.26>;tag=821229615\r\n"
"To: <sip:111@192.168.62.141>\r\n"
"Call-ID: 3326795752@192_168_62_26\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:222@192.168.62.26:5060>\r\n"
"Max-Forwards: 70\r\n"
"User-Agent: C610A IP/42.076.00.000.000\r\n"
"Supported: replaces\r\n"
"Allow-Events: message-summary, refer, ua-profile\r\n"
"Allow: INVITE, ACK, CANCEL, BYE, OPTIONS, INFO, REFER, SUBSCRIBE, NOTIFY\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: __contentlength__\r\n"
;

char raw_sip_message_Common180Ringing[] = "\
SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/UDP 192.168.62.26:5060;branch=__branch__\r\n\
From: \"222\" <sip:222@192.168.62.26>;tag=821229615\r\n\
To: <sip:111@192.168.62.141>;tag=516922128\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:111@192.168.62.141:5060>\r\n\
Content-Length: 0\r\n\r\n\
";

char raw_sip_message_Common200OK[] = "\
SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP 192.168.62.26:5060;branch=__branch__\r\n\
From: \"222\" <sip:222@192.168.62.26>;tag=821229615\r\n\
To: <sip:111@192.168.62.141>\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:111@192.168.62.141:5060>\r\n\
Content-Type: application/sdp\r\n\
Content-Length: __contentlength__\r\n\
";

char raw_sip_message_SDP_G722_PCMA[] =
"v=0\r\n"
"o=222 5020 8 IN IP4 192.168.62.26\r\n"
"s=Mapping\r\n"
"c=IN IP4 192.168.62.26\r\n"
"t=0 0\r\n"
"m=audio 5020 RTP/AVP 9 8\r\n"
"a=rtpmap:9 G722/8000\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
;

char raw_sip_message_SDP_G729_PCMA[] =
"v=0\r\n"
"o=222 5020 8 IN IP4 192.168.62.26\r\n"
"s=Mapping\r\n"
"c=IN IP4 192.168.62.26\r\n"
"t=0 0\r\n"
"m=audio 5020 RTP/AVP 9 18\r\n"
"a=rtpmap:9 PCMA/8000\r\n"
"a=rtpmap:18 G729/8000\r\n"
"a=fmtp:18 annexb=no\r\n"
;

char raw_sip_message_Bug23517_180[] =
"SIP/2.0 180 Ringing\r\n"
"Via: SIP/2.0/TCP 10.174.0.70:60610;branch=__branch__;received=10.174.0.70\r\n"
"Call-ID: __callid__\r\n"
"CSeq: 1 INVITE\r\n"
"From: \"varakina.ev@reg52.mvd\" <sip:varakina.ev@reg52.mvd>;tag=97E7B098C0EEC5EDD04AB60CD254EF0A\r\n"
"To: <sip:10.174.140.80>;tag=64910b49edc2f1bc\r\n"
"Server: TANDBERG/81 (F9.1 PAL)\r\n"
"Content-Length: 0\r\n\r\n"
;

char raw_sip_message_Bug23517_200[] =
"SIP/2.0 200 OK\r\n"
"Via: SIP/2.0/TCP 10.174.0.70:60610;branch=__branch__;received=10.174.0.70\r\n"
"Call-ID: __callid__\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:10.174.140.80@10.174.140.80:5060;transport=tcp>\r\n"
"From: \"varakina.ev@reg52.mvd\" <sip:varakina.ev@reg52.mvd>;tag=97E7B098C0EEC5EDD04AB60CD254EF0A\r\n"
"To: <sip:10.174.140.80>;tag=64910b49edc2f1bc\r\n"
"Allow: INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY\r\n"
"Server: TANDBERG/81 (F9.1 PAL)\r\n"
"Supported: replaces,100rel,timer\r\n"
"Session-Expires: 1800;refresher=uas\r\n"
"Min-SE: 90\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: __contentlength__\r\n"
;

char raw_sip_message_Bug23517_Invite[] =
"INVITE sip:10.174.0.70:60610;transport=tcp SIP/2.0\r\n"
"Via: SIP/2.0/TCP 10.174.140.80:5060;branch=z9hG4bK8c84356575f3e57f8b88a6ed92ac3ec4.1;rport\r\n"
"Call-ID: __callid__\r\n"
"CSeq: 501 INVITE\r\n"
"Contact: <sip:10.174.140.80@10.174.140.80:5060;transport=tcp>\r\n"
"From: <sip:10.174.140.80>;tag=64910b49edc2f1bc\r\n"
"To: \"varakina.ev@reg52.mvd\" <sip:varakina.ev@reg52.mvd>;tag=97E7B098C0EEC5EDD04AB60CD254EF0A\r\n"
"Max-Forwards: 70\r\n"
"Allow: INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY\r\n"
"User-Agent: TANDBERG/81 (F9.1 PAL)\r\n"
"Supported: replaces,100rel,timer\r\n"
"Session-Expires: 1800;refresher=uac\r\n"
"Min-SE: 90\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: __contentlength__\r\n"
;

char raw_sip_message_unauth[] =
"SIP/2.0 401 Unauthorized\r\n"
"Via: SIP/2.0/UDP 10.10.10.63:60498;branch=__branch__;rport=60498\r\n"
"To: \"PhonerLite\" <sip:107@192.168.40.150>;tag=25386\r\n"
"From: \"PhonerLite\" <sip:107@192.168.40.150>;tag=3311252059\r\n"
"Call-ID: __callid__\r\n"
"CSeq: 1 REGISTER\r\n"
"Allow: INVITE,ACK,CANCEL,BYE,REGISTER\r\n"
"WWW-Authenticate: Digest realm=\"Registered Users\",nonce=\"1c3972e5ca952b57af5fbf7ffefcf8f0\",algorithm=MD5\r\n"
"Content-Length: 0\r\n\r\n"
;

char raw_sip_message_test1[] =
"REGISTER sip:185.22.184.14:5060 SIP/2.0\r\n"
"Via: SIP/2.0/TCP 192.168.0.200:5060;branch=z9hG4bK3741300489-812\r\n"
"Max-Forwards: 70\r\n"
"Allow: INVITE,BYE,CANCEL,ACK,INFO,PRACK,COMET,OPTIONS,SUBSCRIBE,NOTIFY,MESSAGE,REFER,REGISTER,UPDATE\r\n"
"From: 185.22.184.14 <sip:185.22.184.14@185.22.184.14>;tag=plcm_3741300540-812;epid=8213020FD0F0CV\r\n"
"To: <sip:185.22.184.14@185.22.184.14>\r\n"
"Call-ID: 3741299705-812\r\n"
"CSeq: 1 REGISTER\r\n"
"Expires: 300\r\n"
"Contact: 185.22.184.14 <sip:185.22.184.14@192.168.0.200:5060;transport=tcp>;proxy=replace;+sip.instance=\"<urn:uuid:e8c7bf50-c217-51c8-a203-227ca4874333>\"\r\n"
"ms-keep-alive:UAC;hop-hop=yes\r\n"
"P_Preferred_Identity:Group Series9\r\n"
"Ms-Device-Info:MAC=00:E0:DB:0F:D0:F0 ,vendor=POLYCOM,version=PolycomRealPresenceGroup500/4.1.5\r\n"
"Event:registration\r\n"
"User-Agent:PolycomRealPresenceGroup500/4.1.5\r\n"
"Supported: timer,replaces,ms-dialog-route-set-update,ms-forking,msrtc-event-categories,gruu-10,ms-userservices-state-notification,ms-cluster-failover\r\n"
"Content-Length: 0\r\n\r\n"
;

char raw_sip_message_ru_ticket_6439[] =
"INVITE sip:10.62.32.85:5060 SIP/2.0\r\n"
"Via: SIP/2.0/UDP 10.62.255.1:5060;x-ds0num=\"ISDN 0 / 0 / 1:15 0 / 0 / 1 : DS1 9 : DS0\";branch=z9hG4bK1281B9C\r\n"
"Remote-Party-ID: <sip:214346@10.62.255.1>;party=calling;screen=no;privacy=off\r\n"
"From: <sip:214346@10.62.255.1>;tag=49473B64-19D5\r\n"
"To: <sip:10.62.32.85>\r\n"
"Date: Thu, 11 Jan 2018 13:20:50 GMT\r\n"
"Call-ID: 13CE6F26-F60911E7-995AB81E-4CDEC078@10.62.255.1\r\n"
"Supported: 100rel,timer,resource-priority,replaces,sdp-anat\r\n"
"Min-SE:  1800\r\n"
"Cisco-Guid: 0323255855-4127789543-2194997273-1451141520\r\n"
"User-Agent: Cisco-SIPGateway/IOS-12.x\r\n"
"Allow: INVITE, OPTIONS, BYE, CANCEL, ACK, PRACK, UPDATE, REFER, SUBSCRIBE, NOTIFY, INFO, REGISTER\r\n"
"CSeq: 101 INVITE\r\n"
"Max-Forwards: 70\r\n"
"Timestamp: 1515676850\r\n"
"Contact: <sip:214346@10.62.255.1:5060>\r\n"
"Expires: 180\r\n"
"Allow-Events: telephone-event\r\n"
"Content-Type: application/sdp\r\n"
"Content-Disposition: session;handling=required\r\n"
"Content-Length: 303\r\n"
"\r\n"
"v=0\r\n"
"o=CiscoSystemsSIP-GW-UserAgent 4351 8658 IN IP4 10.62.255.1\r\n"
"s=SIP Call\r\n"
"c=IN IP4 10.62.255.1\r\n"
"t=0 0\r\n"
"m=audio 16782 RTP/AVP 8 4 18 0\r\n"
"c=IN IP4 10.62.255.1\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=rtpmap:4 G723/8000\r\n"
"a=fmtp:4 bitrate=6.3;annexa=no\r\n"
"a=rtpmap:18 G729/8000\r\n"
"a=fmtp:18 annexb=no\r\n"
"a=rtpmap:0 PCMU/8000\r\n"
;

char raw_sip_message_ImageCom_crash[] =
"OPTIONS sip:nm SIP/2.0\r\n"
"Via: SIP/2.0/TCP nm;branch=foo\r\n"
"From: <sip:nm@nm>;tag=root\r\n"
"To: <sip:nm2@nm2>\r\n"
"Call-ID: 50000\r\n"
"CSeq: 42 OPTIONS\r\n"
"Max-Forwards: 70\r\n"
"Content-Length: 0\r\n"
"Contact: <sip:nm@nm>\r\n"
"Accept: application/sdp\r\n"
"\r\n"
;

char raw_sdp_generic[] =
"v=0\r\n"
"o=a 30317 0 IN IP4 192.168.0.104\r\n"
"s=-\r\n"
"c=IN IP4 192.168.0.104\r\n"
"b=AS:384\r\n"
"t=0 0\r\n"
"m=audio 5062 RTP/AVP 0 8\r\n"
"a=rtpmap:0 PCMU/8000\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=sendrecv\r\n"
"m=video 5064 RTP/AVP 109 34 96 31\r\n"
"b=TIAS:384000\r\n"
"a=rtpmap:109 H264/90000\r\n"
"a=fmtp:109 profile-level-id=42800d; max-mbps=40000; max-fs=1792; max-br=1600\r\n"
"a=rtpmap:34 H263/90000\r\n"
"a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F\r\n"
"a=rtpmap:96 H263-1998/90000\r\n"
"a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;F;J;T\r\n"
"a=rtpmap:31 H261/90000\r\n"
"a=fmtp:31 CIF=1;QCIF=1\r\n"
"a=sendrecv\r\n"
"m=application 5066 RTP/AVP 100\r\n"
"a=rtpmap:100 H224/0\r\n"
"a=sendrecv\r\n"
;

char raw_cisco_ex60_at_PMO[] =
"REGISTER sip:10.4.126.96 SIP/2.0\r\n"
"Via: SIP/2.0/UDP 10.1.5.234:5060;branch=z9hG4bK3ea4b6be1ed91feee02a4124de0dd3fb.1;rport\r\n"
"Call-ID: 6a58cd75c1fc28c5fa7f838a13ebe934\r\n"
"CSeq: 36913 REGISTER\r\n"
"Contact: <sip:0ciscovcs@10.1.5.234:5060>;+sip.instance=\"<urn:uuid:00000000-0000-0000-0000-e8edf3b5f638>\";+u.sip!model.ccm.cisco.com=\"604\";+u.sip!serialno.ccm.cisco.com=A1AZ38F00082;audio=TRUE;video=TRUE;mobility=\"fixed\";duplex=\"full\";description=\"TANDBERG-SIP\"\r\n"
"From: <sip:0ciscovcs@10.4.126.96>;tag=73cfcbbbe617bb59\r\n"
"To: <sip:0ciscovcs@10.4.126.96>\r\n"
"Max-Forwards: 70\r\n"
"Route: <sip:10.4.126.96;lr>\r\n"
"Allow: INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY\r\n"
"User-Agent: TANDBERG/518 (TCNC6.2.0.20b1616)\r\n"
"Expires: 3600\r\n"
"Authorization: Digest nonce=\"461baeffbc0d6d562207716595008fd6\", realm=\"trueconf\", username=\"0ciscovcs\", uri=\"sip:10.4.126.96\", response=\"a6eac2cf723b90f659b08937d645bfb1\", algorithm=MD5\r\n"
"Supported: replaces,100rel,timer,gruu,path,outbound,X-cisco-serviceuri,X-cisco-callinfo,X-cisco-service-control,X-cisco-srtp-fallback,X-cisco-sis-6.0.0,norefersub,extended-refer\r\n"
"Reason: SIP ;cause=200;text=\"cisco-alarm:25 Name=SEPE8EDF3B5F638 ActiveLoad=TCNC6.2.0.20b1616 Last=Device-Initiated-Reset\"\r\n"
"Content-Type: multipart/mixed; boundary=uniqueBoundary\r\n"
"Content-Length: 302\r\n"
"\r\n"
"--uniqueBoundary\r\n"
"Content-Type: application/x-cisco-remotecc-request+xml\r\n"
"Content-Disposition: session;handling=optional\r\n"
"\r\n"
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"<x-cisco-remotecc-request>\r\n"
"<optionsind>\r\n"
"<joinreq/>\r\n"
"<x-cisco-number/>\r\n"
"<ix/>\r\n"
"</optionsind>\r\n"
"</x-cisco-remotecc-request>\r\n"
"\r\n"
"--uniqueBoundary--"
;

char raw_cisco_c60_crash[] =
"\r\n\r\n\r\n\r\n";

char raw_TANDBERG_772_INVITE[] =
"INVITE sip:jason2@panosh.id.au SIP/2.0\r\n\
Via: SIP/2.0/TCP 203.188.221.150:5060;egress-zone=DNS;branch=z9hG4bK0b6a859a139e61fbce736b2fdcdb69e42131388.a9207717acce5a30ae884774563ac252;proxy-call-id=df5ea190-8f41-11e5-9a74-0010f31fd402;rport\r\n\
Via: SIP/2.0/TLS 192.168.1.8:60888;branch=z9hG4bK5fb2a3a89354ffa1c5256467fcdb7424.1;received=203.219.27.45;rport=60888;ingress-zone=DefaultSubZone\r\n\
Call-ID: 6537df3c49b1c298@192.168.1.8\r\n\
CSeq: 100 INVITE\r\n\
Contact: <sip:jason.panosh@vc.grha.org.au;gr=urn:uuid:998e8de6-7d02-5d23-abb3-7e070e1deb8c>\r\n\
From: \"Jason Panosh\" <sip:jason.panosh@vc.grha.org.au>;tag=944c7fa9c27192bc\r\n\
To: <sip:jason2@panosh.id.au>\r\n\
Max-Forwards: 15\r\n\
Record-Route: <sip:proxy-call-id=df5ea190-8f41-11e5-9a74-0010f31fd402@203.188.221.150:5060;transport=tcp;lr>\r\n\
Record-Route: <sip:proxy-call-id=df5ea190-8f41-11e5-9a74-0010f31fd402@203.188.221.150:5061;transport=tls;lr>\r\n\
Allow: INVITE,ACK,CANCEL,BYE,INFO,OPTIONS,REFER,NOTIFY\r\n\
User-Agent: TANDBERG/772 (MCX 4.3.12.13351) - Windows\r\n\
Supported: replaces,timer,gruu\r\n\
Session-Expires: 1800\r\n\
P-Asserted-Identity: \"Jason Panosh\" <sip:jason.panosh@vc.grha.org.au>\r\n\
X-TAATag: df5ea32a-8f41-11e5-a7b5-0010f31fd402\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 1962\r\n\
\r\n\
v=0\r\n\
o=tandberg 1 2 IN IP4 192.168.1.8\r\n\
s=-\r\n\
c=IN IP4 203.188.221.150\r\n\
b=AS:384\r\n\
t=0 0\r\n\
m=audio 52120 RTP/AVP 127 120 121 8 0 11 100\r\n\
b=TIAS:384000\r\n\
a=rtpmap:127 MP4A-LATM/90000\r\n\
a=fmtp:127 profile-level-id=24;object=23;bitrate=64000\r\n\
a=rtpmap:120 G7221/16000\r\n\
a=fmtp:120 bitrate=24000\r\n\
a=rtpmap:121 G7221/16000\r\n\
a=fmtp:121 bitrate=32000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:11 L16/48000\r\n\
a=rtpmap:100 telephone-event/8000\r\n\
a=fmtp:100 0-15\r\n\
a=sendrecv\r\n\
m=video 52130 RTP/AVP 123 122 97 34\r\n\
b=TIAS:384000\r\n\
a=rtpmap:123 H264/90000\r\n\
a=fmtp:123 profile-level-id=42801f;max-mbps=108000;max-fs=3600;max-smbps=323500;packetization-mode=1\r\n\
a=rtpmap:122 H264/90000\r\n\
a=fmtp:122 profile-level-id=42801f;max-mbps=108000;max-fs=3600;max-smbps=323500\r\n\
a=rtpmap:97 H263-1998/90000\r\n\
a=fmtp:97 custom=1280,720,1;custom=1024,768,1;custom=1024,576,1;custom=800,600,1;cif4=1;custom=720,480,1;custom=640,480,1;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;sqcif=1\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 cif4=1;cif=1;qcif=1;sqcif=1\r\n\
a=rtcp-fb:* nack pli\r\n\
a=rtcp-fb:* ccm fir\r\n\
a=sendrecv\r\n\
a=content:main\r\n\
a=label:11\r\n\
a=answer:full\r\n\
m=video 52128 RTP/AVP 123 122 97 34\r\n\
b=TIAS:384000\r\n\
a=rtpmap:123 H264/90000\r\n\
a=fmtp:123 profile-level-id=428028;max-mbps=245760;max-fs=8192;max-smbps=323500;packetization-mode=1\r\n\
a=rtpmap:122 H264/90000\r\n\
a=fmtp:122 profile-level-id=428028;max-mbps=245760;max-fs=8192;max-smbps=323500\r\n\
a=rtpmap:97 H263-1998/90000\r\n\
a=fmtp:97 custom=1280,720,1;custom=1024,768,1;custom=1024,576,1;custom=800,600,1;cif4=1;custom=720,480,1;custom=640,480,1;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;sqcif=1\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 cif4=1;cif=1;qcif=1;sqcif=1\r\n\
a=rtcp-fb:* nack pli\r\n\
a=rtcp-fb:* ccm fir\r\n\
a=sendrecv\r\n\
a=content:slides\r\n\
a=label:12\r\n\
m=application 52126 UDP/BFCP *\r\n\
a=floorctrl:c-s\r\n\
a=confid:1\r\n\
a=floorid:2 mstrm:12\r\n\
a=userid:1\r\n\
a=setup:actpass\r\n\
a=connection:new\r\n\
m=application 52134 RTP/AVP 105\r\n\
a=rtpmap:105 H224/4800\r\n\
a=sendrecv\r\n";

char raw_sdp_bfcp_c_s[] = "\
v=0\r\n\
o=hdx8000 2024860525 0 IN IP4 192.168.62.113\r\n\
s=-\r\n\
c=IN IP4 192.168.62.113\r\n\
b=AS:384\r\n\
t=0 0\r\n\
m=audio 49426 RTP/AVP 115 102 9 15 0 8 18\r\n\
a=rtpmap:115 G7221/32000\r\n\
a=fmtp:115 bitrate=48000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=32000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:18 G729/8000\r\n\
a=fmtp:18 annexb=no\r\n\
a=sendrecv\r\n\
m=video 49428 RTP/AVP 109 96 34 31\r\n\
b=TIAS:384000\r\n\
a=rtpmap:109 H264/90000\r\n\
a=fmtp:109 profile-level-id=42800d; max-mbps=108000; max-fs=3600; max-br=1600; sar=13\r\n\
a=rtpmap:96 H263-1998/90000\r\n\
a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1;F;J;T\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1;F\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=1;QCIF=1\r\n\
a=sendrecv\r\n\
a=rtcp-fb:* ccm fir tmmbr\r\n\
m=application 34683 TCP/BFCP *\r\n\
a=floorctrl:c-s\r\n\
a=setup:actpass\r\n\
a=connection:new\r\n\
";

char raw_sdp_bfcp_c_only[] = "\
v=0\r\n\
o=hdx8000 2024860525 0 IN IP4 192.168.62.113\r\n\
s=-\r\n\
c=IN IP4 192.168.62.113\r\n\
b=AS:384\r\n\
t=0 0\r\n\
m=audio 49426 RTP/AVP 115 102 9 15 0 8 18\r\n\
a=rtpmap:115 G7221/32000\r\n\
a=fmtp:115 bitrate=48000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=32000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:18 G729/8000\r\n\
a=fmtp:18 annexb=no\r\n\
a=sendrecv\r\n\
m=video 49428 RTP/AVP 109 96 34 31\r\n\
b=TIAS:384000\r\n\
a=rtpmap:109 H264/90000\r\n\
a=fmtp:109 profile-level-id=42800d; max-mbps=108000; max-fs=3600; max-br=1600; sar=13\r\n\
a=rtpmap:96 H263-1998/90000\r\n\
a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1;F;J;T\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1;F\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=1;QCIF=1\r\n\
a=sendrecv\r\n\
a=rtcp-fb:* ccm fir tmmbr\r\n\
m=application 34683 TCP/BFCP *\r\n\
a=floorctrl:c-only\r\n\
a=setup:active\r\n\
a=connection:new\r\n\
";

char raw_sdp_bfcp_s_only[] = "\
v=0\r\n\
o=hdx8000 2024860525 0 IN IP4 192.168.62.113\r\n\
s=-\r\n\
c=IN IP4 192.168.62.113\r\n\
b=AS:384\r\n\
t=0 0\r\n\
m=audio 49426 RTP/AVP 115 102 9 15 0 8 18\r\n\
a=rtpmap:115 G7221/32000\r\n\
a=fmtp:115 bitrate=48000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=32000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:18 G729/8000\r\n\
a=fmtp:18 annexb=no\r\n\
a=sendrecv\r\n\
m=video 49428 RTP/AVP 109 96 34 31\r\n\
b=TIAS:384000\r\n\
a=rtpmap:109 H264/90000\r\n\
a=fmtp:109 profile-level-id=42800d; max-mbps=108000; max-fs=3600; max-br=1600; sar=13\r\n\
a=rtpmap:96 H263-1998/90000\r\n\
a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1;F;J;T\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1;F\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=1;QCIF=1\r\n\
a=sendrecv\r\n\
a=rtcp-fb:* ccm fir tmmbr\r\n\
m=application 34683 TCP/BFCP *\r\n\
a=floorctrl:s-only\r\n\
a=setup:passive\r\n\
a=connection:new\r\n\
a=confid:1\r\n\
a=userid:2\r\n\
a=floorid:1 mstrm:3\r\n\
";

char raw_sip_message_not_our_nonce[] =
"REGISTER sip:1.1.1.1:5060 SIP/2.0\r\n"
"Via: SIP/2.0/TCP 1.1.1.2:5060;branch=z9hG4bK_fake\r\n"
"Max-Forwards: 70\r\n"
"From: <sip:fake_user@1.1.1.1>;tag=fake_tag\r\n"
"To: <sip:fake_user@1.1.1.1>\r\n"
"Call-ID: fake_call_id\r\n"
"CSeq: 1 REGISTER\r\n"
"Expires: 300\r\n"
"Contact: <sip:fake_user@1.1.1.2:5060;transport=tcp>\r\n"
"Authorization: Digest nonce=\"fake1\", realm=\"fake2\", username=\"fake3\", uri=\"fake4\", response=\"01234567890123456789012345678912\", algorithm=MD5\r\n"
"Content-Length: 0\r\n\r\n"
;

char raw_hdx8000_ipv6_invite[] =
"INVITE sip:b@[fd00:7:495:1::15] SIP/2.0\r\n\
Via: SIP/2.0/UDP [fd00:7:495:0:2e0:dbff:fe08:aac0]:5060;branch=z9hG4bK3285719264-287341512\r\n\
Max-Forwards: 70\r\n\
Allow: INVITE,BYE,CANCEL,ACK,INFO,PRACK,COMET,OPTIONS,SUBSCRIBE,NOTIFY,REFER,REGISTER,UPDATE\r\n\
Supported: ms-forking,timer,replaces\r\n\
From: hdx8000 <sip:hdx8000@[fd00:7:495:0:2e0:dbff:fe08:aac0]> ;epid=82075008AAC0CG;tag=plcm_3285719264-287341513\r\n\
To:  <sip:b@[fd00:7:495:1::15]> \r\n\
Call-ID: 3285718264-287341511\r\n\
CSeq: 1 INVITE\r\n\
Session-Expires: 90\r\n\
Contact: hdx8000 <sip:hdx8000@[fd00:7:495:0:2e0:dbff:fe08:aac0]:5060;transport=udp> ;proxy=replace\r\n\
User-Agent: Polycom HDX 8000 HD (Release - 2.5.0.8_ne-4035)\r\n\
P_Preferred_Identity: \u041f\u0410\u041b\u0418\u041a\u041e\u041c\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 888\r\n\
\r\n\
v=0\r\n\
o=hdx8000 237992196 0 IN IP6 fd00:7:495:0:2e0:dbff:fe08:aac0\r\n\
s=-\r\n\
c=IN IP6 fd00:7:495:0:2e0:dbff:fe08:aac0\r\n\
b=AS:1920\r\n\
t=0 0\r\n\
m=audio 3230 RTP/AVP 115 102 9 15 0 8 18\r\n\
a=rtpmap:115 G7221/32000\r\n\
a=fmtp:115 bitrate=48000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=32000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:18 G729/8000\r\n\
a=fmtp:18 annexb=no\r\n\
a=sendrecv\r\n\
m=video 3232 RTP/AVP 109 96 34 31\r\n\
b=TIAS:1920000\r\n\
a=rtpmap:109 H264/90000\r\n\
a=fmtp:109 profile-level-id=42800d; max-mbps=108000; max-fs=3600; max-br=1600; sar=13\r\n\
a=rtpmap:96 H263-1998/90000\r\n\
a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=1;QCIF=1\r\n\
a=sendrecv\r\n\
a=rtcp-fb:* ccm fir tmmbr\r\n\
m=application 34600 TCP/BFCP *\r\n\
a=floorctrl:c-s\r\n\
a=setup:actpass\r\n\
a=connection:new\r\n";

char raw_hdx8000_ipv6_ok[] =
"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP [fd00:7:495:1::15]:49808;received=[fd00:7:495:1::15];branch=__branch__\r\n\
From: \"b\" <sip:b@brchk000.trueconf.ua> ;tag=AD1DB0A906C366DE3DBB1C322DB4BD54\r\n\
To:  <sip:hdx8000@[fd00:7:495:0:2e0:dbff:fe08:aac0]> ;tag=plcm_3465955936-287161146\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: hdx8000 <sip:hdx8000@[fd00:7:495:0:2e0:dbff:fe08:aac0]:5060;transport=tcp> ;proxy=replace\r\n\
Allow: INVITE,BYE,CANCEL,ACK,INFO,PRACK,COMET,OPTIONS,SUBSCRIBE,NOTIFY,REFER,REGISTER,UPDATE\r\n\
User-Agent: Polycom HDX 8000 HD (Release - 2.5.0.8_ne-4035)\r\n\
P_Preferred_Identity: \u041f\u0410\u041b\u0418\u041a\u041e\u041c\r\n\
Supported: ms-forking\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 805\r\n\
\r\n\
v=0\r\n\
o=hdx8000 1783831774 0 IN IP6 fd00:7:495:0:2e0:dbff:fe08:aac0\r\n\
s=-\r\n\
c=IN IP6 fd00:7:495:0:2e0:dbff:fe08:aac0\r\n\
b=AS:1536\r\n\
t=0 0\r\n\
m=audio 3230 RTP/AVP 115 102 9 15 0 8 18\r\n\
a=rtpmap:115 G7221/32000\r\n\
a=fmtp:115 bitrate=48000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=32000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:18 G729/8000\r\n\
a=fmtp:18 annexb=no\r\n\
a=sendrecv\r\n\
m=video 3232 RTP/AVP 109 96 34 31\r\n\
b=TIAS:1536000\r\n\
a=rtpmap:109 H264/90000\r\n\
a=fmtp:109 profile-level-id=42800d; max-mbps=108000; max-fs=3600; max-br=1600; sar=13\r\n\
a=rtpmap:96 H263-1998/90000\r\n\
a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=1;QCIF=1\r\n\
a=sendrecv\r\n\
a=rtcp-fb:* ccm fir tmmbr\r\n";

char raw_cisco_e30_register[] =
"REGISTER sip:sip SIP/2.0\r\n"
"Via: SIP/2.0/TCP 192.168.62.43:5060;branch=z9hG4bK148bd9b2bbb45a91000857604129169f.1;rport\r\n"
"Call-ID: e9a2b8954041f803@192.168.62.43\r\n"
"CSeq: 3901 REGISTER\r\n"
"Contact: <sip:192.168.62.43:5060;transport=tcp>;+sip.instance=\"<urn:uuid : 02ec0634 - 751b - 5259 - b1aa - 156a7ca2b6c6>\"\r\n"
"From: <sip:sip>;tag=6f60a23e7652b642\r\n"
"To: <sip:sip>\r\n"
"Max-Forwards: 70\r\n"
"Route: <sip:192.168.61.91;lr>\r\n"
"Allow: INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY\r\n"
"User-Agent: TANDBERG/257 (TENC4.1.6.315255)\r\n"
"Expires: 3600\r\n"
"Supported: replaces,100rel,timer,gruu,path,outbound\r\n"
"Content-Length: 0\r\n\r\n"
;

char raw_vsx7000_ringing[] =
"SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP 192.168.41.140:52922;branch=__branch__\r\n\
From: b <sip:b@brchk000.trueconf.ua> ;tag=C4285400FEB840D2CDC4BFBA87906F5B\r\n\
To:  <sip:vsx7000@192.168.62.41> ;tag=plcm_100717000-17925415\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: vsx7000 <sip:vsx7000@192.168.62.41:5060;transport=tcp> \r\n\
User-Agent: Polycom VSX 7000 (HF 8.5.3.2b - 22Aug2007 16.10)\r\n\
Supported: ms - forking\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_vsx7000_ok[] =
"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP 192.168.41.140:52922;branch=__branch__\r\n\
From: b <sip:b@brchk000.trueconf.ua> ;tag=C4285400FEB840D2CDC4BFBA87906F5B\r\n\
To:  <sip:vsx7000@192.168.62.41> ;tag=plcm_100717000-17925415\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: vsx7000 <sip:vsx7000@192.168.62.41:5060;transport=tcp> \r\n\
Allow: INVITE,BYE,CANCEL,ACK,INFO,PRACK,COMET,OPTIONS,SUBSCRIBE,NOTIFY,REFER,REGISTER,UPDATE\r\n\
User-Agent: Polycom VSX 7000 (HF 8.5.3.2b - 22Aug2007 16.10)\r\n\
Supported: ms-forking\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 989\r\n\
\r\n\
v=0\r\n\
o=TrueConf 459656167 0 IN IP4 192.168.62.41\r\n\
s=-\r\n\
c=IN IP4 192.168.62.41\r\n\
b=AS:1536\r\n\
t=0 0\r\n\
m=audio 49154 RTP/AVP 114 115 113 102 101 9 15 0 8 18 99 98 97 103\r\n\
a=rtpmap:114 G7221/32000\r\n\
a=fmtp:114 bitrate=32000\r\n\
a=rtpmap:115 G7221/32000\r\n\
a=fmtp:115 bitrate=48000\r\n\
a=rtpmap:113 G7221/32000\r\n\
a=fmtp:113 bitrate=24000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=32000\r\n\
a=rtpmap:101 G7221/16000\r\n\
a=fmtp:101 bitrate=24000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:18 G729/8000\r\n\
a=fmtp:18 annexb=no\r\n\
a=rtpmap:99 SIREN14/16000\r\n\
a=fmtp:99 bitrate=48000\r\n\
a=rtpmap:98 SIREN14/16000\r\n\
a=fmtp:98 bitrate=32000\r\n\
a=rtpmap:97 SIREN14/16000\r\n\
a=fmtp:97 bitrate=24000\r\n\
a=rtpmap:103 G7221/16000\r\n\
a=fmtp:103 bitrate=16000\r\n\
a=sendrecv\r\n\
m=video 49156 RTP/AVP 34 96 31\r\n\
b=TIAS:1536000\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=2;CIF=1\r\n\
a=rtpmap:96 H263-1998/90000\r\n\
a=fmtp:96 CIF4=2;CIF=1\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=1;QCIF=1;D\r\n\
a=sendrecv\r\n";

char raw_hdx8000_ringing[] =
"SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP 192.168.41.140:51306;branch=__branch__\r\n\
From: \"b\" <sip:b@brchk000.trueconf.ua> ;tag=0AD9F983BC4EB02E40C5E6303B1995E4\r\n\
To:  <sip:hdx8000@192.168.62.42> ;tag=plcm_2275858000-1034947982\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: hdx8000 <sip:hdx8000@192.168.62.42:5060;transport=tcp> ;proxy=replace\r\n\
User-Agent: Polycom HDX 8000 HD (Release - 2.5.0.8_ne-4035)\r\n\
P_Preferred_Identity: \u041f\u0410\u041b\u0418\u041a\u041e\u041c\r\n\
Supported: ms-forking\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_hdx8000_ok[] =
"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP 192.168.41.140:51306;branch=__branch__\r\n\
From: \"b\" <sip:b@brchk000.trueconf.ua> ;tag=0AD9F983BC4EB02E40C5E6303B1995E4\r\n\
To:  <sip:hdx8000@192.168.62.42> ;tag=plcm_2275858000-1034947982\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: hdx8000 <sip:hdx8000@192.168.62.42:5060;transport=tcp> ;proxy=replace\r\n\
Allow: INVITE,BYE,CANCEL,ACK,INFO,PRACK,COMET,OPTIONS,SUBSCRIBE,NOTIFY,REFER,REGISTER,UPDATE\r\n\
User-Agent: Polycom HDX 8000 HD (Release - 2.5.0.8_ne-4035)\r\n\
P_Preferred_Identity: \u041f\u0410\u041b\u0418\u041a\u041e\u041c\r\n\
Supported: ms-forking\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 768\r\n\
\r\n\
v=0\r\n\
o=hdx8000 418448953 0 IN IP4 192.168.62.42\r\n\
s=-\r\n\
c=IN IP4 192.168.62.42\r\n\
b=AS:1536\r\n\
t=0 0\r\n\
m=audio 3230 RTP/AVP 115 102 9 15 0 8 18\r\n\
a=rtpmap:115 G7221/32000\r\n\
a=fmtp:115 bitrate=48000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=32000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:18 G729/8000\r\n\
a=fmtp:18 annexb=no\r\n\
a=sendrecv\r\n\
m=video 3232 RTP/AVP 109 96 34 31\r\n\
b=TIAS:1536000\r\n\
a=rtpmap:109 H264/90000\r\n\
a=fmtp:109 profile-level-id=42800d; max-mbps=108000; max-fs=3600; max-br=1600; sar=13\r\n\
a=rtpmap:96 H263-1998/90000\r\n\
a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=1;QCIF=1\r\n\
a=sendrecv\r\n\
a=rtcp-fb:* ccm fir tmmbr\r\n";

char raw_hd9030_trying[] =
"SIP/2.0 100 Trying\r\n\
Via: SIP/2.0/UDP 192.168.41.140:5060;branch=__branch__\r\n\
Call-ID: __callid__\r\n\
From: \"b\"<sip:b@brchk000.trueconf.ua>;tag=A9DECAD0529778E25409C1E2BA50EC33\r\n\
To: <sip:192.168.62.46>\r\n\
CSeq: 1 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_hd9030_ringing[] =
"SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/UDP 192.168.41.140:5060;branch=__branch__\r\n\
Call-ID: __callid__\r\n\
From: \"b\"<sip:b@brchk000.trueconf.ua>;tag=A9DECAD0529778E25409C1E2BA50EC33\r\n\
To: <sip:192.168.62.46>;tag=gglecp45\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:192.168.62.46:5060>\r\n\
Allow: INVITE,ACK,BYE,CANCEL,UPDATE,INFO,PRACK\r\n\
User-Agent: Huawei ViewPoint9000\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_hd9030_ok[] =
"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP 192.168.41.140:5060;branch=__branch__\r\n\
Call-ID: __callid__\r\n\
From: \"b\"<sip:b@brchk000.trueconf.ua>;tag=A9DECAD0529778E25409C1E2BA50EC33\r\n\
To: <sip:192.168.62.46>;tag=gglecp45\r\n\
CSeq: 1 INVITE\r\n\
Contact: \"kt\"<sip:192.168.62.46:5060>\r\n\
Allow: INVITE,ACK,BYE,CANCEL,UPDATE,INFO,PRACK\r\n\
User-Agent: Huawei ViewPoint9000\r\n\
Content-Length: 470\r\n\
Content-Type: application/sdp\r\n\
\r\n\
v=0\r\n\
o=huawei 1 0 IN IP4 192.168.62.46\r\n\
s=-\r\n\
c=IN IP4 192.168.62.46\r\n\
b=CT:1536\r\n\
t=0 0\r\n\
m=audio 10002 RTP/AVP 9 15 0 8\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=sendrecv\r\n\
m=video 10004 RTP/AVP 100 34\r\n\
b=AS:1536\r\n\
a=rtpmap:100 H264/90000\r\n\
a=fmtp:100 profile-level-id=42801F;max-br=1536;max-mbps=108000;max-fs=3840\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=1 CIF=1\r\n\
a=rtcp-fb:* ccm fir\r\n\
a=sendrecv\r\n\
a=content:main\r\n\
a=label:0\r\n";

char raw_vc400_trying[] =
"SIP/2.0 100 Trying\r\n\
Via: SIP/2.0/TCP 192.168.62.79:23747;branch=__branch__\r\n\
From: \"a\" <sip:a@matvey.trueconf.loc>;tag=E10CDA670EE11E71897591A07360EFB8\r\n\
To: <sip:192.168.62.48>\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
User-Agent: Yealink VC400 30.10.0.27\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_vc400_ringing[] =
"SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP 192.168.62.79:23747;branch=__branch__\r\n\
From: \"a\" <sip:a@matvey.trueconf.loc>;tag=E10CDA670EE11E71897591A07360EFB8\r\n\
To: <sip:192.168.62.48>;tag=2098396384\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:192.168.62.48:5060;transport=TCP>\r\n\
User-Agent: Yealink VC400 30.10.0.27\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_vc400_ok[] =
"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP 192.168.62.79:23747;branch=__branch__\r\n\
From: \"a\" <sip:a@matvey.trueconf.loc>;tag=E10CDA670EE11E71897591A07360EFB8\r\n\
To: <sip:192.168.62.48>;tag=2098396384\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:192.168.62.48:5060;transport=TCP>\r\n\
Content-Type: application/sdp\r\n\
User-Agent: Yealink VC400 30.10.0.27\r\n\
Content-Length:  1196\r\n\
\r\n\
v=0\r\n\
o=- 20006 20006 IN IP4 192.168.62.48\r\n\
s=SDP data\r\n\
c=IN IP4 192.168.62.48\r\n\
b=AS:1536\r\n\
t=0 0\r\n\
m=audio 50060 RTP/AVP 114 115 113 103 9 0 8\r\n\
a=rtpmap:115 G7221/32000\r\n\
a=fmtp:115 bitrate=48000\r\n\
a=rtpmap:114 G7221/32000\r\n\
a=fmtp:114 bitrate=32000\r\n\
a=rtpmap:113 G7221/32000\r\n\
a=fmtp:113 bitrate=24000\r\n\
a=rtpmap:103 G7221/16000\r\n\
a=fmtp:103 bitrate=24000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=ptime:20\r\n\
a=sendrecv\r\n\
m=video 50062 RTP/AVP 100 34\r\n\
b=TIAS:1536000\r\n\
a=rtpmap:100 H264/90000\r\n\
a=fmtp:100 profile-level-id=428020; max-mbps=244800; max-fs=8160\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=1; CIF=1; QCIF=1;\r\n\
a=ptime:20\r\n\
a=rtcp-fb:* ccm fir\r\n\
a=sendrecv\r\n\
m=application 51019 TCP/BFCP *\r\n\
a=floorctrl:c-only\r\n\
a=setup:active\r\n\
a=connection:new\r\n\
a=confid:1\r\n\
a=userid:2\r\n\
a=floorid:1 m-stream:3\r\n\
m=video 0 RTP/AVP 100 102 101 34 31\r\n\
c=IN IP4 192.168.62.79\r\n\
b=TIAS:1572864\r\n\
a=content:slides\r\n\
a=label:3\r\n\
a=rtpmap:100 H264/90000\r\n\
a=fmtp:100 profile-level-id=42001F\r\n\
a=rtpmap:102 H263-2000/90000\r\n\
a=fmtp:102 CIF4=1;CIF=1\r\n\
a=rtpmap:101 H263-1998/90000\r\n\
a=fmtp:101 CIF4=1;CIF=1\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=1;CIF=1\r\n\
a=rtpmap:31 H261/90000\r\n\
a=sendrecv\r\n\
a=rtcp-fb:* ccm fir\r\n";

char raw_aver_evc_130_trying[] =
"SIP/2.0 100 Trying\r\n\
Via: SIP/2.0/UDP 192.168.62.79:5060;branch=__branch__\r\n\
From: \"\xd0\x94\xd0\xb8\xd1\x81\xd0\xbf\xd0\xbb\xd0\xb5\xd0\xb9\x2e\x20\xd0\x9d\xd0\xb5\xd0\xb9\xd0\xbc\x2e\x20\xd1\x83\x2e\x20\xd0\x90\x2e\" <sip:a@matvey.trueconf.loc>;tag=600ADAD95520389F6740B2D24EE65946\r\n\
To: <sip:192.168.62.89>\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
User-Agent: AVer, EVC130 00.01.12.81\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_aver_evc_130_ringing[] =
"SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/UDP 192.168.62.79:5060;branch=__branch__\r\n\
From: \"\xd0\x94\xd0\xb8\xd1\x81\xd0\xbf\xd0\xbb\xd0\xb5\xd0\xb9\x2e\x20\xd0\x9d\xd0\xb5\xd0\xb9\xd0\xbc\x2e\x20\xd1\x83\x2e\x20\xd0\x90\x2e\" <sip:a@matvey.trueconf.loc>;tag=600ADAD95520389F6740B2D24EE65946\r\n\
To: <sip:192.168.62.89>;tag=548786168\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: AVer <sip:192.168.62.89:5060>\r\n\
User-Agent: AVer, EVC130 00.01.12.81\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_aver_evc_130_ok[] =
"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP 192.168.62.79:5060;branch=__branch__\r\n\
From: \"\xd0\x94\xd0\xb8\xd1\x81\xd0\xbf\xd0\xbb\xd0\xb5\xd0\xb9\x2e\x20\xd0\x9d\xd0\xb5\xd0\xb9\xd0\xbc\x2e\x20\xd1\x83\x2e\x20\xd0\x90\x2e\" <sip:a@matvey.trueconf.loc>;tag=600ADAD95520389F6740B2D24EE65946\r\n\
To: <sip:192.168.62.89>;tag=548786168\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: AVer <sip:192.168.62.89:5060>\r\n\
Content-Type: application/sdp\r\n\
User-Agent: AVer, EVC130 00.01.12.81\r\n\
Content-Length:  1063\r\n\
\r\n\
v=0\r\n\
o=Aver 0 0 IN IP4 192.168.62.89\r\n\
s=-\r\n\
c=IN IP4 192.168.62.89\r\n\
b=AS:1572\r\n\
t=0 0\r\n\
m=audio 30000 RTP/AVP 9\r\n\
a=rtpmap:9 G722/8000\r\n\
a=sendrecv\r\n\
m=video 30002 RTP/AVP 100\r\n\
b=TIAS:1572000\r\n\
a=content:main\r\n\
a=label:1\r\n\
a=rtpmap:100 H264/90000\r\n\
a=fmtp:100 profile-level-id=428028;max-mbps=244800;max-fs=8160;max-br=1572;packetization-mode=0\r\n\
a=sendrecv\r\n\
m=application 30010 TCP/BFCP *\r\n\
a=floorctrl:c-only\r\n\
a=setup:passive\r\n\
a=connection:new\r\n\
a=floorid:1 m-stream:3\r\n\
a=confid:1\r\n\
a=userid:2\r\n\
m=video 30004 RTP/AVP 100\r\n\
b=TIAS:1572000\r\n\
a=content:slides\r\n\
a=label:3\r\n\
a=rtpmap:100 H264/90000\r\n\
a=fmtp:100 profile-level-id=42800d;max-mbps=108000;max-fs=3600;max-br=1572;packetization-mode=0\r\n\
a=rtpmap:109 AVer-H264SVC-1/90000\r\n\
a=fmtp:109 profile-level-id=42800d;max-mbps=108000;max-fs=3600;max-br=1572;packetization-mode=0\r\n\
a=rtpmap:109 AVer-H264SVC-0/90000\r\n\
a=fmtp:109 profile-level-id=42800d;max-mbps=108000;max-fs=3600;max-br=1572;packetization-mode=0\r\n\
a=rtpmap:108 H264/90000\r\n\
a=fmtp:108 profile-level-id=64000d;max-mbps=108000;max-fs=3600;max-br=1572;packetization-mode=0\r\n";

char raw_sony_xg77_trying[] =
"SIP/2.0 100 Trying\r\n\
Via: SIP/2.0/TCP 192.168.62.79:5060;branch=__branch__\r\n\
To: <sip:xg77@192.168.62.153:5060;transport=tcp>\r\n\
From: \"a\"<sip:a@matvey.trueconf.loc>;tag=0494A128B0EADC1543F06FD2B13D17EF\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Authorization: Digest username=\"xg77\",realm=\"trueconf\",nonce=\"e3c8525b726955df2e95497edc1c63b4\",uri=\"sip:matvey.trueconf.loc\",algorithm=MD5,response=\"642741c3f0ace1ad2b098b4dfd1c659a\"\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_sony_xg77_ringing[] =
"SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP 192.168.62.79:5060;branch=__branch__\r\n\
To: <sip:xg77@192.168.62.153:5060;transport=tcp>;tag=c082f568\r\n\
From: \"a\"<sip:a@matvey.trueconf.loc>;tag=0494A128B0EADC1543F06FD2B13D17EF\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:xg77@192.168.62.153:5060;transport=tcp>\r\n\
Allow: ACK,BYE,CANCEL,INVITE,UPDATE,INFO,OPTIONS\r\n\
Authorization: Digest username=\"xg77\",realm=\"trueconf\",nonce=\"e3c8525b726955df2e95497edc1c63b4\",uri=\"sip:matvey.trueconf.loc\",algorithm=MD5,response=\"642741c3f0ace1ad2b098b4dfd1c659a\"\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_sony_xg77_ok[] =
"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP 192.168.62.79:5060;branch=__branch__\r\n\
To: <sip:xg77@192.168.62.153:5060;transport=tcp>;tag=c082f568\r\n\
From: \"a\"<sip:a@matvey.trueconf.loc>;tag=0494A128B0EADC1543F06FD2B13D17EF\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:xg77@192.168.62.153:5060;transport=tcp>\r\n\
Allow: ACK,BYE,CANCEL,INVITE,UPDATE,INFO,OPTIONS\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 421\r\n\
Authorization: Digest username=\"xg77\",realm=\"trueconf\",nonce=\"e3c8525b726955df2e95497edc1c63b4\",uri=\"sip:matvey.trueconf.loc\",algorithm=MD5,response=\"642741c3f0ace1ad2b098b4dfd1c659a\"\r\n\
\r\n\
v=0\r\n\
o=- 3159214526 3159214526 IN IP4 192.168.62.153\r\n\
s=-\r\n\
c=IN IP4 192.168.62.153\r\n\
b=CT:1536\r\n\
t=0 0\r\n\
m=audio 49152 RTP/AVP 9 15 8 0\r\n\
b=AS:64\r\n\
a=sendrecv\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
m=video 49154 RTP/AVP 100 34\r\n\
b=AS:1572\r\n\
a=sendrecv\r\n\
a=rtpmap:100 H264/90000\r\n\
a=fmtp:100 profile-level-id=42c01f\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF=1;QCIF=1;SQCIF=1;CIF4=1\r\n";

char raw_e20_ringing[] =
"SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP 192.168.41.140:54357;branch=__branch__;received=192.168.41.140\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
From: \"b\" <sip:b@brchk000.trueconf.ua>;tag=4D7EFC6DA94A48D9E5D5D592094BB369\r\n\
To: <sip:192.168.62.43>;tag=908a436d0d0d4103\r\n\
Server: TANDBERG/257 (TENC4.1.6.315255)\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_e20_ok[] =
"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP 192.168.41.140:54357;branch=__branch__;received=192.168.41.140\r\n\
Call-ID: __callid__\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:192.168.62.43:5060;transport=tcp>\r\n\
From: \"b\" <sip:b@brchk000.trueconf.ua>;tag=4D7EFC6DA94A48D9E5D5D592094BB369\r\n\
To: <sip:192.168.62.43>;tag=908a436d0d0d4103\r\n\
Allow: INVITE,ACK,CANCEL,BYE,UPDATE,INFO,OPTIONS,REFER,NOTIFY\r\n\
Server: TANDBERG/257 (TENC4.1.6.315255)\r\n\
Supported: replaces,100rel,timer,gruu,path,outbound\r\n\
Session-Expires: 1800;refresher=uas\r\n\
Min-SE: 90\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 885\r\n\
\r\n\
v=0\r\n\
o=tandberg 3 1 IN IP4 192.168.62.43\r\n\
s=-\r\n\
c=IN IP4 192.168.62.43\r\n\
b=AS:1152\r\n\
t=0 0\r\n\
m=audio 1032 RTP/AVP 104 103 9 0 8\r\n\
b=TIAS:64000\r\n\
a=rtpmap:104 G7221/16000\r\n\
a=fmtp:104 bitrate=32000\r\n\
a=rtpmap:103 G7221/16000\r\n\
a=fmtp:103 bitrate=24000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=sendrecv\r\n\
m=video 1034 RTP/AVP 100 101 34 31\r\n\
b=TIAS:1152000\r\n\
a=rtpmap:100 H264/90000\r\n\
a=fmtp:100 profile-level-id=42800d;max-br=906;max-mbps=40500;max-fs=1344;max-smbps=40500;max-fps=3000\r\n\
a=rtpmap:101 H263-1998/90000\r\n\
a=fmtp:101 custom=1024,768,4;custom=1024,576,4;custom=800,600,4;cif4=2;custom=720,480,2;custom=640,480,2;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;maxbr=10880\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 cif4=2;cif=1;qcif=1;maxbr=10880\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 cif=1;qcif=1;maxbr=10880\r\n\
a=rtcp-fb:* nack pli\r\n\
a=sendrecv\r\n\
a=content:main\r\n\
a=label:11\r\n";

char raw_mc850_trying[] =
"SIP/2.0 100 Trying\r\n\
CSeq: 1 INVITE\r\n\
Call-ID: __callid__\r\n\
Content-Length: 0\r\n\
From: \"b\" <sip:b@brchk000.trueconf.ua>;tag=677F80C1108734C399C3BD28049D90C0\r\n\
To: <sip:192.168.62.44>\r\n\
Via: SIP/2.0/UDP 192.168.41.140:5060;branch=__branch__\r\n\
\r\n";

char raw_mc850_ringing[] =
"SIP/2.0 180 Ringing\r\n\
CSeq: 1 INVITE\r\n\
Call-ID: __callid__\r\n\
Contact: <sip:sip@192.168.62.44>\r\n\
Content-Length: 0\r\n\
From: \"b\" <sip:b@brchk000.trueconf.ua>;tag=677F80C1108734C399C3BD28049D90C0\r\n\
To: <sip:192.168.62.44>;tag=084ee129\r\n\
Via: SIP/2.0/UDP 192.168.41.140:5060;branch=__branch__\r\n\
User-Agent: Huawei-MC850/V100R001C02B120\r\n\
Supported: 100rel\r\n\
\r\n";

char raw_mc850_ok[] =
"SIP/2.0 200 OK\r\n\
CSeq: 1 INVITE\r\n\
Call-ID: __callid__\r\n\
Contact: <sip:sip@192.168.62.44>\r\n\
Content-Length: 242\r\n\
From: \"b\" <sip:b@brchk000.trueconf.ua>;tag=677F80C1108734C399C3BD28049D90C0\r\n\
To: <sip:192.168.62.44>;tag=084ee129\r\n\
Via: SIP/2.0/UDP 192.168.41.140:5060;branch=__branch__\r\n\
User-Agent: Huawei-MC850/V100R001C02B120\r\n\
Supported: 100rel\r\n\
Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,REGISTER,PRACK,UPDATE\r\n\
Content-Type: application/sdp\r\n\
\r\n\
v=0\r\n\
o=Huawei 17908027 17908027 IN IP4 192.168.62.44\r\n\
s=Sip Call\r\n\
c=IN IP4 192.168.62.44\r\n\
t=0 0\r\n\
m=audio 0 RTP/AVP 114\r\n\
m=video 10008 RTP/AVP 100\r\n\
b=AS:704\r\n\
a=rtpmap:100 H264/90000\r\n\
a=fmtp:100 profile-level-id=420816; max-br=704\r\n\
a=sendrecv\r\n";

char raw_hd9030_invite[] =
"INVITE sip:b@192.168.41.140 SIP/2.0\r\n\
Via: SIP/2.0/UDP 192.168.62.46:5060;branch=z9hG4bKjq00dgej5ld04l5jhje0fq2f2\r\n\
Call-ID: baegfhfpae4qfeglafb4ahhqc0pb00ap@127.0.0.1\r\n\
From: <sip:192.168.62.46>;tag=4pla0lcc\r\n\
To: <sip:b@192.168.41.140>\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:192.168.62.46:5060>\r\n\
Max-Forwards: 70\r\n\
User-Agent: Huawei ViewPoint9000\r\n\
Allow: INVITE,ACK,BYE,CANCEL,UPDATE,INFO,PRACK\r\n\
Content-Length: 887\r\n\
Content-Type: application/sdp\r\n\
\r\n\
v=0\r\n\
o=huawei 1 0 IN IP4 192.168.62.46\r\n\
s=-\r\n\
c=IN IP4 192.168.62.46\r\n\
b=CT:1920\r\n\
t=0 0\r\n\
m=audio 10002 RTP/AVP 98 9 15 8 0 97\r\n\
a=rtpmap:98 MP4A-LATM/90000\r\n\
a=fmtp:98 bitrate=256000;profile-level-id=25;object=23\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:97 telephone-event/8000\r\n\
a=fmtp:97 0-15\r\n\
a=sendrecv\r\n\
m=video 10004 RTP/AVP 105 34 31\r\n\
b=AS:1920\r\n\
a=rtpmap:105 H264/90000\r\n\
a=fmtp:105 profile-level-id=42801F;max-br=1920;max-mbps=115000;max-fs=3840\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF=1 XMAX=88 YMAX=60 MPI=1 CIF4=1 XMAX=176 YMAX=120 MPI=1 XMAX=160 YMAX=120 MPI=6 QCIF=1 SQCIF=1 XMAX=200 YMAX=150 MPI=6 XMAX=256 YMAX=192 MPI=6\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=1 QCIF=1\r\n\
a=rtcp-fb:* ccm fir\r\n\
a=rtcp-fb:* ccm tmmbr\r\n\
a=sendrecv\r\n\
a=content:main\r\n\
a=label:11\r\n\
m=application 7684 RTP/AVP 100\r\n\
a=rtpmap:100 H224/4800\r\n\
a=sendrecv\r\n";

char raw_hd9030_ack[] =
"ACK sip:b%40192.168.41.140@192.168.41.140:5060;transport=udp SIP/2.0\r\n\
Via: SIP/2.0/UDP 192.168.62.46:5060;branch=z9hG4bKbpq0dgejcpapbjlp0dcjqep50\r\n\
Call-ID: baegfhfpae4qfeglafb4ahhqc0pb00ap@127.0.0.1\r\n\
From: <sip:192.168.62.46>;tag=4pla0lcc\r\n\
To: <sip:b@192.168.41.140>;tag=1AEE2E9A6C300397E1553653238BE0AA\r\n\
CSeq: 1 ACK\r\n\
Max-Forwards: 70\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_vc400_invite[] =
"INVITE sip:a@192.168.62.79 SIP/2.0\r\n\
Via: SIP/2.0/TCP 192.168.62.48:5060;branch=z9hG4bK1308202258\r\n\
From: \"RRC-VC400 (1332)\" <sip:192.168.62.48>;tag=3015585994\r\n\
To: <sip:a@192.168.62.79>\r\n\
Call-ID: 0_3884661032@192.168.62.48\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:192.168.62.48:5060>\r\n\
Content-Type: application/sdp\r\n\
Max-Forwards: 70\r\n\
User-Agent: Yealink VC400 30.10.0.27\r\n\
Supported: replaces\r\n\
Content-Length:  1132\r\n\
\r\n\
v=0\r\n\
o=- 20010 20010 IN IP4 192.168.62.48\r\n\
s=SDP data\r\n\
c=IN IP4 192.168.62.48\r\n\
b=AS:2048\r\n\
t=0 0\r\n\
m=audio 50100 RTP/AVP 123 122 121 102 9 8 0\r\n\
a=rtpmap:123 G7221/32000\r\n\
a=fmtp:123 bitrate=48000\r\n\
a=rtpmap:122 G7221/32000\r\n\
a=fmtp:122 bitrate=32000\r\n\
a=rtpmap:121 G7221/32000\r\n\
a=fmtp:121 bitrate=24000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=24000\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=ptime:20\r\n\
a=sendrecv\r\n\
m=video 50102 RTP/AVP 98 99 97 34 117\r\n\
b=TIAS:2048000\r\n\
a=rtpmap:98 H264/90000\r\n\
a=fmtp:98 profile-level-id=428028; max-mbps=245760; max-fs=8192\r\n\
a=rtpmap:99 H264/90000\r\n\
a=fmtp:99 profile-level-id=428028; max-mbps=245760; max-fs=8192; packetization-mode=1\r\n\
a=rtpmap:97 H264/90000\r\n\
a=fmtp:97 profile-level-id=640028; max-mbps=245760; max-fs=8192; packetization-mode=1\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF4=1; CIF=1; QCIF=1;\r\n\
a=rtpmap:117 YL-FPR/90000\r\n\
a=fmtp:117 yl-capset=7;yl-ver=1;yl-ext=19\r\n\
a=ptime:20\r\n\
a=rtcp-fb:* ccm fir\r\n\
a=sendrecv\r\n\
m=application 50108 RTP/AVP 100\r\n\
a=rtpmap:100 H224/4800\r\n\
a=sendrecv\r\n\
m=application 50104 UDP/BFCP *\r\n\
a=floorctrl:c-s\r\n\
a=setup:actpass\r\n\
a=connection:new\r\n";

char raw_vc400_ack[] =
"ACK sip:a%40192.168.62.79@192.168.62.79:5060;transport=tcp SIP/2.0\r\n\
Via: SIP/2.0/TCP 192.168.62.48:5060;branch=z9hG4bK1861951092\r\n\
From: \"RRC-VC400 (1332)\" <sip:192.168.62.48>;tag=3015585994\r\n\
To: <sip:a@192.168.62.79>;tag=4706530087241B2A185275C614CD57AF\r\n\
Call-ID: 0_3884661032@192.168.62.48\r\n\
CSeq: 1 ACK\r\n\
Contact: <sip:192.168.62.48:5060>\r\n\
Max-Forwards: 70\r\n\
User-Agent: Yealink VC400 30.10.0.27\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_aver_evc_130_invite[] =
"INVITE sip:a@192.168.62.79 SIP/2.0\r\n\
Via: SIP/2.0/UDP 192.168.62.89:5060;rport;branch=z9hG4bK234138762\r\n\
From: <sip:AVer@192.168.62.89>;tag=1156946410\r\n\
To: <sip:a@192.168.62.79>\r\n\
Call-ID: 1230108781\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:AVer@192.168.62.89:5060>\r\n\
Content-Type: application/sdp\r\n\
Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO\r\n\
Max-Forwards: 70\r\n\
User-Agent: AVer, EVC130 00.01.12.81\r\n\
Content-Length:  2113\r\n\
\r\n\
v=0\r\n\
o=Aver 0 0 IN IP4 192.168.62.89\r\n\
s=-\r\n\
c=IN IP4 192.168.62.89\r\n\
b=AS:2048\r\n\
t=0 0\r\n\
m=audio 30000 RTP/AVP 9 0 8 15 101 102 103 104 105 97\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:101 G7221/16000\r\n\
a=fmtp:101 bitrate=24000\r\n\
a=rtpmap:102 G7221/16000\r\n\
a=fmtp:102 bitrate=32000\r\n\
a=rtpmap:103 G7221/32000\r\n\
a=fmtp:103 bitrate=24000\r\n\
a=rtpmap:104 G7221/32000\r\n\
a=fmtp:104 bitrate=32000\r\n\
a=rtpmap:105 G7221/32000\r\n\
a=fmtp:105 bitrate=48000\r\n\
a=rtpmap:97 telephone-event/8000\r\n\
a=fmtp:97 0-11\r\n\
a=sendrecv\r\n\
m=video 30002 RTP/AVP 109 108 107 106 34 31\r\n\
b=TIAS:2048000\r\n\
a=content:main\r\n\
a=label:1\r\n\
a=rtpmap:109 AVer-H264SVC-1/90000\r\n\
a=fmtp:109 profile-level-id=428028;max-mbps=244800;max-fs=8160;max-br=2048;packetization-mode=0\r\n\
a=rtpmap:109 AVer-H264SVC-0/90000\r\n\
a=fmtp:109 profile-level-id=428028;max-mbps=244800;max-fs=8160;max-br=2048;packetization-mode=0\r\n\
a=rtpmap:108 H264/90000\r\n\
a=fmtp:108 profile-level-id=640028;max-mbps=244800;max-fs=8160;max-br=2048;packetization-mode=0\r\n\
a=rtpmap:107 H264/90000\r\n\
a=fmtp:107 profile-level-id=428028;max-mbps=244800;max-fs=8160;max-br=2048;packetization-mode=0\r\n\
a=rtpmap:106 H263-1998/90000\r\n\
a=fmtp:106 CIF=1;QCIF=1\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF=1;QCIF=1\r\n\
a=rtpmap:31 H261/90000\r\n\
a=fmtp:31 CIF=2;QCIF=2\r\n\
a=sendrecv\r\n\
m=application 30006 RTP/AVP 100\r\n\
a=rtpmap:100 H224/4800\r\n\
a=sendrecv\r\n\
m=application 30010 UDP/BFCP *\r\n\
a=floorctrl:c-s\r\n\
a=setup:passive\r\n\
a=connection:new\r\n\
a=floorid:1 m-stream:3\r\n\
a=confid:155\r\n\
a=userid:2\r\n\
m=video 30004 RTP/AVP 109 108 107\r\n\
b=TIAS:2048000\r\n\
a=content:slides\r\n\
a=label:3\r\n\
a=rtpmap:109 AVer-H264SVC-1/90000\r\n\
a=fmtp:109 profile-level-id=42800d;max-mbps=108000;max-fs=3600;max-br=2048;packetization-mode=0\r\n\
a=rtpmap:109 AVer-H264SVC-0/90000\r\n\
a=fmtp:109 profile-level-id=42800d;max-mbps=108000;max-fs=3600;max-br=2048;packetization-mode=0\r\n\
a=rtpmap:108 H264/90000\r\n\
a=fmtp:108 profile-level-id=64000d;max-mbps=108000;max-fs=3600;max-br=2048;packetization-mode=0\r\n\
a=rtpmap:107 H264/90000\r\n\
a=fmtp:107 profile-level-id=42800d;max-mbps=108000;max-fs=3600;max-br=2048;packetization-mode=0\r\n\
a=sendrecv\r\n";

char raw_aver_evc_130_ack[] =
"ACK sip:a%40192.168.62.79@192.168.62.79:5060;transport=udp SIP/2.0\r\n\
Via: SIP/2.0/UDP 192.168.62.89:5060;rport;branch=z9hG4bK544687684\r\n\
From: <sip:AVer@192.168.62.89>;tag=1156946410\r\n\
To: <sip:a@192.168.62.79>;tag=DD312524AFA4E301BAEFD666745D6311\r\n\
Call-ID: 1230108781\r\n\
CSeq: 1 ACK\r\n\
Contact: <sip:AVer@192.168.62.89:5060>\r\n\
Max-Forwards: 70\r\n\
User-Agent: AVer, EVC130 00.01.12.81\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_sony_xg77_invite[] =
"INVITE sip:a@matvey.trueconf.loc SIP/2.0\r\n\
Via: SIP/2.0/TCP 192.168.62.153:5060;branch=z9hG4bKcc4caf86\r\n\
Max-Forwards: 70\r\n\
To: <sip:a@matvey.trueconf.loc>\r\n\
From: <sip:xg77@matvey.trueconf.loc>;tag=cc4c6896\r\n\
Call-ID: cc4c785e-e014-11d3-8141-421b47e33a7f\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:xg77@192.168.62.153:5060;transport=tcp>\r\n\
Allow: ACK,BYE,CANCEL,INVITE,UPDATE,INFO,OPTIONS\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 675\r\n\
Authorization: Digest username=\"xg77\",realm=\"trueconf\",nonce=\"e3c8525b726955df2e95497edc1c63b4\",uri=\"sip:matvey.trueconf.loc\",algorithm=MD5,response=\"2b429bcd192f341faa844538bb78b8dd\"\r\n\
\r\n\
v=0\r\n\
o=- 3159215405 3159215405 IN IP4 192.168.62.153\r\n\
s=-\r\n\
c=IN IP4 192.168.62.153\r\n\
b=CT:2048\r\n\
t=0 0\r\n\
m=audio 49152 RTP/AVP 9 15 8 0\r\n\
b=AS:64\r\n\
a=sendrecv\r\n\
a=rtpmap:9 G722/8000\r\n\
a=rtpmap:15 G728/8000\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
m=video 49154 RTP/AVP 105 96 34\r\n\
b=AS:1984\r\n\
a=sendrecv\r\n\
a=rtpmap:105 H264/90000\r\n\
a=fmtp:105 profile-level-id=42c01f;max-mbps=216000;max-fs=3840;max-br=2048\r\n\
a=rtpmap:96 MP4V-ES/90000\r\n\
a=fmtp:96 profile-level-id=3;config=000001b003000001b509000001000000012000844007a8582120a21f\r\n\
a=rtpmap:34 H263/90000\r\n\
a=fmtp:34 CIF=1;QCIF=1;SQCIF=1;CIF4=1;D=1;F=1;J=1\r\n\
m=application 49156 RTP/AVP 100\r\n\
b=AS:64\r\n\
a=sendrecv\r\n\
a=rtpmap:100 H224/4800\r\n";

char raw_sony_xg77_ack[] =
"ACK sip:a%40matvey.trueconf.loc@192.168.62.79:5060;transport=tcp SIP/2.0\r\n\
Via: SIP/2.0/TCP 192.168.62.153:5060;branch=z9hG4bKcf938b7e\r\n\
Max-Forwards: 70\r\n\
To: <sip:a@matvey.trueconf.loc>;tag=81E20273CE28603C33B45BF233E61EF7\r\n\
From: <sip:xg77@matvey.trueconf.loc>;tag=cc4c6896\r\n\
Call-ID: cc4c785e-e014-11d3-8141-421b47e33a7f\r\n\
CSeq: 1 ACK\r\n\
Contact: <sip:xg77@192.168.62.153:5060;transport=tcp>\r\n\
Authorization: Digest username=\"xg77\",realm=\"trueconf\",nonce=\"e3c8525b726955df2e95497edc1c63b4\",uri=\"sip:matvey.trueconf.loc\",algorithm=MD5,response=\"67f43dc42dbd23607da0d07a5f9e3fff\"\r\n\
Content-Length: 0\r\n\
\r\n";

char raw_compact_header[] =
"INVITE sip:192.168.0.103 SIP/2.0\r\n"
"v: SIP/2.0/UDP 192.168.0.100:5060;rport;branch=z9hG4bKPjxaGHm7W.9fqkrTP.wF8lmS.kKCrVoRnD\r\n"
"Max-Forwards: 70\r\n"
"o: presence\r\n"
"f: <sip:192.168.0.100>;tag=ofmgUL-c4JP1y4RIkzHZ.xcskWlx9Btw\r\n"
"t: <sip:192.168.0.103>\r\n"
"m: <sip:192.168.0.100:5060;ob>\r\n"
"i: .RmpN7NE0I4NeKZzwrdgGE8v.34hrvyX\r\n"
"CSeq: 30150 INVITE\r\n"
"k: replaces, 100rel, timer, norefersub\r\n"
"Expires: 1800\r\n"
"Min-SE: 90\r\n"
"User-Agent: CSipSimple_mako-22/r2457\r\n"
"c: application/sdp\r\n"
"l:   597\r\n\r\n";

char raw_rockwareC9_options[] =
"OPTIONS sip:192.168.41.158 SIP/2.0\r\n"
"Via: SIP/2.0/UDP 192.168.62.97:5060;branch=z9hG4bK546ae721-c0fe-e511-8404-b4994cc870d6;rport\r\n"
"CSeq: 1 OPTIONS\r\n"
"Content-Length: 0\r\n"
"Call-ID: 746de721-c0fe-e511-8404-b4994cc870d6@localhost\r\n"
"To: <sip:192.168.41.158>\r\n"
"Max-Forwards: 70\r\n"
"From: <sip:192.168.41.158>\r\n\r\n";

char bug_33043_unauthorized[] =
"SIP/2.0 401 Unauthorized\r\n"
"Via: SIP/2.0/UDP 192.168.66.50:5060;branch=__branch__;received=109.254.39.82\r\n"
"From: <sip:201$vpbx471200081@188.187.220.27>;tag=__fromtag__\r\n"
"To: <sip:201$vpbx471200081@188.187.220.27>;tag=d462f0f62e1b11e6b61d2c768a52bacc\r\n"
"Call-ID: __callid__\r\n"
"CSeq: 1 REGISTER\r\n"
"WWW-Authenticate: Digest realm=\"SIP-REGISTRAR\", nonce=\"AD014D7D\"\r\n"
"Server: TS-v4.5.1-20g\r\n"
"Content-Length: 0\r\n\r\n";

char raw_ticket_en1393_200ok[] =
"SIP/2.0 200 OK\r\n"
"Via: SIP/2.0/TCP 148.81.43.125:51779;branch=__branch__;received=148.81.43.125;ingress-zone=DefaultZone\r\n"
"Call-ID: __callid__\r\n"
"CSeq: 1 INVITE\r\n"
"Remote-Party-ID: <sip:0048617003566@man.poznan.pl>;party=called;screen=no;privacy=off\r\n"
"Contact: <sip:0048617003566@10.100.128.19:5560;transport=tcp>;isfocus\r\n"
"From: \"test1\" <sip:test1@ifpan.eud.pl>;tag=2DB5612ACC33CBFEC9403B5F46BD2C3D\r\n"
"To: <sip:0048617003566@man.poznan.pl>;tag=19962626-dc42ffb0-f011-c601-ce64-b82c2a390902-44928255\r\n"
"Record-Route: <sip:proxy-call-id=9fba18e5-b614-4b7a-88de-af8fa2579823@10.100.128.23:5060;transport=tcp;lr>,<sip:proxy-call-id=9fba18e5-b614-4b7a-88de-af8fa2579823@10.100.128.23:5061;transport=tls;lr>,<sip:proxy-call-id=41d3cebc-31fe-407d-a2ee-77b68c0dfa57@10.100.128.41:7011;transport=tls;lr>,<sip:proxy-call-id=41d3cebc-31fe-407d-a2ee-77b68c0dfa57@150.254.210.29:5060;transport=tcp;lr>\r\n"
"Allow: INVITE,OPTIONS,INFO,BYE,CANCEL,ACK,PRACK,UPDATE,REFER,SUBSCRIBE,NOTIFY\r\n"
"Server: Cisco-CUCM10.5\r\n"
"Date: Mon, 10 Oct 2016 10:17:40 GMT\r\n"
"Supported: replaces\r\n"
"Supported: X-cisco-srtp-fallback\r\n"
"Supported: Geolocation\r\n"
"Require: timer\r\n"
"Session-Expires: 1800;refresher=uas\r\n"
"P-Preferred-Identity: <sip:0048617003566@man.poznan.pl>\r\n"
"Allow-Events: presence\r\n"
"Call-Info: <urn:x-cisco-remotecc:callinfo>;x-cisco-video-traffic-class=IMMERSIVE\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: 1002\r\n"
"\r\n"
"v=0\r\n"
"o=CiscoSystemsCCM-SIP 19962626 1 IN IP4 10.100.128.19\r\n"
"s=SIP Call\r\n"
"c=IN IP4 150.254.210.29\r\n"
"b=AS:1536\r\n"
"t=0 0\r\n"
"m=audio 41878 RTP/AVP 9 105 106 0 8 15 18 4\r\n"
"a=rtpmap:9 G722/8000\r\n"
"a=rtpmap:105 G7221/16000\r\n"
"a=fmtp:105 bitrate=32000\r\n"
"a=rtpmap:106 G7221/16000\r\n"
"a=fmtp:106 bitrate=24000\r\n"
"a=rtpmap:0 PCMU/8000\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=rtpmap:15 G728/8000\r\n"
"a=rtpmap:4 G723/8000\r\n"
"a=rtpmap:18 G729/8000\r\n"
"a=fmtp:18 annexb=no\r\n"
"a=trafficclass:conversational.audio.immersive.aq:admitted\r\n"
"a=rtcp:41879 IN IP4 150.254.210.29\r\n"
"m=video 41880 RTP/AVP 96 98 34 31\r\n"
"b=TIAS:3968000\r\n"
"a=label:11\r\n"
"a=rtpmap:96 H264/90000\r\n"
"a=fmtp:96 profile-level-id=420016;packetization-mode=0;max-mbps=108000;max-fs=3840;max-dpb=5408;max-fps=6000\r\n"
"a=rtpmap:98 H263-1998/90000\r\n"
"a=fmtp:98 QCIF=1;CIF=1;CIF4=1;CUSTOM=352,240,1\r\n"
"a=rtpmap:34 H263/90000\r\n"
"a=fmtp:34 QCIF=1;CIF=1;CIF4=1\r\n"
"a=rtpmap:31 H261/90000\r\n"
"a=fmtp:31 CIF=1;QCIF=1\r\n"
"a=content:main\r\n"
"a=trafficclass:conversational.video.immersive.aq:admitted\r\n"
"a=rtcp:41881 IN IP4 150.254.210.29\r\n";

const char raw_lync_unauthorized[] =
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 01 Dec 2016 08:21:44 GMT\r\n"
"WWW-Authenticate: NTLM realm=\"SIP Communications Service\", targetname=\"LYNC2013-SERVER.lync.loc\", version=4\r\n"
"WWW-Authenticate: Kerberos realm=\"SIP Communications Service\", targetname=\"sip/LYNC2013-SERVER.lync.loc\", version=4\r\n"
"WWW-Authenticate: TLS-DSK realm=\"SIP Communications Service\", targetname=\"LYNC2013-SERVER.lync.loc\", version=4, sts-uri=\"https://LYNC2013-SERVER.lync.loc:443/CertProv/CertProvisioningService.svc\"\r\n"
"From: <sip:test2@192.168.74.12>;tag=11B70D02F6AF81E4624E69B7DA7B7486\r\n"
"To: <sip:test2@192.168.74.12>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 9DF4125236B37C8903C1465C929EA313\r\n"
"CSeq: 1 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:13982;branch=z9hG4bK52C66871D7B732FB7C2970EF64A1CD77-0;ms-received-port=13982;ms-received-cid=25100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

const char raw_lync_unauthorized2[] =
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

const char raw_lync_ok[] =
"SIP/2.0 200 OK\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"E23CD474\", srand=\"973D56C9\", snum=\"5\", rspauth=\"01000000000000008ed442d47eccd46a\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"P-Asserted-Identity: <sip:test2@lync.loc>\r\n"
"Via: SIP/2.0/TLS 192.168.66.57;branch=z9hG4bKBA8FB102EBD8D62FB32CD9BCE2645411-1;received=192.168.66.57;ms-received-port=16451;ms-received-cid=1FD200\r\n"
"From: \"test4\"<sip:test4@lync.loc>;tag=6CBFE75D4C76D79BA9C5247278F80176;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;epid=d2ff665153;tag=f4482b7273\r\n"
"Call-ID: 29825BB510065422A6E6103A3AACB4A5\r\n"
"CSeq: 1 INVITE\r\n"
"Record-Route: <sip:LYNC2013-SERVER.lync.loc:5061;transport=tls;opaque=state:T:F:Ci.R1fd200;lr;ms-route-sig=ga-x1dCN4_wDGxroSpopZJJZzgcA0vmqmBd_0iwRNIyo2yZsXLMfgzWAAA>\r\n"
"Contact: <sip:test2@lync.loc;opaque=user:epid:JLcBefPLmVK_0AQYJ-UEiAAA;gruu>\r\n"
"User-Agent: UCCAPI/15.0.4569.1503 OC/15.0.4569.1503 (Microsoft Lync)\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, UPDATE, REFER, NOTIFY, BENOTIFY, OPTIONS\r\n"
"ms-client-diagnostics: 51007;reason=\"Callee media connectivity diagnosis info\";CalleeMediaDebug=\"audio:ICEWarn=0x80000,LocalSite=192.168.56.1:14832,RemoteSite=192.168.66.57:6002,PortRange=1025:65000,LocalLocation=2,RemoteLocation=0,FederationType=0,Interfaces=0x12\"\r\n"
"ms-endpoint-location-data: NetworkScope;ms-media-location-type=Intranet\r\n"
"Supported: replaces\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: 2283\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 1 IN IP4 192.168.66.57\r\n"
"s=noname\r\n"
"c=IN IP4 192.168.66.57\r\n"
"b=CT:99980\r\n"
"t=0 0\r\n"
"a=x-mediabw:main-video send=4000;recv=4000\r\n"
"a=x-devicecaps:audio:send,recv;video:send,recv\r\n"
"m=audio 23508 RTP/SAVP 103 9 0 8\r\n"
"a=x-ssrc-range:3933771607-3933771607\r\n"
"a=rtcp-fb:* x-message app send:dsh recv:dsh\r\n"
"a=rtcp-rsize\r\n"
"a=label:main-audio\r\n"
"a=x-source:main-audio\r\n"
"a=ice-ufrag:0sfO\r\n"
"a=ice-pwd:OjyZ/pr8oLuDPWcmj9gDMEav\r\n"
"a=candidate:1 1 UDP 2130706431 192.168.56.1 14832 typ host \r\n"
"a=candidate:1 2 UDP 2130705918 192.168.56.1 14833 typ host \r\n"
"a=candidate:2 1 UDP 2130705919 192.168.66.57 23508 typ host \r\n"
"a=candidate:2 2 UDP 2130705406 192.168.66.57 23509 typ host \r\n"
"a=x-candidate-ipv6:3 1 UDP 33553407 2001:0:5ef5:79fb:3ca3:36ab:fa66:7cf9 24634 typ host \r\n"
"a=x-candidate-ipv6:3 2 UDP 33552894 2001:0:5ef5:79fb:3ca3:36ab:fa66:7cf9 24635 typ host \r\n"
"a=candidate:4 1 TCP-ACT 1684797951 192.168.56.1 14832 typ srflx raddr 192.168.56.1 rport 14832 \r\n"
"a=candidate:4 2 TCP-ACT 1684797438 192.168.56.1 14832 typ srflx raddr 192.168.56.1 rport 14832 \r\n"
"a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:9+s+/+675CaW+4oOF+C4ViBZc2nIjsFEVVoJbL0J|2^31\r\n"
"a=maxptime:200\r\n"
"a=rtpmap:103 G7221/16000\r\n"
"a=fmtp:103 bitrate=24000\r\n"
"a=rtpmap:9 G722/8000\r\n"
"a=rtpmap:0 PCMU/8000\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=ptime:20\r\n"
"m=video 32444 RTP/SAVP 122\r\n"
"a=x-ssrc-range:3933771608-3933771707\r\n"
"a=rtcp-fb:* x-message app send:src,x-pli recv:src,x-pli\r\n"
"a=rtcp-rsize\r\n"
"a=label:main-video\r\n"
"a=x-source:main-video\r\n"
"a=ice-ufrag:bjVF\r\n"
"a=ice-pwd:Gqh3DXwZsRGEbbnx5TuaOYYk\r\n"
"a=candidate:1 1 UDP 2130706431 192.168.56.1 18708 typ host \r\n"
"a=candidate:1 2 UDP 2130705918 192.168.56.1 18709 typ host \r\n"
"a=candidate:2 1 UDP 2130705919 192.168.66.57 32444 typ host \r\n"
"a=candidate:2 2 UDP 2130705406 192.168.66.57 32445 typ host \r\n"
"a=x-candidate-ipv6:3 1 UDP 33553407 2001:0:5ef5:79fb:3ca3:36ab:fa66:7cf9 16172 typ host \r\n"
"a=x-candidate-ipv6:3 2 UDP 33552894 2001:0:5ef5:79fb:3ca3:36ab:fa66:7cf9 16173 typ host \r\n"
"a=candidate:4 1 TCP-ACT 1684797951 192.168.56.1 18708 typ srflx raddr 192.168.56.1 rport 18708 \r\n"
"a=candidate:4 2 TCP-ACT 1684797438 192.168.56.1 18708 typ srflx raddr 192.168.56.1 rport 18708 \r\n"
"a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:5tBMMHlqa8Ms9KaXJhU8CpES7IVilFxRZq2w6kzP|2^31\r\n"
"a=rtpmap:122 X-H264UC/90000\r\n"
"a=fmtp:122 packetization-mode=1;mst-mode=NI-TC\r\n";

const char raw_lync_invite[] =
"INVITE sip:test4@192.168.66.57:3538;transport=tls SIP/2.0\r\n"
"Record-Route: <sip:LYNC2013-SERVER.lync.loc:5061;transport=tls;opaque=state:T:F:Ci.Rc6ee00;lr;ms-route-sig=agtnI0PHO0jvTWlLPLdbdZfVUQdZaaHxAelO7o-qBkMiW1Q_j_MfgzWAAA>;tag=0C757E5CA9712CAAD078B70A0C7F28D2\r\n"
"Via: SIP/2.0/TLS 192.168.74.12:5061;branch=z9hG4bK3A32B5F7.FFF843B5B9F6A7E2;branched=FALSE;ms-internal-info=\"cmXEG-f45t4EFOjg4c9f5tF97PGVFcS4f0BO_A9x1T0H-1Q_j_NIrDIwAA\"\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"0F67E368\", srand=\"91847696\", snum=\"3\", rspauth=\"01000000373035390b1a578e7dc964b7\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Max-Forwards: 69\r\n"
"Content-Length: 6581\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:1590;ms-received-port=1590;ms-received-cid=C6B200\r\n"
"P-Asserted-Identity: \"test2\"<sip:test2@lync.loc>\r\n"
"From: \"test2\"<sip:test2@lync.loc>;tag=c5a4bb5e07;epid=d2ff665153\r\n"
"To: <sip:test4@lync.loc>;epid=01010101\r\n"
"Call-ID: a45397523c114f738470ffb675e3bd75\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:test2@lync.loc;opaque=user:epid:JLcBefPLmVK_0AQYJ-UEiAAA;gruu>\r\n"
"User-Agent: UCCAPI/15.0.4569.1503 OC/15.0.4569.1503 (Microsoft Lync)\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"Ms-Conversation-ID: AdKCt6xl6HOex5lgSXKtavryVhRJlw==\r\n"
"Supported: timer\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-sender\r\n"
"Supported: ms-early-media\r\n"
"Supported: 100rel\r\n"
"ms-keep-alive: UAC;hop-hop=yes\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, UPDATE, REFER, NOTIFY, BENOTIFY, OPTIONS\r\n"
"ms-subnet: 192.168.66.0\r\n"
"Accept-Language: ru-RU\r\n"
"ms-endpoint-location-data: NetworkScope;ms-media-location-type=Intranet\r\n"
"Supported: replaces\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: multipart/alternative;boundary=\"----=_NextPart_000_001C_01D282D0.D2055920\"\r\n"
"history-info: <sip:test4@lync.loc>;index=1\r\n"
"\r\n"
"------=_NextPart_000_001C_01D282D0.D2055920\r\n"
"Content-Type: application/sdp\r\n"
"Content-Transfer-Encoding: 7bit\r\n"
"Content-ID: <620c23ac899ea6b8cc5ac222d052211d@lync.loc>\r\n"
"Content-Disposition: session; handling=optional; ms-proxy-2007fallback\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.56.1\r\n"
"s=session\r\n"
"c=IN IP4 192.168.56.1\r\n"
"b=CT:99980\r\n"
"t=0 0\r\n"
"m=audio 11650 RTP/SAVP 117 114 104 9 112 111 0 8 116 115 103 97 13 118 101\r\n"
"a=candidate:d5lFvGToRDKh/UhyFNN9rharqthzUrF2FIRP+boPcnE 1 1Riht1AWk/x7TSyhrZQfDA UDP 0.830 192.168.66.57 31848 \r\n"
"a=candidate:d5lFvGToRDKh/UhyFNN9rharqthzUrF2FIRP+boPcnE 2 1Riht1AWk/x7TSyhrZQfDA UDP 0.830 192.168.66.57 31849 \r\n"
"a=candidate:1KnO5SLcp8Sm96p3mM/TcHP94AhjFZki4nCkkat7hWw 1 0w7pS47gkm5ShOad9xcsug UDP 0.840 192.168.56.1 11650 \r\n"
"a=candidate:1KnO5SLcp8Sm96p3mM/TcHP94AhjFZki4nCkkat7hWw 2 0w7pS47gkm5ShOad9xcsug UDP 0.840 192.168.56.1 11651 \r\n"
"a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:T1EEGzmyDXu8BLLX6Elp/ZH2p00z1SsvYA/sW/g8|2^31|1:1\r\n"
"a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:I02Vheh4j82HVcofoXMjaPZXfaJYBw/F/L7wpzP6|2^31|1:1\r\n"
"a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:/LJ71gEY/PPlVWDoPS5AGVjCZ1QXS/QOqLb37JiK|2^31\r\n"
"a=maxptime:200\r\n"
"a=rtpmap:117 G722/8000/2\r\n"
"a=rtpmap:114 x-msrta/16000\r\n"
"a=fmtp:114 bitrate=29000\r\n"
"a=rtpmap:104 SILK/16000\r\n"
"a=fmtp:104 useinbandfec=1; usedtx=0\r\n"
"a=rtpmap:9 G722/8000\r\n"
"a=rtpmap:112 G7221/16000\r\n"
"a=fmtp:112 bitrate=24000\r\n"
"a=rtpmap:111 SIREN/16000\r\n"
"a=fmtp:111 bitrate=16000\r\n"
"a=rtpmap:0 PCMU/8000\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=rtpmap:116 AAL2-G726-32/8000\r\n"
"a=rtpmap:115 x-msrta/8000\r\n"
"a=fmtp:115 bitrate=11800\r\n"
"a=rtpmap:103 SILK/8000\r\n"
"a=fmtp:103 useinbandfec=1; usedtx=0\r\n"
"a=rtpmap:97 RED/8000\r\n"
"a=rtpmap:13 CN/8000\r\n"
"a=rtpmap:118 CN/16000\r\n"
"a=rtpmap:101 telephone-event/8000\r\n"
"a=fmtp:101 0-16\r\n"
"a=ptime:20\r\n"
"m=video 4778 RTP/SAVP 122 121 123\r\n"
"a=x-caps:121 263:1920:1080:30.0:2000000:1;4359:1280:720:30.0:1500000:1;8455:640:480:30.0:600000:1;12551:640:360:30.0:600000:1;16647:352:288:15.0:250000:1;20743:424:240:15.0:250000:1;24839:176:144:15.0:180000:1\r\n"
"a=candidate:50I4LPZA7hODi2cQ9inLUZQFH9XbfTjZZTJxRut9mr8 1 qxGJSrOI0V7lN6gQB21/mQ UDP 0.830 192.168.66.57 15428 \r\n"
"a=candidate:50I4LPZA7hODi2cQ9inLUZQFH9XbfTjZZTJxRut9mr8 2 qxGJSrOI0V7lN6gQB21/mQ UDP 0.830 192.168.66.57 15429 \r\n"
"a=candidate:7PRncjQDueZEw6x+25fk5B+F4rz7b3LPa3ajuMPCKss 1 EAV7z4Bl0ol7CCNZq5vgKg UDP 0.840 192.168.56.1 4778 \r\n"
"a=candidate:7PRncjQDueZEw6x+25fk5B+F4rz7b3LPa3ajuMPCKss 2 EAV7z4Bl0ol7CCNZq5vgKg UDP 0.840 192.168.56.1 4779 \r\n"
"a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:10Ijv6TcpgqNxtUWs4oWrhbOym5Aw+W+YYVgT22O|2^31|1:1\r\n"
"a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:lO/Uyfojg75bOECdXvwQTjGxEwrKFb9wCCggIGXv|2^31|1:1\r\n"
"a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:knIOYI7zOT0BXv6Bn/gcNRBboKrdXFpl0xGXlO2K|2^31\r\n"
"a=rtpmap:122 X-H264UC/90000\r\n"
"a=fmtp:122 packetization-mode=1;mst-mode=NI-TC\r\n"
"a=rtpmap:121 x-rtvc1/90000\r\n"
"a=rtpmap:123 x-ulpfecuc/90000\r\n"
"\r\n"
"------=_NextPart_000_001C_01D282D0.D2055920\r\n"
"Content-Type: application/sdp\r\n"
"Content-Transfer-Encoding: 7bit\r\n"
"Content-ID: <37df6f7ddd9b0e56592077f652b025ce@lync.loc>\r\n"
"Content-Disposition: session; handling=optional\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 1 IN IP4 192.168.56.1\r\n"
"s=session\r\n"
"c=IN IP4 192.168.56.1\r\n"
"b=CT:99980\r\n"
"t=0 0\r\n"
"a=x-mediabw:main-video send=4000;recv=4000\r\n"
"a=x-devicecaps:audio:send,recv;video:send,recv\r\n"
"m=audio 15676 RTP/SAVP 117 114 104 9 112 111 0 8 116 115 103 97 13 118 101\r\n"
"a=x-ssrc-range:1330447912-1330447912\r\n"
"a=rtcp-fb:* x-message app send:dsh recv:dsh\r\n"
"a=rtcp-rsize\r\n"
"a=label:main-audio\r\n"
"a=x-source:main-audio\r\n"
"a=ice-ufrag:M+7x\r\n"
"a=ice-pwd:iqMvxVs03gM/jyhb9feLQ466\r\n"
"a=candidate:1 1 UDP 2130706431 192.168.56.1 15676 typ host \r\n"
"a=candidate:1 2 UDP 2130705918 192.168.56.1 15677 typ host \r\n"
"a=candidate:2 1 UDP 2130705919 192.168.66.57 7538 typ host \r\n"
"a=candidate:2 2 UDP 2130705406 192.168.66.57 7539 typ host \r\n"
"a=x-candidate-ipv6:3 1 UDP 33553407 2001:0:5ef5:79fd:34b2:244d:fa66:7cf9 2936 typ host \r\n"
"a=x-candidate-ipv6:3 2 UDP 33552894 2001:0:5ef5:79fd:34b2:244d:fa66:7cf9 2937 typ host \r\n"
"a=candidate:4 1 TCP-ACT 1684797951 192.168.56.1 15676 typ srflx raddr 192.168.56.1 rport 15676 \r\n"
"a=candidate:4 2 TCP-ACT 1684797438 192.168.56.1 15676 typ srflx raddr 192.168.56.1 rport 15676 \r\n"
"a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:T1EEGzmyDXu8BLLX6Elp/ZH2p00z1SsvYA/sW/g8|2^31|1:1\r\n"
"a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:I02Vheh4j82HVcofoXMjaPZXfaJYBw/F/L7wpzP6|2^31|1:1\r\n"
"a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:/LJ71gEY/PPlVWDoPS5AGVjCZ1QXS/QOqLb37JiK|2^31\r\n"
"a=maxptime:200\r\n"
"a=rtpmap:117 G722/8000/2\r\n"
"a=rtpmap:114 x-msrta/16000\r\n"
"a=fmtp:114 bitrate=29000\r\n"
"a=rtpmap:104 SILK/16000\r\n"
"a=fmtp:104 useinbandfec=1; usedtx=0\r\n"
"a=rtpmap:9 G722/8000\r\n"
"a=rtpmap:112 G7221/16000\r\n"
"a=fmtp:112 bitrate=24000\r\n"
"a=rtpmap:111 SIREN/16000\r\n"
"a=fmtp:111 bitrate=16000\r\n"
"a=rtpmap:0 PCMU/8000\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=rtpmap:116 AAL2-G726-32/8000\r\n"
"a=rtpmap:115 x-msrta/8000\r\n"
"a=fmtp:115 bitrate=11800\r\n"
"a=rtpmap:103 SILK/8000\r\n"
"a=fmtp:103 useinbandfec=1; usedtx=0\r\n"
"a=rtpmap:97 RED/8000\r\n"
"a=rtpmap:13 CN/8000\r\n"
"a=rtpmap:118 CN/16000\r\n"
"a=rtpmap:101 telephone-event/8000\r\n"
"a=fmtp:101 0-16\r\n"
"a=ptime:20\r\n"
"m=video 12852 RTP/SAVP 122 121 123\r\n"
"a=x-ssrc-range:1330447913-1330448012\r\n"
"a=rtcp-fb:* x-message app send:src,x-pli recv:src,x-pli\r\n"
"a=rtcp-rsize\r\n"
"a=label:main-video\r\n"
"a=x-source:main-video\r\n"
"a=ice-ufrag:lmCJ\r\n"
"a=ice-pwd:Oow3nRFPa5zy4n3BphgbGzlN\r\n"
"a=x-caps:121 263:1920:1080:30.0:2000000:1;4359:1280:720:30.0:1500000:1;8455:640:480:30.0:600000:1;12551:640:360:30.0:600000:1;16647:352:288:15.0:250000:1;20743:424:240:15.0:250000:1;24839:176:144:15.0:180000:1\r\n"
"a=candidate:1 1 UDP 2130706431 192.168.56.1 12852 typ host \r\n"
"a=candidate:1 2 UDP 2130705918 192.168.56.1 12853 typ host \r\n"
"a=candidate:2 1 UDP 2130705919 192.168.66.57 19398 typ host \r\n"
"a=candidate:2 2 UDP 2130705406 192.168.66.57 19399 typ host \r\n"
"a=x-candidate-ipv6:3 1 UDP 33553407 2001:0:5ef5:79fd:34b2:244d:fa66:7cf9 25684 typ host \r\n"
"a=x-candidate-ipv6:3 2 UDP 33552894 2001:0:5ef5:79fd:34b2:244d:fa66:7cf9 25685 typ host \r\n"
"a=candidate:4 1 TCP-ACT 1684797951 192.168.56.1 12852 typ srflx raddr 192.168.56.1 rport 12852 \r\n"
"a=candidate:4 2 TCP-ACT 1684797438 192.168.56.1 12852 typ srflx raddr 192.168.56.1 rport 12852 \r\n"
"a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:10Ijv6TcpgqNxtUWs4oWrhbOym5Aw+W+YYVgT22O|2^31|1:1\r\n"
"a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:lO/Uyfojg75bOECdXvwQTjGxEwrKFb9wCCggIGXv|2^31|1:1\r\n"
"a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:knIOYI7zOT0BXv6Bn/gcNRBboKrdXFpl0xGXlO2K|2^31\r\n"
"a=rtpmap:122 X-H264UC/90000\r\n"
"a=fmtp:122 packetization-mode=1;mst-mode=NI-TC\r\n"
"a=rtpmap:121 x-rtvc1/90000\r\n"
"a=rtpmap:123 x-ulpfecuc/90000\r\n"
"\r\n"
"------=_NextPart_000_001C_01D282D0.D2055920--\r\n\r\n";

const char raw_lync_chat_invite[] =
"INVITE sip:user2@192.168.66.57:3232;transport=tls SIP/2.0\r\n"
"Record-Route: <sip:stego07.skype2015.loc:5061;transport=tls;opaque=state:T:F:Ci.R3f0c00;lr;ms-route-sig=dafMv69A_iBwIQKr67sQrGN4XDG5ysEN67cY4FXQ5ZI96hmkLQQYH83AAA>;tag=33080DF1E72C6E4AF368B8EE1E483BD3\r\n"
"Via: SIP/2.0/TLS 192.168.73.7:5061;branch=z9hG4bK2DA3892F.D0429AA14DA5C5DB;branched=FALSE;ms-internal-info=\"dbizzcGUAbWb32iQXwKgozvsnUJRCNoqJBkNTultltKZqhmkLQCI7eJwAA\"\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"69F5EF2F\", srand=\"76CB1B20\", snum=\"4\", rspauth=\"01000000653e3c6633b88f047684aeb6\", targetname=\"stego07.skype2015.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Max-Forwards: 69\r\n"
"Content-Length: 246\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:37737;ms-received-port=37737;ms-received-cid=3E8A00\r\n"
"From: \"user1\"<sip:user1@skype2015.loc>;tag=9d5c2a38ba;epid=b84a59c035\r\n"
"To: <sip:user2@skype2015.loc>;epid=46c4b2d3ac\r\n"
"Call-ID: ce878a8bc0ca4c769ca729bc2f56a998\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:user1@skype2015.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu>\r\n"
"User-Agent: UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"ms-text-format: text/plain; charset=UTF-8; ms-body=cXdl\r\n"
"ms-im-format: text/html; charset=UTF-8; ms-body=PFNQQU4gbGFuZz0iZW4tdXMiIHN0eWxlPSJjb2xvcjojMDAwMDAwOyBmb250LWZhbWlseTpTZWdvZSBVSTsgZm9udC1zaXplOjEwcHQ7Ij5xd2U8L1NQQU4+\r\n"
"ms-im-format: text/rtf; charset=UTF-8; ms-body=e1xydGYxXGZiaWRpc1xhbnNpXGFuc2ljcGcxMjUxXGRlZmYwXG5vdWljb21wYXRcZGVmbGFuZzEwNDl7XGZvbnR0Ymx7XGYwXGZuaWxcZmNoYXJzZXQwIFNlZ29lIFVJO317XGYxXGZuaWwgU2Vnb2UgVUk7fX0NCntcY29sb3J0YmwgO1xyZWQwXGdyZWVuMFxibHVlMDt9DQp7XCpcZ2VuZXJhdG9yIFJpY2hlZDIwIDE2LjAuNDQwNX1cdmlld2tpbmQ0XHVjMSANClxwYXJkXGNmMVxmMFxmczIwXGxhbmcxMDMzIHF3ZVxmMVxsYW5nMTA0OVxwYXINCntcKlxseW5jZmxhZ3M8cnRmPTE+fX0NCg==\r\n"
"Supported: ms-embedded-first-message\r\n"
"Supported: ms-delayed-accept\r\n"
"Supported: ms-renders-isf\r\n"
"Supported: ms-renders-gif\r\n"
"Supported: ms-renders-mime-alternative\r\n"
"Ms-Conversation-ID: AdKjt1Xy9lDJ8KRjRYaBRsRaq4N/vw==\r\n"
"Supported: timer\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-sender\r\n"
"Supported: ms-early-media\r\n"
"Roster-Manager: sip:user1@skype2015.loc\r\n"
"EndPoints: <sip:user1@skype2015.loc>, <sip:user2@skype2015.loc>\r\n"
"Supported: com.microsoft.rtc-multiparty\r\n"
"ms-keep-alive: UAC;hop-hop=yes\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY\r\n"
"ms-subnet: 192.168.66.0\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: application/sdp\r\n"
"history-info: <sip:user2@skype2015.loc>;index=1\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n\r\n";

const char raw_lync_chat_ok[] =
"SIP/2.0 200 OK\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"0D55F715\", srand=\"F0242A91\", snum=\"19\", rspauth=\"0100000099000000bc72e2771316752a\", targetname=\"stego07.skype2015.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:19020;branch=z9hG4bK59AF70DBCB98977CAA41F5B2E95CFA3D-1;ms-received-port=19020;ms-received-cid=AB5A00\r\n"
"From: \"user2\"<sip:user2@skype2015.loc>;tag=1E27BE5F4171801F786FDCA07B6D228F;epid=46c4b2d3ac\r\n"
"To: <sip:user1@skype2015.loc>;epid=b84a59c035;tag=1b28537901\r\n"
"Call-ID: EB3C1BC90F032BCB54D58F5EF01FA723\r\n"
"CSeq: 1 INVITE\r\n"
"Record-Route: <sip:stego07.skype2015.loc:5061;transport=tls;opaque=state:T:F:Ci.Rab5a00;lr;ms-route-sig=geYAHJsrsv3E29RK6JTi0UsrNwVavSfGzLzietkHwWssYnTFQSQYH83AAA>\r\n"
"Contact: <sip:user1@skype2015.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu>\r\n"
"User-Agent: UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)\r\n"
"Supported: ms-sender\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: 265\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip sip:user1@skype2015.loc\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite\r\n\r\n";

const char raw_invite_with_tel_event[] =
"INVITE sip:1000@192.168.1.103;user=phone SIP/2.0\r\n"
"Allow: INVITE, ACK, CANCEL, BYE, PRACK, NOTIFY, REFER, SUBSCRIBE, OPTIONS, UPDATE, INFO\r\n"
"Supported: replaces,timer,path,100rel\r\n"
"User-Agent: OmniPCX Enterprise R11.2 l2.300.20.e\r\n"
"Session-Expires: 1800;refresher=uac\r\n"
"Min-SE: 900\r\n"
"P-Asserted-Identity: \"Mahmoud\" <sip:2000@192.168.1.245;user=phone>\r\n"
"Content-Type: application/sdp\r\n"
"To: <sip:1000@192.168.1.103;user=phone>\r\n"
"From: \"Mahmoud\" <sip:2000@192.168.1.245;user=phone>;tag=64f2b3f31c3ea3b5016ca10c27777c3f\r\n"
"Contact: <sip:2000@192.168.1.245;transport=UDP>\r\n"
"Call-ID: 65316581e68c27ba43a72a3eabea74a1@192.168.1.245\r\n"
"CSeq: 954795622 INVITE\r\n"
"Via: SIP/2.0/UDP 192.168.1.245;branch=z9hG4bK6dcdebbe51974c0ac5cd29c83e741ba8\r\n"
"Max-Forwards: 70\r\n"
"Content-Length: 288\r\n"
"\r\n"
"v=0\r\n"
"o=OXE 946772597 946772597 IN IP4 192.168.1.245\r\n"
"s=abs\r\n"
"c=IN IP4 192.168.1.246\r\n"
"t=0 0\r\n"
"m=audio 32544 RTP/AVP 8 18 97\r\n"
"a=sendrecv\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=ptime:20\r\n"
"a=maxptime:30\r\n"
"a=rtpmap:18 G729/8000\r\n"
"a=fmtp:18 annexb=no\r\n"
"a=ptime:20\r\n"
"a=maxptime:40\r\n"
"a=rtpmap:97 telephone-event/8000\r\n";

const char raw_sip_image_ok[] =
"SIP/2.0 200 OK\r\n"
"Via: SIP/2.0/UDP 192.168.8.242:5060;rport;branch=z9hG4bK513820daffc9a9056fa6d4885422b52e\r\n"
"From: <sip:192.168.8.242>;tag=769497ecf0dafc37\r\n"
"To: <sip:192.168.120.1>;tag=FE9F9AD4-15E6\r\n"
"Date: Fri, 07 Jul 2017 08:55:27 GMT\r\n"
"Call-ID: 716a046d34fec0b32481b285393275f8\r\n"
"Server: Cisco-SIPGateway/IOS-12.x\r\n"
"CSeq: 1278847279 OPTIONS\r\n"
"Supported: 100rel,replaces\r\n"
"Allow: INVITE, OPTIONS, BYE, CANCEL, ACK, PRACK, COMET, REFER, SUBSCRIBE, NOTIFY, INFO, UPDATE, REGISTER\r\n"
"Accept: application/sdp\r\n"
"Allow-Events: telephone-event\r\n"
"Content-Length: 453\r\n"
"Content-Type: application/sdp\r\n"
"\r\n"
"v=0\r\n"
"o=CiscoSystemsSIP-GW-UserAgent 3830 7875 IN IP4 192.168.120.1\r\n"
"s=SIP Call\r\n"
"c=IN IP4 192.168.120.1\r\n"
"t=0 0\r\n"
"m=audio 0 RTP/AVP 18 0 8 4 2 15 3\r\n"
"c=IN IP4 192.168.120.1\r\n"
"m=image 0 udptl t38\r\n"
"c=IN IP4 192.168.120.1\r\n"
"a=T38FaxVersion:0\r\n"
"a=T38MaxBitRate:9600\r\n"
"a=T38FaxFillBitRemoval:0\r\n"
"a=T38FaxTranscodingMMR:0\r\n"
"a=T38FaxTranscodingJBIG:0\r\n"
"a=T38FaxRateManagement:transferredTCF\r\n"
"a=T38FaxMaxBuffer:200\r\n"
"a=T38FaxMaxDatagram:72\r\n"
"a=T38FaxUdpEC:t38UDPRedundancy\r\n";


////////Messages for SIP message limit test

char raw_sip_message_over_3000b[] =
"INVITE sip:user2@192.168.66.57:3232;transport=tls SIP/2.0\r\n"
"Record-Route: <sip:stego07.skype2015.loc:5061;transport=tls;opaque=state:T:F:Ci.R3f0c00;lr;ms-route-sig=dafMv69A_iBwIQKr67sQrGN4XDG5ysEN67cY4FXQ5ZI96hmkLQQYH83AAA>;tag=33080DF1E72C6E4AF368B8EE1E483BD3\r\n"
"Via: SIP/2.0/TLS 192.168.73.7:5061;branch=z9hG4bK2DA3892F.D0429AA14DA5C5DB;branched=FALSE;ms-internal-info=\"dbizzcGUAbWb32iQXwKgozvsnUJRCNoqJBkNTultltKZqhmkLQCI7eJwAA\"\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"69F5EF2F\", srand=\"76CB1B20\", snum=\"4\", rspauth=\"01000000653e3c6633b88f047684aeb6\", targetname=\"stego07.skype2015.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Max-Forwards: 69\r\n"
"Content-Length: 246\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:37737;ms-received-port=37737;ms-received-cid=3E8A00\r\n"
"From: \"user1\"<sip:user1@skype2015.loc>;tag=9d5c2a38ba;epid=b84a59c035\r\n"
"To: <sip:user2@skype2015.loc>;epid=46c4b2d3ac\r\n"
"Call-ID: ce878a8bc0ca4c769ca729bc2f56a998\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:user1@skype2015.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu>\r\n"
"User-Agent: UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"ms-text-format: text/plain; charset=UTF-8; ms-body=cXdl\r\n"
"ms-im-format: text/html; charset=UTF-8; ms-body=PFNQQU4gbGFuZz0iZW4tdXMiIHN0eWxlPSJjb2xvcjojMDAwMDAwOyBmb250LWZhbWlseTpTZWdvZSBVSTsgZm9udC1zaXplOjEwcHQ7Ij5xd2U8L1NQQU4+\r\n"
"ms-im-format: text/rtf; charset=UTF-8; ms-body=e1xydGYxXGZiaWRpc1xhbnNpXGFuc2ljcGcxMjUxXGRlZmYwXG5vdWljb21wYXRcZGVmbGFuZzEwNDl7XGZvbnR0Ymx7XGYwXGZuaWxcZmNoYXJzZXQwIFNlZ29lIFVJO317XGYxXGZuaWwgU2Vnb2UgVUk7fX0NCntcY29sb3J0YmwgO1xyZWQwXGdyZWVuMFxibHVlMDt9DQp7XCpcZ2VuZXJhdG9yIFJpY2hlZDIwIDE2LjAuNDQwNX1cdmlld2tpbmQ0XHVjMSANClxwYXJkXGNmMVxmMFxmczIwXGxhbmcxMDMzIHF3ZVxmMVxsYW5nMTA0OVxwYXINCntcKlxseW5jZmxhZ3M8cnRmPTE+fX0NCg==\r\n"
"Supported: ms-embedded-first-message\r\n"
"Supported: ms-delayed-accept\r\n"
"Supported: ms-renders-isf\r\n"
"Supported: ms-renders-gif\r\n"
"Supported: ms-renders-mime-alternative\r\n"
"Ms-Conversation-ID: AdKjt1Xy9lDJ8KRjRYaBRsRaq4N/vw==\r\n"
"Supported: timer\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-sender\r\n"
"Supported: ms-early-media\r\n"
"Roster-Manager: sip:user1@skype2015.loc\r\n"
"EndPoints: <sip:user1@skype2015.loc>, <sip:user2@skype2015.loc>\r\n"
"Supported: com.microsoft.rtc-multiparty\r\n"
"ms-keep-alive: UAC;hop-hop=yes\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY\r\n"
"ms-subnet: 192.168.66.0\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: application/sdp\r\n"
"history-info: <sip:user2@skype2015.loc>;index=1\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n";

char raw_2_sip_messages_over_3000b_and_less_1000b[] =
"INVITE sip:user2@192.168.66.57:3232;transport=tls SIP/2.0\r\n"
"Record-Route: <sip:stego07.skype2015.loc:5061;transport=tls;opaque=state:T:F:Ci.R3f0c00;lr;ms-route-sig=dafMv69A_iBwIQKr67sQrGN4XDG5ysEN67cY4FXQ5ZI96hmkLQQYH83AAA>;tag=33080DF1E72C6E4AF368B8EE1E483BD3\r\n"
"Via: SIP/2.0/TLS 192.168.73.7:5061;branch=z9hG4bK2DA3892F.D0429AA14DA5C5DB;branched=FALSE;ms-internal-info=\"dbizzcGUAbWb32iQXwKgozvsnUJRCNoqJBkNTultltKZqhmkLQCI7eJwAA\"\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"69F5EF2F\", srand=\"76CB1B20\", snum=\"4\", rspauth=\"01000000653e3c6633b88f047684aeb6\", targetname=\"stego07.skype2015.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Max-Forwards: 69\r\n"
"Content-Length: 246\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:37737;ms-received-port=37737;ms-received-cid=3E8A00\r\n"
"From: \"user1\"<sip:user1@skype2015.loc>;tag=9d5c2a38ba;epid=b84a59c035\r\n"
"To: <sip:user2@skype2015.loc>;epid=46c4b2d3ac\r\n"
"Call-ID: ce878a8bc0ca4c769ca729bc2f56a998\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:user1@skype2015.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu>\r\n"
"User-Agent: UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"ms-text-format: text/plain; charset=UTF-8; ms-body=cXdl\r\n"
"ms-im-format: text/html; charset=UTF-8; ms-body=PFNQQU4gbGFuZz0iZW4tdXMiIHN0eWxlPSJjb2xvcjojMDAwMDAwOyBmb250LWZhbWlseTpTZWdvZSBVSTsgZm9udC1zaXplOjEwcHQ7Ij5xd2U8L1NQQU4+\r\n"
"ms-im-format: text/rtf; charset=UTF-8; ms-body=e1xydGYxXGZiaWRpc1xhbnNpXGFuc2ljcGcxMjUxXGRlZmYwXG5vdWljb21wYXRcZGVmbGFuZzEwNDl7XGZvbnR0Ymx7XGYwXGZuaWxcZmNoYXJzZXQwIFNlZ29lIFVJO317XGYxXGZuaWwgU2Vnb2UgVUk7fX0NCntcY29sb3J0YmwgO1xyZWQwXGdyZWVuMFxibHVlMDt9DQp7XCpcZ2VuZXJhdG9yIFJpY2hlZDIwIDE2LjAuNDQwNX1cdmlld2tpbmQ0XHVjMSANClxwYXJkXGNmMVxmMFxmczIwXGxhbmcxMDMzIHF3ZVxmMVxsYW5nMTA0OVxwYXINCntcKlxseW5jZmxhZ3M8cnRmPTE+fX0NCg==\r\n"
"Supported: ms-embedded-first-message\r\n"
"Supported: ms-delayed-accept\r\n"
"Supported: ms-renders-isf\r\n"
"Supported: ms-renders-gif\r\n"
"Supported: ms-renders-mime-alternative\r\n"
"Ms-Conversation-ID: AdKjt1Xy9lDJ8KRjRYaBRsRaq4N/vw==\r\n"
"Supported: timer\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-sender\r\n"
"Supported: ms-early-media\r\n"
"Roster-Manager: sip:user1@skype2015.loc\r\n"
"EndPoints: <sip:user1@skype2015.loc>, <sip:user2@skype2015.loc>\r\n"
"Supported: com.microsoft.rtc-multiparty\r\n"
"ms-keep-alive: UAC;hop-hop=yes\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY\r\n"
"ms-subnet: 192.168.66.0\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: application/sdp\r\n"
"history-info: <sip:user2@skype2015.loc>;index=1\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_2_sip_messages_content_part_over_3000b_and_full_less_1000b[] =
"v=0\r\n"
"o=- 0 1 IN IP4 192.168.56.1\r\n"
"s=session\r\n"
"c=IN IP4 192.168.56.1\r\n"
"b=CT:99980\r\n"
"t=0 0\r\n"
"a=x-mediabw:main-video send=4000;recv=4000\r\n"
"a=x-devicecaps:audio:send,recv;video:send,recv\r\n"
"m=audio 15676 RTP/SAVP 117 114 104 9 112 111 0 8 116 115 103 97 13 118 101\r\n"
"a=x-ssrc-range:1330447912-1330447912\r\n"
"a=rtcp-fb:* x-message app send:dsh recv:dsh\r\n"
"a=rtcp-rsize\r\n"
"a=label:main-audio\r\n"
"a=x-source:main-audio\r\n"
"a=ice-ufrag:M+7x\r\n"
"a=ice-pwd:iqMvxVs03gM/jyhb9feLQ466\r\n"
"a=candidate:1 1 UDP 2130706431 192.168.56.1 15676 typ host \r\n"
"a=candidate:1 2 UDP 2130705918 192.168.56.1 15677 typ host \r\n"
"a=candidate:2 1 UDP 2130705919 192.168.66.57 7538 typ host \r\n"
"a=candidate:2 2 UDP 2130705406 192.168.66.57 7539 typ host \r\n"
"a=x-candidate-ipv6:3 1 UDP 33553407 2001:0:5ef5:79fd:34b2:244d:fa66:7cf9 2936 typ host \r\n"
"a=x-candidate-ipv6:3 2 UDP 33552894 2001:0:5ef5:79fd:34b2:244d:fa66:7cf9 2937 typ host \r\n"
"a=candidate:4 1 TCP-ACT 1684797951 192.168.56.1 15676 typ srflx raddr 192.168.56.1 rport 15676 \r\n"
"a=candidate:4 2 TCP-ACT 1684797438 192.168.56.1 15676 typ srflx raddr 192.168.56.1 rport 15676 \r\n"
"a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:T1EEGzmyDXu8BLLX6Elp/ZH2p00z1SsvYA/sW/g8|2^31|1:1\r\n"
"a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:I02Vheh4j82HVcofoXMjaPZXfaJYBw/F/L7wpzP6|2^31|1:1\r\n"
"a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:/LJ71gEY/PPlVWDoPS5AGVjCZ1QXS/QOqLb37JiK|2^31\r\n"
"a=maxptime:200\r\n"
"a=rtpmap:117 G722/8000/2\r\n"
"a=rtpmap:114 x-msrta/16000\r\n"
"a=fmtp:114 bitrate=29000\r\n"
"a=rtpmap:104 SILK/16000\r\n"
"a=fmtp:104 useinbandfec=1; usedtx=0\r\n"
"a=rtpmap:9 G722/8000\r\n"
"a=rtpmap:112 G7221/16000\r\n"
"a=fmtp:112 bitrate=24000\r\n"
"a=rtpmap:111 SIREN/16000\r\n"
"a=fmtp:111 bitrate=16000\r\n"
"a=rtpmap:0 PCMU/8000\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=rtpmap:116 AAL2-G726-32/8000\r\n"
"a=rtpmap:115 x-msrta/8000\r\n"
"a=fmtp:115 bitrate=11800\r\n"
"a=rtpmap:103 SILK/8000\r\n"
"a=fmtp:103 useinbandfec=1; usedtx=0\r\n"
"a=rtpmap:97 RED/8000\r\n"
"a=rtpmap:13 CN/8000\r\n"
"a=rtpmap:118 CN/16000\r\n"
"a=rtpmap:101 telephone-event/8000\r\n"
"a=fmtp:101 0-16\r\n"
"a=ptime:20\r\n"
"m=video 12852 RTP/SAVP 122 121 123\r\n"
"a=x-ssrc-range:1330447913-1330448012\r\n"
"a=rtcp-fb:* x-message app send:src,x-pli recv:src,x-pli\r\n"
"a=rtcp-rsize\r\n"
"a=label:main-video\r\n"
"a=x-source:main-video\r\n"
"a=ice-ufrag:lmCJ\r\n"
"a=ice-pwd:Oow3nRFPa5zy4n3BphgbGzlN\r\n"
"a=x-caps:121 263:1920:1080:30.0:2000000:1;4359:1280:720:30.0:1500000:1;8455:640:480:30.0:600000:1;12551:640:360:30.0:600000:1;16647:352:288:15.0:250000:1;20743:424:240:15.0:250000:1;24839:176:144:15.0:180000:1\r\n"
"a=candidate:1 1 UDP 2130706431 192.168.56.1 12852 typ host \r\n"
"a=candidate:1 2 UDP 2130705918 192.168.56.1 12853 typ host \r\n"
"a=candidate:2 1 UDP 2130705919 192.168.66.57 19398 typ host \r\n"
"a=candidate:2 2 UDP 2130705406 192.168.66.57 19399 typ host \r\n"
"a=x-candidate-ipv6:3 1 UDP 33553407 2001:0:5ef5:79fd:34b2:244d:fa66:7cf9 25684 typ host \r\n"
"a=x-candidate-ipv6:3 2 UDP 33552894 2001:0:5ef5:79fd:34b2:244d:fa66:7cf9 25685 typ host \r\n"
"a=candidate:4 1 TCP-ACT 1684797951 192.168.56.1 12852 typ srflx raddr 192.168.56.1 rport 12852 \r\n"
"a=candidate:4 2 TCP-ACT 1684797438 192.168.56.1 12852 typ srflx raddr 192.168.56.1 rport 12852 \r\n"
"a=cryptoscale:1 client AES_CM_128_HMAC_SHA1_80 inline:10Ijv6TcpgqNxtUWs4oWrhbOym5Aw+W+YYVgT22O|2^31|1:1\r\n"
"a=crypto:2 AES_CM_128_HMAC_SHA1_80 inline:lO/Uyfojg75bOECdXvwQTjGxEwrKFb9wCCggIGXv|2^31|1:1\r\n"
"a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:knIOYI7zOT0BXv6Bn/gcNRBboKrdXFpl0xGXlO2K|2^31\r\n"
"a=rtpmap:122 X-H264UC/90000\r\n"
"a=fmtp:122 packetization-mode=1;mst-mode=NI-TC\r\n"
"a=rtpmap:121 x-rtvc1/90000\r\n"
"a=rtpmap:123 x-ulpfecuc/90000\r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_2_sip_messages_content_part_less_300b_and_full_less_1000b[] =
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_sip_message_less_1000b[] =
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_4_sip_messages_content_part_less_300b_and_3_full_less_1000b[] =
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_sip_message_without_content_over_2500b_p1[] =
"INVITE sip:user2@192.168.66.57:3232;transport=tls SIP/2.0\r\n"
"Record-Route: <sip:stego07.skype2015.loc:5061;transport=tls;opaque=state:T:F:Ci.R3f0c00;lr;ms-route-sig=dafMv69A_iBwIQKr67sQrGN4XDG5ysEN67cY4FXQ5ZI96hmkLQQYH83AAA>;tag=33080DF1E72C6E4AF368B8EE1E483BD3\r\n"
"Via: SIP/2.0/TLS 192.168.73.7:5061;branch=z9hG4bK2DA3892F.D0429AA14DA5C5DB;branched=FALSE;ms-internal-info=\"dbizzcGUAbWb32iQXwKgozvsnUJRCNoqJBkNTultltKZqhmkLQCI7eJwAA\"\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"69F5EF2F\", srand=\"76CB1B20\", snum=\"4\", rspauth=\"01000000653e3c6633b88f047684aeb6\", targetname=\"stego07.skype2015.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Max-Forwards: 69\r\n"
"Content-Length: 246\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:37737;ms-received-port=37737;ms-received-cid=3E8A00\r\n"
"From: \"user1\"<sip:user1@skype2015.loc>;tag=9d5c2a38ba;epid=b84a59c035\r\n"
"To: <sip:user2@skype2015.loc>;epid=46c4b2d3ac\r\n"
"Call-ID: ce878a8bc0ca4c769ca729bc2f56a998\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:user1@skype2015.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu>\r\n"
"User-Agent: UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"ms-text-format: text/plain; charset=UTF-8; ms-body=cXdl\r\n"
"ms-im-format: text/html; charset=UTF-8; ms-body=PFNQQU4gbGFuZz0iZW4tdXMiIHN0eWxlPSJjb2xvcjojMDAwMDAwOyBmb250LWZhbWlseTpTZWdvZSBVSTsgZm9udC1zaXplOjEwcHQ7Ij5xd2U8L1NQQU4+\r\n"
"ms-im-format: text/rtf; charset=UTF-8; ms-body=e1xydGYxXGZiaWRpc1xhbnNpXGFuc2ljcGcxMjUxXGRlZmYwXG5vdWljb21wYXRcZGVmbGFuZzEwNDl7XGZvbnR0Ymx7XGYwXGZuaWxcZmNoYXJzZXQwIFNlZ29lIFVJO317XGYxXGZuaWwgU2Vnb2UgVUk7fX0NCntcY29sb3J0YmwgO1xyZWQwXGdyZWVuMFxibHVlMDt9DQp7XCpcZ2VuZXJhdG9yIFJpY2hlZDIwIDE2LjAuNDQwNX1cdmlld2tpbmQ0XHVjMSANClxwYXJkXGNmMVxmMFxmczIwXGxhbmcxMDMzIHF3ZVxmMVxsYW5nMTA0OVxwYXINCntcKlxseW5jZmxhZ3M8cnRmPTE+fX0NCg==\r\n"
"Supported: ms-embedded-first-message\r\n"
"Supported: ms-delayed-accept\r\n"
"Supported: ms-renders-isf\r\n"
"Supported: ms-renders-gif\r\n"
"Supported: ms-renders-mime-alternative\r\n"
"Ms-Conversation-ID: AdKjt1Xy9lDJ8KRjRYaBRsRaq4N/vw==\r\n"
"Supported: timer\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-sender\r\n"
"Supported: ms-early-media\r\n"
"Roster-Manager: sip:user1@skype2015.loc\r\n"
"EndPoints: <sip:user1@skype2015.loc>, <sip:user2@skype2015.loc>\r\n"
"Supported: com.microsoft.rtc-multiparty\r\n"
"ms-keep-alive: UAC;hop-hop=yes\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY\r\n"
"ms-subnet: 192.168.66.0\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: application/sdp\r\n"
"history-info: <sip:user2@skype2015.loc>;index=1\r\n"
"\r\n";

char raw_sip_message_content_over_plus_other_mess_less_1000b_p2[] =
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_part_of_headers_sip_message_over_2000b_w_c_l[] =
"INVITE sip:user2@192.168.66.57:3232;transport=tls SIP/2.0\r\n"
"Record-Route: <sip:stego07.skype2015.loc:5061;transport=tls;opaque=state:T:F:Ci.R3f0c00;lr;ms-route-sig=dafMv69A_iBwIQKr67sQrGN4XDG5ysEN67cY4FXQ5ZI96hmkLQQYH83AAA>;tag=33080DF1E72C6E4AF368B8EE1E483BD3\r\n"
"Via: SIP/2.0/TLS 192.168.73.7:5061;branch=z9hG4bK2DA3892F.D0429AA14DA5C5DB;branched=FALSE;ms-internal-info=\"dbizzcGUAbWb32iQXwKgozvsnUJRCNoqJBkNTultltKZqhmkLQCI7eJwAA\"\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"69F5EF2F\", srand=\"76CB1B20\", snum=\"4\", rspauth=\"01000000653e3c6633b88f047684aeb6\", targetname=\"stego07.skype2015.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Max-Forwards: 69\r\n"
"Content-Length: 246\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:37737;ms-received-port=37737;ms-received-cid=3E8A00\r\n"
"From: \"user1\"<sip:user1@skype2015.loc>;tag=9d5c2a38ba;epid=b84a59c035\r\n"
"To: <sip:user2@skype2015.loc>;epid=46c4b2d3ac\r\n"
"Call-ID: ce878a8bc0ca4c769ca729bc2f56a998\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:user1@skype2015.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu>\r\n"
"User-Agent: UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"ms-text-format: text/plain; charset=UTF-8; ms-body=cXdl\r\n"
"ms-im-format: text/html; charset=UTF-8; ms-body=PFNQQU4gbGFuZz0iZW4tdXMiIHN0eWxlPSJjb2xvcjojMDAwMDAwOyBmb250LWZhbWlseTpTZWdvZSBVSTsgZm9udC1zaXplOjEwcHQ7Ij5xd2U8L1NQQU4+\r\n"
"ms-im-format: text/rtf; charset=UTF-8; ms-body=e1xydGYxXGZiaWRpc1xhbnNpXGFuc2ljcGcxMjUxXGRlZmYwXG5vdWljb21wYXRcZGVmbGFuZzEwNDl7XGZvbnR0Ymx7XGYwXGZuaWxcZmNoYXJzZXQwIFNlZ29lIFVJO317XGYxXGZuaWwgU2Vnb2UgVUk7fX0NCntcY29sb3J0YmwgO1xyZWQwXGdyZWVuMFxibHVlMDt9DQp7XCpcZ2VuZXJhdG9yIFJpY2hlZDIwIDE2LjAuNDQwNX1cdmlld2tpbmQ0XHVjMSANClxwYXJkXGNmMVxmMFxmczIwXGxhbmcxMDMzIHF3ZVxmMVxsYW5nMTA0OVxwYXINCntcKlxseW5jZmxhZ3M8cnRmPTE+fX0NCg==\r\n"
"Supported: ms-embedded-first-message\r\n"
"Supported: ms-delayed-accept\r\n"
"Supported: ms-renders-isf\r\n"
"Supported: ms-renders-gif\r\n"
"Supported: ms-renders-mime-alternative\r\n"
"Ms-Conversation-ID: AdKjt1Xy9lDJ8KRjRYaBRsRaq4N/vw==\r\n"
"Supported: timer\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-sender\r\n"
"Supported: ms-early-media\r\n"
"Roster-Manager: sip:user1@skype2015.loc\r\n"
"EndPoints: <sip:user1@skype2015.loc>, <sip:user2@skype2015.loc>\r\n"
"Supported: com.microsoft.rtc-multiparty\r\n"
"ms-keep-alive: UAC;hop-hop=yes\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY\r\n"
"ms-subnet: 192.168.66.0\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: application/sdp\r\n";

char raw_part_of_headers_and_content_less_500b_wo_c_l_and_mess_less_1000[] =
"history-info: <sip:user2@skype2015.loc>;index=1\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_part_of_headers_sip_message_over_2000b_wo_c_l[] =
"INVITE sip:user2@192.168.66.57:3232;transport=tls SIP/2.0\r\n"
"Record-Route: <sip:stego07.skype2015.loc:5061;transport=tls;opaque=state:T:F:Ci.R3f0c00;lr;ms-route-sig=dafMv69A_iBwIQKr67sQrGN4XDG5ysEN67cY4FXQ5ZI96hmkLQQYH83AAA>;tag=33080DF1E72C6E4AF368B8EE1E483BD3\r\n"
"Via: SIP/2.0/TLS 192.168.73.7:5061;branch=z9hG4bK2DA3892F.D0429AA14DA5C5DB;branched=FALSE;ms-internal-info=\"dbizzcGUAbWb32iQXwKgozvsnUJRCNoqJBkNTultltKZqhmkLQCI7eJwAA\"\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"69F5EF2F\", srand=\"76CB1B20\", snum=\"4\", rspauth=\"01000000653e3c6633b88f047684aeb6\", targetname=\"stego07.skype2015.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Max-Forwards: 69\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:37737;ms-received-port=37737;ms-received-cid=3E8A00\r\n"
"From: \"user1\"<sip:user1@skype2015.loc>;tag=9d5c2a38ba;epid=b84a59c035\r\n"
"To: <sip:user2@skype2015.loc>;epid=46c4b2d3ac\r\n"
"Call-ID: ce878a8bc0ca4c769ca729bc2f56a998\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:user1@skype2015.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu>\r\n"
"User-Agent: UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"ms-text-format: text/plain; charset=UTF-8; ms-body=cXdl\r\n"
"ms-im-format: text/html; charset=UTF-8; ms-body=PFNQQU4gbGFuZz0iZW4tdXMiIHN0eWxlPSJjb2xvcjojMDAwMDAwOyBmb250LWZhbWlseTpTZWdvZSBVSTsgZm9udC1zaXplOjEwcHQ7Ij5xd2U8L1NQQU4+\r\n"
"ms-im-format: text/rtf; charset=UTF-8; ms-body=e1xydGYxXGZiaWRpc1xhbnNpXGFuc2ljcGcxMjUxXGRlZmYwXG5vdWljb21wYXRcZGVmbGFuZzEwNDl7XGZvbnR0Ymx7XGYwXGZuaWxcZmNoYXJzZXQwIFNlZ29lIFVJO317XGYxXGZuaWwgU2Vnb2UgVUk7fX0NCntcY29sb3J0YmwgO1xyZWQwXGdyZWVuMFxibHVlMDt9DQp7XCpcZ2VuZXJhdG9yIFJpY2hlZDIwIDE2LjAuNDQwNX1cdmlld2tpbmQ0XHVjMSANClxwYXJkXGNmMVxmMFxmczIwXGxhbmcxMDMzIHF3ZVxmMVxsYW5nMTA0OVxwYXINCntcKlxseW5jZmxhZ3M8cnRmPTE+fX0NCg==\r\n"
"Supported: ms-embedded-first-message\r\n"
"Supported: ms-delayed-accept\r\n"
"Supported: ms-renders-isf\r\n"
"Supported: ms-renders-gif\r\n"
"Supported: ms-renders-mime-alternative\r\n"
"Ms-Conversation-ID: AdKjt1Xy9lDJ8KRjRYaBRsRaq4N/vw==\r\n"
"Supported: timer\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-sender\r\n"
"Supported: ms-early-media\r\n"
"Roster-Manager: sip:user1@skype2015.loc\r\n"
"EndPoints: <sip:user1@skype2015.loc>, <sip:user2@skype2015.loc>\r\n"
"Supported: com.microsoft.rtc-multiparty\r\n"
"ms-keep-alive: UAC;hop-hop=yes\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY\r\n"
"ms-subnet: 192.168.66.0\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: application/sdp\r\n";

char raw_part_of_headers_and_content_less_500b_w_c_l_and_mess_less_1000[] =
"Content-Length: 246\r\n"
"history-info: <sip:user2@skype2015.loc>;index=1\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n"
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_part_of_headers_sip_message_over_2000b_part_of_content[] =
"INVITE sip:user2@192.168.66.57:3232;transport=tls SIP/2.0\r\n"
"Record-Route: <sip:stego07.skype2015.loc:5061;transport=tls;opaque=state:T:F:Ci.R3f0c00;lr;ms-route-sig=dafMv69A_iBwIQKr67sQrGN4XDG5ysEN67cY4FXQ5ZI96hmkLQQYH83AAA>;tag=33080DF1E72C6E4AF368B8EE1E483BD3\r\n"
"Via: SIP/2.0/TLS 192.168.73.7:5061;branch=z9hG4bK2DA3892F.D0429AA14DA5C5DB;branched=FALSE;ms-internal-info=\"dbizzcGUAbWb32iQXwKgozvsnUJRCNoqJBkNTultltKZqhmkLQCI7eJwAA\"\r\n"
"Authentication-Info: NTLM qop=\"auth\", opaque=\"69F5EF2F\", srand=\"76CB1B20\", snum=\"4\", rspauth=\"01000000653e3c6633b88f047684aeb6\", targetname=\"stego07.skype2015.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"Max-Forwards: 69\r\n"
"Content-Length: 246\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:37737;ms-received-port=37737;ms-received-cid=3E8A00\r\n"
"From: \"user1\"<sip:user1@skype2015.loc>;tag=9d5c2a38ba;epid=b84a59c035\r\n"
"To: <sip:user2@skype2015.loc>;epid=46c4b2d3ac\r\n"
"Call-ID: ce878a8bc0ca4c769ca729bc2f56a998\r\n"
"CSeq: 1 INVITE\r\n"
"Contact: <sip:user1@skype2015.loc;opaque=user:epid:hYnsEUpJhlGc0T-xd_I7vAAA;gruu>\r\n"
"User-Agent: UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)\r\n"
"Supported: ms-dialog-route-set-update\r\n"
"ms-text-format: text/plain; charset=UTF-8; ms-body=cXdl\r\n"
"ms-im-format: text/html; charset=UTF-8; ms-body=PFNQQU4gbGFuZz0iZW4tdXMiIHN0eWxlPSJjb2xvcjojMDAwMDAwOyBmb250LWZhbWlseTpTZWdvZSBVSTsgZm9udC1zaXplOjEwcHQ7Ij5xd2U8L1NQQU4+\r\n"
"ms-im-format: text/rtf; charset=UTF-8; ms-body=e1xydGYxXGZiaWRpc1xhbnNpXGFuc2ljcGcxMjUxXGRlZmYwXG5vdWljb21wYXRcZGVmbGFuZzEwNDl7XGZvbnR0Ymx7XGYwXGZuaWxcZmNoYXJzZXQwIFNlZ29lIFVJO317XGYxXGZuaWwgU2Vnb2UgVUk7fX0NCntcY29sb3J0YmwgO1xyZWQwXGdyZWVuMFxibHVlMDt9DQp7XCpcZ2VuZXJhdG9yIFJpY2hlZDIwIDE2LjAuNDQwNX1cdmlld2tpbmQ0XHVjMSANClxwYXJkXGNmMVxmMFxmczIwXGxhbmcxMDMzIHF3ZVxmMVxsYW5nMTA0OVxwYXINCntcKlxseW5jZmxhZ3M8cnRmPTE+fX0NCg==\r\n"
"Supported: ms-embedded-first-message\r\n"
"Supported: ms-delayed-accept\r\n"
"Supported: ms-renders-isf\r\n"
"Supported: ms-renders-gif\r\n"
"Supported: ms-renders-mime-alternative\r\n"
"Ms-Conversation-ID: AdKjt1Xy9lDJ8KRjRYaBRsRaq4N/vw==\r\n"
"Supported: timer\r\n"
"Supported: histinfo\r\n"
"Supported: ms-safe-transfer\r\n"
"Supported: ms-sender\r\n"
"Supported: ms-early-media\r\n"
"Roster-Manager: sip:user1@skype2015.loc\r\n"
"EndPoints: <sip:user1@skype2015.loc>, <sip:user2@skype2015.loc>\r\n"
"Supported: com.microsoft.rtc-multiparty\r\n"
"ms-keep-alive: UAC;hop-hop=yes\r\n"
"Allow: INVITE, BYE, ACK, CANCEL, INFO, MESSAGE, UPDATE, REFER, NOTIFY, BENOTIFY\r\n"
"ms-subnet: 192.168.66.0\r\n"
"Supported: ms-conf-invite\r\n"
"Content-Type: application/sdp\r\n"
"history-info: <sip:user2@skype2015.loc>;index=1\r\n"
"\r\n"
"v=0\r\n"
"o=- 0 0 IN IP4 192.168.66.57\r\n"
"s=session\r\n";

char raw_part_of_content_less_500b_and_mess_less_1000[] =
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n"
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

char raw_part_of_content_less_100b[] =
"c=IN IP4 192.168.66.57\r\n"
"t=0 0\r\n"
"m=message 5060 sip null\r\n";
char raw_part_of_content_less_300b_and_mess_less_1000[] =
"a=accept-types:text/plain multipart/alternative image/gif text/rtf text/html application/x-ms-ink application/ms-imdn+xml text/x-msmsgsinvite \r\n"
"SIP/2.0 401 Unauthorized\r\n"
"Date: Thu, 08 Dec 2016 09:39:41 GMT\r\n"
"WWW-Authenticate: NTLM opaque=\"78D29AC8\", gssapi-data=\"TlRMTVNTUAACAAAAAAAAADgAAADzgpjiyggGf8bkagEAAAAAAAAAAJoAmgA4AAAABgGxHQAAAA8CAAgATABZAE4AQwABAB4ATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIABAAQAGwAeQBuAGMALgBsAG8AYwADADAATABZAE4AQwAyADAAMQAzAC0AUwBFAFIAVgBFAFIALgBsAHkAbgBjAC4AbABvAGMABQAQAGwAeQBuAGMALgBsAG8AYwAHAAgA3Uju/zZR0gEAAAAA\", targetname=\"LYNC2013-SERVER.lync.loc\", realm=\"SIP Communications Service\", version=4\r\n"
"From: <sip:test2@lync.loc>;tag=8E9A17553EB6074EA50DD285529213AC;epid=01010101\r\n"
"To: <sip:test2@lync.loc>;tag=AC5312E6748A51BA96E6D129AB85A765\r\n"
"Call-ID: 423C0DE2E19B21F63D4CB068D359BABD\r\n"
"CSeq: 2 REGISTER\r\n"
"Via: SIP/2.0/TLS 192.168.66.57:21877;branch=z9hG4bKB91A85A9DE279EA5606CBE050949347D-1;ms-received-port=21877;ms-received-cid=35D100\r\n"
"Server: RTC/5.0\r\n"
"Content-Length: 0\r\n\r\n";

const char raw_sip_registration_unauthorized_401Unauthorized[] =
"SIP/2.0 401 Unauthorized\r\n"
"Via: SIP/2.0/TCP __from__;branch=__branch__;received=80.91.181.218;rport=56747\r\n"
"From: <sip:__user_from__@__host_from__>;tag=__tag_from__\r\n"
"To: <sip:__user_to__@__host_to__>;tag=as609cb1aa\r\n"
"Call-ID: __callid__\r\n"
"CSeq: __cseq__ REGISTER\r\n"
"Server: Asterisk PBX 11.13.1~dfsg-2+deb8u2\r\n"
"Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE\r\n"
"Supported: replaces, timer\r\n"
"WWW-Authenticate: Digest algorithm = MD5, realm = \"trueconf.ru\", nonce = \"6a3d9ed5\"\r\n"
"Content-Length : 0\r\n\r\n";

const char raw_sip_registration_200Ok[] =
"SIP/2.0 200 OK\r\n"
"Via: SIP/2.0/TCP __from__;branch=__branch__;received=80.91.181.218;rport=56747\r\n"
"From: <sip:__user_from__@__host_from__>;tag=__tag_from__\r\n"
"To: <sip:__user_to__@__host_to__>;tag=as609cb1aa\r\n"
"Call-ID: __callid__\r\n"
"CSeq: __cseq__ REGISTER\r\n"
"Server: Asterisk PBX 11.13.1~dfsg-2+deb8u2\r\n"
"Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE\r\n"
"Supported: replaces, timer\r\n"
"Expires: __expires__\r\n"
"Contact: <sip:__contact_user__@__contact_host__;transport=tcp>;expires=60\r\n"
"Date: Tue, 17 Apr 2018 14:54:49 GMT\r\n"
"Content-Length: 0\r\n\r\n";

const char raw_sip_registration_403Forbidden[] =
"SIP/2.0 403 Forbidden\r\n"
"Via: SIP/2.0/TCP __from__;branch=__branch__;received=80.91.181.218;rport=6313\r\n"
"From: <sip:__user_from__@__host_from__>;tag=__tag_from__\r\n"
"To: <sip:__user_to__@__host_to__>;tag=as58e28f6f\r\n"
"Call-ID: __callid__\r\n"
"CSeq: __cseq__ REGISTER\r\n"
"Server: Asterisk PBX 11.13.1~dfsg-2+deb8u2\r\n"
"Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE\r\n"
"Supported: replaces, timer\r\n"
"Content-Length: 0\r\n\r\n";
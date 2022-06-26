#pragma once

#include "../../std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_ALL_

#define DPRINT_SAMPLES_NUMBER (40)

#define SAMPLE(n, fmt, ...) \
	case n:\
	{\
	    out_fmt = (char *) fmt;\
	    dprint0(fmt, __VA_ARGS__); \
	}\
	break;

// call without arguments except format string
#define SAMPLE0(n, fmt) \
	case n:\
	{\
	    out_fmt = (char *)fmt; \
	    dprint0(fmt); \
	}\
	break;

inline int get_dprint_samples_count(void)
{
	return DPRINT_SAMPLES_NUMBER;
}

inline void run_dprint_sample(size_t sample_number, char *&out_fmt)
{
	switch (sample_number)
	{
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_USPRS
#endif
		// PRESENC
		SAMPLE(0, "@SeqPost to %s:%s seq_id = % 08x\n", "ru7.trueconf.net#as", "kerer03@trueconf.com", 2);
		SAMPLE(1, "@GetServer for call_id =%s, use_cache = %d\n", "bobyr.rastumaliev@trueconf.com", 1);
		SAMPLE(2, "@ResolveServer is vcs; call_id = %s ==> serv = %s (cache:%s)\n", "a_vladimirsafonov84@gmail.com", "a_vladimirsafonov84@gmail.com#vcs", "a_vladimirsafonov84@gmail.com#vcs");
		SAMPLE(3, "@Subscribing %s:%s to %s\n", "ru7.trueconf.net#as", "fh38611@trueconf.com", "support@trueconf.com");
		SAMPLE(4, "@Unsubscribe %s:%s from %s\n", "ru7.trueconf.net#as", "fh38611@trueconf.com", "support@trueconf.com");

		// MULTIGW
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY
#endif
		SAMPLE(5, "NewPeerCfg for %s num=%d\n", "fh38611@trueconf.com", 0);
		SAMPLE(6, "VS_UDPConnRouter::SendFromTo from = %s to %s; data_sz = %d\n", "UDP:192.168.122.1:5060", "UDP:213.145.43.128:5060", 615);
		SAMPLE(7, "VS_UDPConnRouter::HandleUDPWrite sz = %d; writed from = %s to = %s\n", 613, "UDP:192.168.0.111:5060", "UDP:213.145.0.122:5060");
		SAMPLE(8, "VS_UDPConnRouter::HandleRead sz = %d; our_addr = %s; peer addr = %s;\n", 469, "UDP:192.168.122.1:5060", "UDP:213.145.43.128:5060");
		SAMPLE(9, "VS_TransportConnection::Terminate peer addr = %s\n", "UDP:188.138.1.17:5409");
		// TRANSPO
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT
#endif
		SAMPLE(10, "TransportRouter->OnEndpointConnect: endpointID=%s\n", "");
		SAMPLE(11, "DeleteEndpoint: : %s\n", "349F85509027A53451988B4E257590BA");
		SAMPLE(12, "DeleteMsg rs=%s | %s:%s:[%s] ==> %s:%s:[%s] | ttl=%5.5u\n",
			"DeleteEndpoint", "ru7.trueconf.net#as", "", "PRESENCE",
			"yandex.ru#vcs", "", "PRESENCE", 12345);
		SAMPLE(13, "ACS: Zombied Connection. Listening: %s:%s, Incoming: %s:%s.\tUsed Server Resources of %u ms.\n",
			"192.168.122.1", "80", "113.187.17.159", "2515", 1234);
		SAMPLE(14, "EXIT FLAGS: %d\n", 11);
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_MCS
#endif
		SAMPLE(15, "MSC: Method %20s| srv %20s | user %20s\n",
			"Ping", "ru9.trueconf.net#as", "msk_pt@trueconf.com");
		SAMPLE(16, "MSC: Method %20s| srv %20s | user %20s\n",
			"CreateConference", "ru7.trueconf.net#as", "anna_m@trueconf.com");
		SAMPLE(17, "MSC: Method %20s| srv %20s | user %20s\n",
			"Invite", "ru7.trueconf.net#as ", "anna_m@trueconf.com");
		SAMPLE(18, "MSC: Method %20s| srv %20s | user %20s\n",
			"Accept", "ru7.trueconf.net#as", "zik.zak@trueconf.com");
		SAMPLE(19, "MSC: RemovePart_Event %s %d %d \n", "inom4477@trueconf.com", 3, 0);
		// CONF
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_CONFS
#endif
		SAMPLE(20, "KICK: RemoveConfKicks from %s conference\n", "0014abe4@ru7.trueconf.net#as");
		SAMPLE(21, "Invite: Resolving %s(%s): status %d at %s\n", "zik.zak@trueconf.com", "zik.zak@trueconf.com", 1, "ru9.trueconf.net#as");
		SAMPLE(22, "Accept from %s to <%s>\n", "caps@trueconf.com", "0014abc6@ru7.trueconf.net#as");
		SAMPLE(23, "AddParticipant <%s> to <%s> conf\n", "sergeypisanov@trueconf.com", "0014abe9@ru7.trueconf.net#as");
		SAMPLE(24, "RemoveParticipant %s from <%s>\n", "evropeeckremen1@trueconf.com", "0014a9cc@ru7.trueconf.net#as");
		// H323 parser
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER
#endif
		SAMPLE(25, "NewDialogID: %s, sip_to: %s\n", "77543F3DF76D645D65285233135C3447", "#h323:@10.144.53.10");
		SAMPLE(26, "VS_H323Parser::InviteMethod(dialog:%s, from:%s, to:%s)\n", "77543F3DF76D645D65285233135C3447", "it@s1600-vks01.rf.rshbank.ru", "#h323:@10.144.53.10/1A9F9F8E191039514E4ECB5FD21866A2");
		SAMPLE(27, "VS_H323Parser::GetBufForSend: %u bytes\n", 209);
		SAMPLE(28, "VS_H323Parser::SetRecvBuf: %lu bytes from %s\n", 93l, "TCP:10.144.53.10:1720");
		SAMPLE0(29, "VS_H323Parser::OnH245Message: request\n");
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER
#endif
		SAMPLE(30, "NewDialogID: %s, sip_to: %s\n", "15B79139375A5BF4F1DDA4A9F4540B53", "12355861@213.145.43.128");
		SAMPLE(31, "RecvSIPBuf() %d bytes\n%s\n", 469, "SIP/2.0 200 Ok");
		SAMPLE0(32, "VS_SIPParser::OnResponseArrived()\n");
		SAMPLE0(33, "VS_SIPParser::Shutdown()\n");
		SAMPLE(34, "Dialog count: %d\n", 0);
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_LOGSERVICE
#endif
		SAMPLE(35, "LOG: add %s to bs map\n", "bs.trueconf.ru#bs");
		SAMPLE(36, "LOG: Send %s ConfStart %s\n", "bs.trueconf.ru#bs", "0014abbd@ru7.trueconf.net#as");
		SAMPLE(37, "LOG: LogParticipantInvite(%s->%s, %s)\n", "19981204@trueconf.com", "echotest_ru@trueconf.com", "0014abfb@ru7.trueconf.net#as");
		SAMPLE(38, "LOG: LogParticipantJoin(%s:%s, %s)\n", "gennadyshubin@trueconf.com", "ru7.trueconf.net#as", "0014abf9@ru7.trueconf.net#as");
		SAMPLE(39, "LOG: LogParticipantLeave(%s:%s, %s)\n", "evropeeckremen1@trueconf.com", "ru8.trueconf.net#as", "0014a9cc@ru7.trueconf.net#as");
		// default
#ifdef DEBUG_CURRENT_MODULE
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_ALL_
#endif
	default:
		dprint0("Congratulations! You should not have seen this message!\n");
		break;
	}
}

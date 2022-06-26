#pragma once

// flags from here https://msdn.microsoft.com/en-us/library/cc236650.aspx
// all flags are in reverse order i.e 0 is 31, 1 is 30 etc.
#define NTLMSSP_NEGOTIATE_56						(1 << 31)
#define NTLMSSP_NEGOTIATE_KEY_EXCH					(1 << 30)
#define NTLMSSP_NEGOTIATE_128						(1 << 29)
#define NTLMSSP_NEGOTIATE_VERSION					(1 << 25)
#define NTLMSSP_NEGOTIATE_TARGET_INFO				(1 << 23)
#define NTLMSSP_REQUEST_NON_NT_SESSION_KEY			(1 << 22)
#define NTLMSSP_NEGOTIATE_IDENTIFY					(1 << 20)
#define NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY	(1 << 19)
#define NTLMSSP_TARGET_TYPE_SERVER					(1 << 17)
#define NTLMSSP_TARGET_TYPE_DOMAIN					(1 << 16)
#define NTLMSSP_NEGOTIATE_ALWAYS_SIGN				(1 << 15)
#define NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED	(1 << 13)
#define NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED		(1 << 12)
#define NTLMSSP_ANONYMOUS							(1 << 11)
#define NTLMSSP_NEGOTIATE_NTLM						(1 << 9)
#define NTLMSSP_NEGOTIATE_LM_KEY					(1 << 7)
#define NTLMSSP_NEGOTIATE_DATAGRAM					(1 << 6)
#define NTLMSSP_NEGOTIATE_SEAL						(1 << 5)
#define NTLMSSP_NEGOTIATE_SIGN						(1 << 4)
#define NTLMSSP_REQUEST_TARGET						(1 << 2)
#define NTLM_NEGOTIATE_OEM							(1 << 1)
#define NTLMSSP_NEGOTIATE_UNICODE					0x1

// defined here https://msdn.microsoft.com/en-us/library/cc236654.aspx
#define NTLMSSP_REVISION_W2K3	0x0F

// atribute-value types from here https://msdn.microsoft.com/en-us/library/cc236646.aspx
#define MsvChannelBindings	0x000A
#define MsvAvTargetName		0x0009
#define MsvAvTimestamp		0x0007
#define MsvAvFlags			0x0006
#define MsvAvEOL			0x0000
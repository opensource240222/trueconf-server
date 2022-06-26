
#ifndef VC_STD_APPLICATION_INFO_H
#define VC_STD_APPLICATION_INFO_H

static const char REG_CurrentConfiguratuon[]= "Current configuration";
static const char REG_Endpoint[] = "Endpoint";
static const char REG_Servers[]= "Servers";
static const char REG_Server[] = "Server";
static const char REG_DefaultServer[] = "DefaultServer";
static const char REG_Key[] = "Key";
static const char REG_DefaultServerProtocol[] = "TCP";
static const char REG_TempServerName[] = "**#**";
static const char REG_DirectUDP_IPPriority[] = "DirectUDP_IPPriority";


extern void VS_GetAppPorts(unsigned long* &ports, unsigned long &num);
/// Read current server from registry
bool VS_ReadAS(char * server, bool Default = false);
/// Write current server to registry
bool VS_WriteAS(const char * server, bool Default = false);

/// Ser application name and application version
void VS_SetAppVer(const char * app, const char * ver);
extern char  GUI_version[];
extern char  APP_version[];

#endif

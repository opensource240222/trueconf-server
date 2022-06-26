#pragma once

#ifndef _VS_TESTACSCONST_H
#define _VS_TESTACSCONST_H
/* common constants and variables */

#include <winsock2.h>
#include <ntddndis.h>
#include <qos.h>
#include <traffic.h>
#include <MSTcpIP.h>
#include <windows.h>

#include "_testacstypes.h"
#include "_testacsinclude.h"

/* TestApp Extern */
extern const char* _TestServiceNames[];
extern FLOWSPEC default_g711;
extern LPWSAPROTOCOL_INFO QoS_proto;
extern _TestMode AppMode;
extern char GUI_version[];
extern char APP_version[];
extern char myFileName[MAX_PATH + 1];
extern char myFileParm[MAX_PATH + 1];
extern WSADATA wsaData;
extern _RTL_CRITICAL_SECTION gcs;
extern char *m_strSysConfig;
extern char hkey[1024];

/* Visicron Extern */
extern VS_Certificate serverCertificate;
extern char *m_strSysConfig;
extern char hkey[1024];

#endif
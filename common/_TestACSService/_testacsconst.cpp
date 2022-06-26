#include <winsock2.h>
#include <ntddndis.h>
#include <qos.h>
#include <traffic.h>
#include <MSTcpIP.h>
#include <windows.h>

#include "_testacstypes.h"
#include "_testacsinclude.h"

/* TestApp Extern */
char GUI_version[]("_TestACSService+!NA");
char APP_version[]("0.3.0.1");
const char* _TestServiceNames[]= {"UNDEFINED", "TRANSPORT_PING", "VS_PING", "VS_CHAT"};
_TestMode AppMode = {"\0", "\0", 1000, 0, 100, 3, 50, INFINITE, 0, INFINITE, 0, INFINITE, 1, false, false, false, false};
char hkey[1024] = {"\0"};
char myFileName[MAX_PATH + 1] = {"\0"};
char myFileParm[MAX_PATH + 1] = {"\0"};
char *m_strSysConfig(new char[10240]);
WSADATA wsaData = {};
_RTL_CRITICAL_SECTION gcs = {}; // global critical section

/* Visicron Extern */
VS_Certificate serverCertificate;

/* function descriptions */
signed long _TestMode::_TranslateFlood(const char *flood_str) {
	const char *_f1 = strchr(flood_str, '(');
	const char *_f2 = strchr(flood_str, ')');
	const char *_fz = strchr(flood_str, ',');
	if ((_f1) && (_f2) && (_fz)) {
		char *map = (char *)malloc(0xff);
		memset((void *)map, 0, 0xff);
		strncpy_s(map, 0xff, _f1 + 1, _fz - _f1 - 1);
		signed long _tmp = atoi(map);
		if (_tmp) {
			flood_sleep = _tmp;
		} else { free(map); return -1; }
		memset((void *)map, 0, 0xff);
		strncpy_s(map, 0xff, _fz + 1, _f2 - _fz - 1);
		_tmp = atoi(map);
		if (_tmp) {
			flood_mult = _tmp;
		} else { free(map); return -1; }
		free(map);
	} else { return -1; }
	return 0;
};

signed long _TestMode::_TranslateKill(const char *kill_str) {
	const char *_f1 = strchr(kill_str, '(');
	const char *_f2 = strchr(kill_str, ')');
	const char *_fz = strchr(kill_str, ',');
	if ((_f1) && (_f2) && (_fz)) {
		char *map = (char *)malloc(0xff);
		memset((void *)map, 0, 0xff);
		strncpy_s(map, 0xff, _f1 + 1, _fz - _f1 - 1);
		signed long _tmp = atoi(map);
		if (_tmp) {
			kill_sleep = _tmp;
		} else { free(map); return -1; }
		memset((void *)map, 0, 0xff);
		strncpy_s(map, 0xff, _fz + 1, _f2 - _fz - 1);
		_tmp = atoi(map);
		if (_tmp) {
			kill_mult = _tmp;
		} else { free(map); return -1; }
		free(map);
	} else { return -1; }
	return 0;
};

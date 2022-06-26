
#pragma once

class VS_SimpleStr;

bool VS_AutoDiscoveryServers(char* _domain, VS_SimpleStr* &servers, unsigned int &num, bool get_info = false);
const char* VS_DNSGetDefaultService(const char* domain, bool IsOldSRV = false);

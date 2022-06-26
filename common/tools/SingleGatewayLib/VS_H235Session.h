#pragma once

#include "tools/H323Gateway/Lib/h235/VS_H235CryptoEngine.h"
#include "tools/H323Gateway/Lib/src/VS_H235SecurityCapability.h"

#include <unordered_map>
namespace vs
{
class SharedBuffer;
}
class VS_H235Session{
	VS_H235CryptoEngine m_encrContext;
	std::unordered_map<unsigned /*sync_flag*/, VS_H235CryptoEngine> m_decrContexts;

	unsigned m_currSyncFlag = 0;
	int m_recv_payloadType = -1;
public:
	VS_H235Session(const VS_H235SecurityCapability * recv, const VS_H235SecurityCapability * send, const int recvPayload);
	VS_H235Session(){}
	bool InitRecvContext(const VS_H235SecurityCapability * recv, const int recvPayload);
	bool InitGenericContext(const VS_H235SecurityCapability * send);
	VS_H235Session(const VS_H235Session&) = delete;
	VS_H235Session& operator=(const VS_H235Session&) = delete;

	encryption_meta DecryptPacket(vs::SharedBuffer &packet) const;
	vs::SharedBuffer EncryptPacket(const vs::SharedBuffer &packet, encryption_meta &OUTmeta) const;
	bool Valid() const;
	void CloneTo(VS_H235Session &OUT_session) const;
};
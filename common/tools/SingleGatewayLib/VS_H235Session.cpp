#include "VS_H235Session.h"
#include "../../Transcoder/RTPPacket.h"
#include "std-generic/cpplib/SharedBuffer.h"

#include <algorithm>

VS_H235Session::VS_H235Session(const VS_H235SecurityCapability * recv, const VS_H235SecurityCapability * send, const int recvPayload){
	assert(recv != nullptr || send != nullptr);

	if (recv){
		InitRecvContext(recv, recvPayload);
	}

	if (send){
		InitGenericContext(send);
	}
}

bool VS_H235Session::InitRecvContext(const VS_H235SecurityCapability * recv, const int recvPayload){
	if (!recv) return false;

	VS_H235CryptoEngine ctx;
	if (!ctx.Init(recv->h235_sessionKey, recv->m)) return false;

	m_decrContexts[recv->syncFlag] = std::move(ctx);
	m_recv_payloadType = recvPayload;
	return true;
}

bool VS_H235Session::InitGenericContext(const VS_H235SecurityCapability * send){
	if (!send) return false;

	m_currSyncFlag = send->syncFlag;
	return m_encrContext.Init(send->h235_sessionKey, send->m);
}

void Fill_IV(uint8_t *ivSequence, RTPPacket &p){
	assert(ivSequence != nullptr);

	uint16_t seqNo = vs_htons(p.SeqNo());
	memcpy(ivSequence, &seqNo, 2);

	uint32_t timestamp = vs_htonl(p.Timestamp());
	memcpy(ivSequence + 2, &timestamp, 4);
}

encryption_meta VS_H235Session::DecryptPacket(vs::SharedBuffer &packet) const{
	if (m_decrContexts.empty() || packet.size() <= 0) return encryption_meta();

	uint8_t ivSequence[VS_H235CryptoEngine::IV_SEQUENCE_LEN];
	RTPPacket p(packet.data<const void>(), packet.size(),false,false);

	Fill_IV(ivSequence, p);
	auto decrContextIt = m_decrContexts.find(p.PayloadType());
	if(decrContextIt == m_decrContexts.end()) return encryption_meta();
	auto & decrContext = decrContextIt->second;
	if (!decrContext.Valid()) return encryption_meta();

	encryption_meta em;
	std::vector<uint8_t> decr_data;
	encryption_method encr_m = encryption_method::unknown;

	em.succsess = decrContext.Decrypt(p.Data(), p.DataSize(), ivSequence, p.Padding(), decr_data, &encr_m);
	em.encr_m = encr_m;
	if (!em.succsess) return em;

	auto new_size = decr_data.size() + p.HeaderSize();
	if (packet.size() < new_size){
		vs::SharedBuffer new_packet(new_size);
		packet = std::move(new_packet);
	}

    char * pData = packet.data<char>();
    pData[0] &= 0xdf;   // xx0x xxxx                                                        // we return the real length of the decrypted data without padding
                        //   ^ - this bit is zero
    pData[1] =  m_recv_payloadType & 0x7F | pData[1] & 0x80;                                //  restore payload type and marker mxxx xxxx = 0xxx xxxx | m000 0000
	std::memcpy(packet.data<char>() + p.HeaderSize(), decr_data.data(), decr_data.size());	// copy decrypted body
	packet.shrink(0, new_size);

	return em;
}

vs::SharedBuffer VS_H235Session::EncryptPacket(const vs::SharedBuffer &packet, encryption_meta &OUTmeta) const{
	if (!m_encrContext.Valid() || packet.size() <= 0) return vs::SharedBuffer();
	uint8_t ivSequence[VS_H235CryptoEngine::IV_SEQUENCE_LEN];
	RTPPacket p(packet.data<const void>(), packet.size());

	Fill_IV(ivSequence, p);

	encryption_method encr_m = encryption_method::unknown;
	std::vector<uint8_t> encr_data;
	bool res(false), padding_used(false);
	std::tie(res, padding_used) = m_encrContext.Encrypt(p.Data(), p.DataSize(), ivSequence, encr_data,false, &encr_m);
	if (!res) return vs::SharedBuffer();

	vs::SharedBuffer encr_packet(encr_data.size() + p.HeaderSize());
	memcpy(encr_packet.data(), packet.data(), p.HeaderSize());									// copy header
	char * pData = encr_packet.data<char>();
	padding_used ? pData[0] |= 0x20 : pData[0] &= 0xdf;                                         // set padding if used "pData[0] |= 0x20" = xx1x xxx, "Data[0] &= 0xdf" = xx0x xxx
	pData[1] = m_currSyncFlag & 0x7F | pData[1] & 0x80;                                         // set payload=syncFlag and save marker if present: mxxx xxxx = 0xxx xxxx | m000 0000
	std::memcpy(encr_packet.data<char>() + p.HeaderSize(), encr_data.data(), encr_data.size());	// copy encrypted body

	OUTmeta.succsess = true;
	OUTmeta.encr_m = encr_m;
	return encr_packet;
}

void VS_H235Session::CloneTo(VS_H235Session &OUT_session) const{
	auto initAndClone = [&OUT_session](const std::pair< const unsigned, VS_H235CryptoEngine>& it) {
		auto &decrContext = it.second;
		if (decrContext.Valid()) {
			VS_H235CryptoEngine ctx;
			if (ctx.Init(decrContext.GetKey(), decrContext.GetAlgorithm(), decrContext.GetMode())) {
				OUT_session.m_decrContexts[it.first] = std::move(ctx);
			}
		}
	};

	if (m_encrContext.Valid()) OUT_session.m_encrContext.Init(m_encrContext.GetKey(), m_encrContext.GetAlgorithm(), m_encrContext.GetMode());
	std::for_each(m_decrContexts.begin(), m_decrContexts.end(), initAndClone);
}

bool VS_H235Session::Valid() const {
	auto CtxValid = [](const std::pair< const unsigned, VS_H235CryptoEngine>& item) -> bool {
		return item.second.Valid();
	};

	return m_encrContext.Valid() || std::any_of(m_decrContexts.begin(), m_decrContexts.end(), CtxValid);
}

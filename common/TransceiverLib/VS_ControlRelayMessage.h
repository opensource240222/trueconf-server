#pragma once

#include "VS_MainRelayMessage.h"
#include "net/Port.h"
#include "../streams/fwd.h"
#include "std-generic/cpplib/string_view.h"

enum VS_Conference_Type : int;
enum VS_GroupConf_SubType : int;

class VS_ControlRelayMessage : public VS_MainRelayMessage{
public:
	VS_ControlRelayMessage();
	virtual ~VS_ControlRelayMessage();
	bool IsValid() const override;

	bool MakeConferenceStart(const char *conference_name);
	bool MakeConferenceStart(const char *conference_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type);
	bool MakeConferencsStop(const char *conference_name);
	bool MakeParticipantConnect(const char *conference_name, const char *part_name);
	bool MakeParticipantDisconnect(const char *conference_name, const char *part_name);
	bool MakeSetPartCaps(const char* conference_name, const char* participant_name, const void* caps_buf, size_t buf_sz);
	bool MakeTransmitFrame(const char *conference_name, const char *participant_name, const stream::FrameHeader *frame_head, const void *frame_data);
	bool MakeRestrictBitrateSVC(const char* conferenceName, const char* participantName, int32_t v_bitrate, int32_t bitrate, int32_t old_bitrate);
	bool MakeRequestKeyFrame(const char *conferenceName, const char *participantName);
	bool MakeLive555Info(net::port port, string_view secret);

	const char *GetConferenceName() const;
	bool        GetConferenceTypes(VS_Conference_Type &type, VS_GroupConf_SubType &subtype) const;
	const char *GetOwnerName() const;
	const char *GetParticipantName() const;
	const void* GetCaps(size_t& caps_buf_ln) const;
	const stream::FrameHeader *GetFrameHead() const;
	const void *GetFrameData() const;
	bool GetSVCBitrate(int32_t& v_btr, int32_t& btr, int32_t& o_btr) const;
	net::port GetLive555Port() const;
	const char* GetLive555Secret() const;

private:
	unsigned long GetConfNameIndex() const;
	unsigned long GetPartNameIndex() const;
	unsigned long GetCapsLnIndex()	const;
	unsigned long GetCapsBufIndex() const;
	unsigned long GetFrameHeadIndex() const;
	unsigned long GetFrameDataIndex() const;
	unsigned long GetSVCBitrateIndex() const;
	unsigned long GetConfTypesIndex() const;
	unsigned long GetOwnerNameIndex() const;
	unsigned long GetLive555PortIndex() const;
	unsigned long GetLive555SecretIndex() const;
};
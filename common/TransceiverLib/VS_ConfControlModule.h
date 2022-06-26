#pragma once
#include "VS_RelayModule.h"
#include "VS_EnvelopeRelayMessage.h"
#include <boost/make_shared.hpp>
#include <boost/signals2.hpp>
//#include <boost/function.hpp>

class VS_ConfControlMessage;

class VS_ConfControlModule : public VS_RelayModule
{
	typedef boost::signals2::signal<void (const char *, const char *part_name)> KeyFrameReqSig;
	typedef boost::signals2::signal<void (const char *, const char *, const long , const long, const long)>  RestrictBitrateSVCSig;
public:
	VS_ConfControlModule(void);
	~VS_ConfControlModule(void);
	static const char *module_name;

	enum MessageType
	{
		e_UpdateFilter,
		e_SetDisplayName,
		e_KeyFrameReq,
		e_SetBitrate,
		e_UpdateLayout,
		e_SlideCommand,
	};


	// call it on server (or transceiver)
	bool UpdateFilter(const char *conf_name, const char *part_name, const int32_t fltr, const int32_t role);
	bool SetDisplayName(const char *conf_name, const char *part_name, const char *displayname);
	bool KeyFrameReq(const char *conf_name, const char *part_name);
	bool RestrictBitrateSVC(const char *conferenceName, const char *participantName, const int32_t v_bitrate, const int32_t bitrate, const int32_t old_bitrate);
	bool UpdateLayout(const char *conferenceName, const char *layout);
	bool SlideCommand(string_view conf, string_view from, string_view cmd);

	boost::signals2::connection ConnectToKeyFrameReq(const VS_ConfControlModule::KeyFrameReqSig::slot_type &slot )
	{
		return m_fireKeyFrameReq.connect(slot);
	}
	boost::signals2::connection ConnectToRestrictBitrateSig(const VS_ConfControlModule::RestrictBitrateSVCSig::slot_type &slot)
	{
		return m_fireRestrictBitrateSVC.connect(slot);
	}

private:
	// thuse methods are called on transceiver (or server)
	virtual void onUpdateFilter(const char *conf_name, const char *part_name, const long fltr, const int32_t role)
	{}
	virtual void onSetDisplayName(const char *conf_name, const char *part_name, const char *displayname)
	{}
	virtual void onKeyFrameReq(const char *conf_name, const char *part_name)
	{
		m_fireKeyFrameReq(conf_name, part_name);
	}
	virtual void onRestrictBitrateSVC(const char *conferenceName, const char *participantName, const long v_bitrate, const long bitrate, const long old_bitrate)
	{
		m_fireRestrictBitrateSVC(conferenceName, participantName,v_bitrate,bitrate,old_bitrate);
	}
	virtual void onUpdateLayout(const char *conferenceName, const char *layout) {}
	virtual void onSlideCommand(string_view conf, string_view from, string_view cmd) {}

	// internal method
	bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess);

	KeyFrameReqSig			m_fireKeyFrameReq;
	RestrictBitrateSVCSig	m_fireRestrictBitrateSVC;

};







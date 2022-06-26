#include "VS_ConfControlModule.h"
#include <boost/bind.hpp>
#include "std-generic/cpplib/utf8.h"

const char *VS_ConfControlModule::module_name = "VS_ConfControlModule";

// serealize messages
class VS_ConfControlMessage : public VS_EnvelopeRelayMessage
{
public:

	VS_ConfControlMessage() : VS_EnvelopeRelayMessage(VS_ConfControlModule::module_name){}
	~VS_ConfControlMessage(){}

	VS_ConfControlMessage(VS_ConfControlModule::MessageType t) : VS_EnvelopeRelayMessage(VS_ConfControlModule::module_name)
	{
		SetParam("Type", (int32_t)t);
		Make();
	}

	template <class T1>
	VS_ConfControlMessage(VS_ConfControlModule::MessageType t, const T1 &p1) : VS_EnvelopeRelayMessage(VS_ConfControlModule::module_name)
	{
		SetParam("Type", (int32_t)t);
		SetParam("p1", p1);
		Make();
	}

	template <class T1, class T2>
	VS_ConfControlMessage(VS_ConfControlModule::MessageType t, T1 p1, const T2 &p2) : VS_EnvelopeRelayMessage(VS_ConfControlModule::module_name)
	{
		SetParam("Type", (int32_t)t);
		SetParam("p1", p1);
		SetParam("p2", p2);
		Make();
	}

	template <class T1, class T2, class T3>
	VS_ConfControlMessage(VS_ConfControlModule::MessageType t, const T1 &p1, const T2 &p2, const T3 &p3) : VS_EnvelopeRelayMessage(VS_ConfControlModule::module_name)
	{
		SetParam("Type", (int32_t)t);
		SetParam("p1", p1);
		SetParam("p2", p2);
		SetParam("p3", p3);
		Make();
	}
	template <class T1, class T2, class T3, class T4>
	VS_ConfControlMessage(VS_ConfControlModule::MessageType t, const T1 &p1, const T2 &p2, const T3 &p3, const T4 &p4) : VS_EnvelopeRelayMessage(VS_ConfControlModule::module_name)
	{
		SetParam("Type", (int32_t)t);
		SetParam("p1", p1);
		SetParam("p2", p2);
		SetParam("p3", p3);
		SetParam("p4", p4);
		Make();
	}
	template <class T1, class T2, class T3, class T4, class T5>
	VS_ConfControlMessage(VS_ConfControlModule::MessageType t, const T1 &p1, const T2 &p2, const T3 &p3, const T4 &p4, const T5 &p5) : VS_EnvelopeRelayMessage(VS_ConfControlModule::module_name)
	{
		SetParam("Type", (int32_t)t);
		SetParam("p1", p1);
		SetParam("p2", p2);
		SetParam("p3", p3);
		SetParam("p4", p4);
		SetParam("p5", p5);
		Make();
	}
};

VS_ConfControlModule::VS_ConfControlModule(void) : VS_RelayModule( module_name )
{
}

VS_ConfControlModule::~VS_ConfControlModule(void)
{
}

bool VS_ConfControlModule::UpdateFilter(const char *conf_name, const char *part_name, const int32_t fltr, const int32_t role)
{
	return conf_name && part_name && VS_RelayModule::SendMsg(
		boost::shared_ptr<VS_ConfControlMessage>(new VS_ConfControlMessage(e_UpdateFilter, conf_name, part_name, fltr, role))
	);
}

bool VS_ConfControlModule::SetDisplayName(const char *conf_name, const char *part_name, const char *displayname)
{
	return conf_name && part_name && displayname && VS_RelayModule::SendMsg(
		boost::shared_ptr<VS_ConfControlMessage>(new VS_ConfControlMessage(e_SetDisplayName, conf_name, part_name, displayname))
	);
}
bool VS_ConfControlModule::KeyFrameReq(const char *conf_name, const char *part_name)
{
	return conf_name && part_name && VS_RelayModule::SendMsg(
		boost::shared_ptr<VS_ConfControlMessage>(new VS_ConfControlMessage(e_KeyFrameReq,conf_name,part_name))
	);
}
bool VS_ConfControlModule::RestrictBitrateSVC(const char *conferenceName, const char *participantName, const int32_t v_bitrate, const int32_t bitrate, const int32_t old_bitrate)
{
	return conferenceName && participantName && VS_RelayModule::SendMsg(
		boost::shared_ptr<VS_ConfControlMessage>(new VS_ConfControlMessage(e_SetBitrate,conferenceName,participantName,v_bitrate,bitrate,old_bitrate))
	);
}

bool VS_ConfControlModule::UpdateLayout(const char *conferenceName, const char *layout)
{
	return conferenceName && VS_RelayModule::SendMsg(
		boost::shared_ptr<VS_ConfControlMessage>(new VS_ConfControlMessage(e_UpdateLayout, conferenceName, layout))
	);
}

bool VS_ConfControlModule::SlideCommand(string_view conf, string_view from, string_view cmd) {
	return !conf.empty() && VS_RelayModule::SendMsg(
		boost::shared_ptr<VS_ConfControlMessage>(new VS_ConfControlMessage(e_SlideCommand, conf, from, cmd))
	);
}


bool VS_ConfControlModule::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{
	VS_ConfControlMessage m;
	m.SetMessage( mess->GetMess() );
	VS_ConfControlModule::MessageType type;

	{
		int32_t value;
		m.GetParam("Type", value);
		type = static_cast<decltype(type)>(value);
	}

	if (type == VS_ConfControlModule::e_UpdateFilter)
	{
		const char *conf_name = m.GetStrValRef("p1");
		const char *part_name = m.GetStrValRef("p2");
		int32_t fltr(0); m.GetParam("p3", fltr);
		int32_t role(0); m.GetParam("p4", role);
		onUpdateFilter(conf_name, part_name, fltr, role);
	} else
	if (type == VS_ConfControlModule::e_SetDisplayName)
	{
		const char *conf_name = m.GetStrValRef("p1");
		const char *part_name = m.GetStrValRef("p2");
		const char *displayname = m.GetStrValRef("p3");
		onSetDisplayName(conf_name, part_name, displayname);
	} else
	if (VS_ConfControlModule::e_KeyFrameReq == type)
	{
		const char *conf_name = m.GetStrValRef("p1");
		const char *part_name = m.GetStrValRef("p2");
		onKeyFrameReq(conf_name,part_name);
	} else
	if (VS_ConfControlModule::e_SetBitrate == type)
	{
		const char *conf_name = m.GetStrValRef("p1");
		const char *part_name = m.GetStrValRef("p2");
		int32_t v_bitrate(0), bitrate(0), old_bitrate(0);
		m.GetParam("p3",v_bitrate);
		m.GetParam("p4",bitrate);
		m.GetParam("p5",old_bitrate);
		onRestrictBitrateSVC(conf_name,part_name,v_bitrate,bitrate,old_bitrate);
	} else
	if (VS_ConfControlModule::e_UpdateLayout == type)
	{
		const char *conf_name = m.GetStrValRef("p1");
		const char *layout = m.GetStrValRef("p2");
		onUpdateLayout(conf_name, layout);
	}
	else if (VS_ConfControlModule::e_SlideCommand == type) {
		auto conf = m.GetStrValueView("p1");
		auto part = m.GetStrValueView("p2");
		auto cmd = m.GetStrValueView("p3");
		onSlideCommand(conf, part, cmd);
	}
	return true;
}


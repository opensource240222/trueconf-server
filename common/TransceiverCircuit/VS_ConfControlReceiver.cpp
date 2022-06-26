#include "VS_ConfControlReceiver.h"
#include "VS_MediaSourceCollection.h"
#include "VS_RelayMediaSource.h"

VS_ConfControlReceiver::VS_ConfControlReceiver(void)
{
}

VS_ConfControlReceiver::~VS_ConfControlReceiver(void)
{
}

void VS_ConfControlReceiver::SetMediaSoreceCollection(boost::shared_ptr<VS_MediaSourceCollection> &p)
{
	m_collection = p;
}

void VS_ConfControlReceiver::onUpdateFilter(const char *conf_name, const char *part_name, const long fltr, const int32_t role)
{
	if (!m_collection) return;

	auto s = m_collection->GetMediaSource(conf_name, false);
	if (s) s->UpdateFilter(conf_name, part_name, fltr, role);
}

void VS_ConfControlReceiver::onSetDisplayName(const char *conf_name, const char *part_name, const char *displayname)
{
	if (!m_collection) return;

	auto s = m_collection->GetMediaSource(conf_name, false);
	if (s) s->SetParticipantDisplayname(conf_name, part_name, displayname);
}

void VS_ConfControlReceiver::onUpdateLayout(const char * conf_name, const char * layout)
{
	if (!m_collection) return;

	auto s = m_collection->GetMediaSource(conf_name, false);
	if (s) s->SetFixedLayout(layout);
}

void VS_ConfControlReceiver::onSlideCommand(string_view conf, string_view from, string_view cmd)
{
	if (!m_collection) return;

	auto s = m_collection->GetMediaSource(std::string(conf).c_str(), false);
	if (s) s->SetSlideCmd(conf, from, cmd);
}

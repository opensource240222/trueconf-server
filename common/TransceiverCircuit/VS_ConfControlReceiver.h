#pragma once

#include "../TransceiverLib/VS_RelayModule.h"
#include "../TransceiverLib/VS_ConfControlModule.h"

class VS_MediaSourceCollection;

class VS_ConfControlReceiver : public VS_ConfControlModule
{
public:
	VS_ConfControlReceiver(void);
	~VS_ConfControlReceiver(void);

	void SetMediaSoreceCollection(boost::shared_ptr<VS_MediaSourceCollection> &p);

private:
	virtual void onUpdateFilter(const char *conf_name, const char *part_name, const long fltr, const int32_t role);
	virtual void onSetDisplayName(const char *conf_name, const char *part_name, const char *displayname);
	virtual void onUpdateLayout(const char *conf_name, const char *layout);
	virtual void onSlideCommand(string_view conf, string_view from, string_view cmd);

	boost::shared_ptr<VS_MediaSourceCollection> m_collection;
};


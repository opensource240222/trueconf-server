#pragma once
#include "TransceiverLib/VS_RelayModule.h"
#include <string>
#include <map>
#include "std/cpplib/VS_Lock.h"

class VS_MediaSourceCollection;
class VS_RecordConfMediaPeer;

class VS_ConfRecorderModuleReceiver : public VS_RelayModule, VS_Lock
{
public:
	VS_ConfRecorderModuleReceiver(const boost::shared_ptr<VS_MediaSourceCollection> &media_source_collection);
	virtual ~VS_ConfRecorderModuleReceiver(){};

	virtual bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess);

private:
	boost::shared_ptr<VS_MediaSourceCollection> m_media_source_collection;
	void DoStartRecordConference(const char *conf_name, const char *part_name);
	void DoStopRecordConference(const char *conf_name, const char *part_name);
	void DoPauseRecordConference(const char *conf_name, const char *part_name);
	void DoResumeRecordConference(const char *conf_name, const char *part_name);

	std::map<std::tuple<std::string, std::string>, std::shared_ptr<VS_RecordConfMediaPeer>> m_recordConfPeers;
};
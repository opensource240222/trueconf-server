#pragma once

#include <string>
#include <mutex>
#include <boost/signals2.hpp>
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"


class VS_RelayMediaSource;
class VS_FrameReceiverConnector;
class VS_ConfConditionConnector;
class VS_ConfControlModule;

class VS_MediaSourceCollection
{
public:
	VS_MediaSourceCollection(const std::shared_ptr<VS_FrameReceiverConnector> &frame_connector,
										   const std::shared_ptr<VS_ConfConditionConnector> &cond_connector,
										   const std::shared_ptr<VS_ConfControlModule> &confControl);
	bool AddMediaSource(const char *conf_name, const std::shared_ptr<VS_RelayMediaSource> media_source);
	std::shared_ptr<VS_RelayMediaSource>	GetMediaSource(const char* conf_name, bool create_new = true);
	void RemoveMediaSource(const char *conf_name);
private:
	void ReqKeyFrame(const char *conf,const char *part);
	void RestrictBitrateSVC(const char *conferenceName, const char *participantName, const long v_bitrate, const long bitrate, const long old_bitrate);

	typedef std::tuple<
				std::shared_ptr<VS_RelayMediaSource>,	//mediasource
				boost::signals2::scoped_connection,		//keyframereq
				boost::signals2::scoped_connection		//restrictbitrate
				>
				SourceDescrType;
	typedef std::shared_ptr<SourceDescrType> SharedSourceDescrType;

	vs::map<
			std::string,								//conf name
	        SharedSourceDescrType,
			vs::less<>
			>
		m_media_source_storage;

	std::weak_ptr<VS_FrameReceiverConnector>	m_frameConnector;
	std::weak_ptr<VS_ConfConditionConnector>	m_condConnector;

	std::weak_ptr<VS_ConfControlModule>		m_confControlModule;

	std::mutex m_mutex;
};
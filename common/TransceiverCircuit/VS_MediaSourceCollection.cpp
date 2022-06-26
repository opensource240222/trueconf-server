#include "VS_MediaSourceCollection.h"
#include "VS_RelayMediaSource.h"
#include "VS_FrameReceiverConnector.h"
#include "VS_ConfConditionConnector.h"
#include "../TransceiverLib/VS_ConfControlModule.h"
#include "std/cpplib/MakeShared.h"

VS_MediaSourceCollection::VS_MediaSourceCollection(const std::shared_ptr<VS_FrameReceiverConnector> &frame_connector,
												   const std::shared_ptr<VS_ConfConditionConnector> &cond_connector,
												   const std::shared_ptr<VS_ConfControlModule> &confControl) : m_frameConnector(frame_connector),m_condConnector(cond_connector), m_confControlModule(confControl)
{}
bool VS_MediaSourceCollection::AddMediaSource(const char *conf_name, const std::shared_ptr<VS_RelayMediaSource> media_source)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if(m_media_source_storage.find(conf_name) != m_media_source_storage.end())
		return false;
	SharedSourceDescrType sdt(std::make_shared<SourceDescrType>());
	std::get<0>(*sdt) = media_source;
	std::get<1>(*sdt) = media_source->ConnectToKeyFrameReq(boost::bind(&VS_MediaSourceCollection::ReqKeyFrame,this,_1,_2));
	std::get<2>(*sdt) = media_source->ConnectToRestrictBitrateSVC(boost::bind(&VS_MediaSourceCollection::RestrictBitrateSVC,this,_1,_2,_3,_4,_5));
	m_media_source_storage[conf_name] = sdt;
	return true;
}
std::shared_ptr<VS_RelayMediaSource> VS_MediaSourceCollection::GetMediaSource(const char *conf_name, bool create_new)
{
	std::shared_ptr<VS_RelayMediaSource>	res;
	std::unique_lock<std::mutex> lock(m_mutex);
	auto iter = m_media_source_storage.find(conf_name);
	if(iter!=m_media_source_storage.end())
		res = std::get<0>(*iter->second);
	else if (create_new == false)
		res.reset();
	else
	{
		auto  media_source = vs::MakeShared<VS_RelayMediaSource>();
		auto cond_connector = m_condConnector.lock();
		auto frame_connector = m_frameConnector.lock();
		if(cond_connector && frame_connector)
		{
			cond_connector->ConnectToConfControl(conf_name, media_source);
			frame_connector->ConnectToTransmitFrame(conf_name, media_source);
			SharedSourceDescrType sdt(std::make_shared<SourceDescrType>());
			std::get<0>(*sdt) = media_source;
			std::get<1>(*sdt) = media_source->ConnectToKeyFrameReq(boost::bind(&VS_MediaSourceCollection::ReqKeyFrame,this,_1,_2));
			std::get<2>(*sdt) = media_source->ConnectToRestrictBitrateSVC(boost::bind(&VS_MediaSourceCollection::RestrictBitrateSVC,this,_1,_2,_3,_4,_5));
			m_media_source_storage[conf_name] = sdt;
			res = media_source;
		}
	}
	return res;
}
void VS_MediaSourceCollection::RemoveMediaSource(const char *conf_name)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	std::map<std::string,SharedSourceDescrType>::iterator i = m_media_source_storage.find(conf_name);
	if(i!=m_media_source_storage.end())
	{
		auto cond_connector = m_condConnector.lock();
		auto frame_connector = m_frameConnector.lock();
		if(cond_connector && frame_connector)
		{
			cond_connector->DisconnectFromConfControl(conf_name, std::get<0>(*i->second));
			frame_connector->DisconnectFromTransmitFrame(conf_name, std::get<0>(*i->second));
		}
		m_media_source_storage.erase(i);

	}
}
void VS_MediaSourceCollection::ReqKeyFrame(const char *conf, const char *part)
{
	auto lock = m_confControlModule.lock();
	if(lock)
		lock->KeyFrameReq(conf,part);
}
void VS_MediaSourceCollection::RestrictBitrateSVC(const char *conferenceName, const char *participantName, const long v_bitrate, const long bitrate, const long old_bitrate)
{
	auto lock = m_confControlModule.lock();
	if(lock)
		lock->RestrictBitrateSVC(conferenceName,participantName,v_bitrate,bitrate,old_bitrate);
}
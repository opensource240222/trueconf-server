#include "VS_ConfRecorderModuleReceiver.h"
#include "TransceiverLib/VS_ConfRecordRelayMsg.h"
#include "TransceiverLib/VS_ConfRecorderModuleCtrl.h"
#include "VS_MediaSourceCollection.h"
#include "VS_RelayMediaSource.h"
#include "VS_RecordConfMediaPeer.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/utf8_old.h"
#include "std-generic/cpplib/TimeUtils.h"
#include <boost/filesystem.hpp>
#include <boost/smart_ptr/make_shared.hpp>

VS_ConfRecorderModuleReceiver::VS_ConfRecorderModuleReceiver(const boost::shared_ptr<VS_MediaSourceCollection> &media_source_collection) : VS_RelayModule(VS_ConfRecordRelayMsg::module_name),
																																		m_media_source_collection(media_source_collection)
{}

bool VS_ConfRecorderModuleReceiver::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{

	boost::shared_ptr<VS_ConfRecordRelayMsg> conf_write_mess(new VS_ConfRecordRelayMsg);
	if(conf_write_mess->SetMessage(mess->GetMess()))
	{
		switch(conf_write_mess->GetConfWriteMessType())
		{
		case VS_ConfRecordRelayMsg::e_StartRecordConference:
			DoStartRecordConference(conf_write_mess->GetConferenceName(),conf_write_mess->GetPartName());
			break;
		case VS_ConfRecordRelayMsg::e_StopRecordConference:
			DoStopRecordConference(conf_write_mess->GetConferenceName(),conf_write_mess->GetPartName());
			break;
		case VS_ConfRecordRelayMsg::e_PauseRecordConference:
			DoPauseRecordConference(conf_write_mess->GetConferenceName(), conf_write_mess->GetPartName());
			break;
		case VS_ConfRecordRelayMsg::e_ResumeRecordConference:
			DoResumeRecordConference(conf_write_mess->GetConferenceName(), conf_write_mess->GetPartName());
			break;
		}
	}
	/**
		get mediasource
		Create Peer
		Connect Peer to mediasource
	**/
	return true;
}

enum e_ConfRecordFileType{
	CRFT_AVI,
	CRFT_MKV,
	CRFT_MP4
};


void VS_ConfRecorderModuleReceiver::DoStartRecordConference(const char *conf_name, const char *part_name)
{
	auto source = m_media_source_collection->GetMediaSource(conf_name);
	if (!source || !conf_name || !*conf_name)
		return;

	boost::filesystem::path path;
	if (!VS_ConfRecorderModuleCtrl::PrepareRecordingDirectory(path))
		return;

	std::string avi_name;
	std::string full_avi_name = path.string();
	full_avi_name += "/";
	std::string confName(conf_name);
	auto pos = confName.find('@');
	if (pos != std::string::npos)
		confName.erase(pos);
	avi_name += confName;
	avi_name += "_";
	avi_name += tu::TimeToString(std::chrono::system_clock::now(), "%Y-%m-%d_%H-%M-%S", true);

	VS_RegistryKey cfg(false, CONFIGURATION_KEY);

	unsigned long FileType = CRFT_MKV;
	if (cfg.GetValue(&FileType, 4, VS_REG_INTEGER_VT, "ConfRecordFileType") > 0) {
		if (FileType != CRFT_AVI && FileType != CRFT_MKV && FileType != CRFT_MP4)
			FileType = CRFT_MKV;
	}

	VS_MediaFormat mf;
	source->GetPeerMediaFormat(&mf, vs_media_peer_type_record);

	VS_SimpleStr id = "write:";
	switch (FileType)
	{
	case CRFT_AVI:
		id += "avi:";
		avi_name += ".avi";
		mf.SetVideo(mf.dwVideoWidht, mf.dwVideoHeight, VS_VCODEC_VPX);
		mf.SetAudio(16000, VS_ACODEC_PCM);
		break;
	case CRFT_MKV:
		id += "mkv:";
		avi_name += ".mkv";
		mf.SetVideo(mf.dwVideoWidht, mf.dwVideoHeight, VS_VCODEC_VPX);
		mf.SetAudio(16000, VS_ACODEC_PCM);
		break;
	case CRFT_MP4:
		id += "mp4:";
		avi_name += ".mp4";
		mf.SetVideo(mf.dwVideoWidht, mf.dwVideoHeight, VS_VCODEC_H264);
		mf.SetAudio(16000, VS_ACODEC_AAC);
		break;
	}

	VS_SimpleStr cn = conf_name;
	char *c = strchr(cn, '@');
	if (c) *c = 0;
	id += cn;

	auto msg = boost::make_shared<VS_ConfRecordRelayMsg>();
	msg->MakeOnStartRecordConf(conf_name, "", avi_name.c_str(), std::chrono::system_clock::now());
	SendMsg(msg);

	if (!mf.IsVideoValid())
		mf.ReSet();

	full_avi_name += avi_name;
	auto peer = std::make_shared<VS_RecordConfMediaPeer>(id, part_name, full_avi_name.c_str(), &mf);
	{
		VS_AutoLock lock(this);
		m_recordConfPeers[std::make_tuple(conf_name, part_name)] = peer;
	}
	source->PeerConnect(peer);
	peer->ApplyMediaFormat();
}

void VS_ConfRecorderModuleReceiver::DoStopRecordConference(const char *conf_name, const char *part_name)
{
	std::shared_ptr<VS_RecordConfMediaPeer> peer;
	{
		VS_AutoLock lock(this);
		auto i = m_recordConfPeers.find(std::make_tuple(conf_name, part_name));
		if (i == m_recordConfPeers.end())
			return;
		peer = i->second;
		m_recordConfPeers.erase(i);
	}

	auto source = m_media_source_collection->GetMediaSource(conf_name, false);
	if (source)
		source->PeerDisconnect(peer->GetPeerId().c_str());
	peer->Stop();

	auto msg = boost::make_shared<VS_ConfRecordRelayMsg>();
	msg->MakeOnStopRecordConf(conf_name, "", std::chrono::system_clock::now(), peer->GetFileSize());
	SendMsg(msg);
}

void VS_ConfRecorderModuleReceiver::DoPauseRecordConference(const char * conf_name, const char * part_name)
{
	std::shared_ptr<VS_RecordConfMediaPeer> peer;
	{
		VS_AutoLock lock(this);
		auto i = m_recordConfPeers.find(std::make_tuple(conf_name, part_name));
		if (i == m_recordConfPeers.end())
			return;
		peer = i->second;
	}

	peer->Pause();
}

void VS_ConfRecorderModuleReceiver::DoResumeRecordConference(const char * conf_name, const char * part_name)
{
	std::shared_ptr<VS_RecordConfMediaPeer> peer;
	{
		VS_AutoLock lock(this);
		auto i = m_recordConfPeers.find(std::make_tuple(conf_name, part_name));
		if (i == m_recordConfPeers.end())
			return;
		peer = i->second;
	}

	peer->Resume();
}

#include "api/trueconf/configurations.h"

#include "VS_RelayMediaSource.h"
#include "../streams/Protocol.h"
#include "../std/cpplib/VS_Protocol.h"
#include "../std/cpplib/VS_RcvFunc.h"
#include "../std/cpplib/VS_Utils.h"
#include "../std/cpplib/VS_ClientCaps.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_RegistryConst.h"
#include "../std/cpplib/json/writer.h"
#include "../std/cpplib/json/reader.h"
#include "../std/debuglog/VS_Debug.h"
#include "std/Globals.h"
#include "std/VS_TransceiverInfo.h"
#include "std/cpplib/layout_json.h"
#include "../MediaParserLib/VS_VPXParser.h"
#include "../MediaParserLib/VS_H264Parser.h"
#include "Transcoder/LoadBalancing/VS_LoadBalancer.h"
#include "streams/Command.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/clib/vs_time.h"
#include "tools/Server/CommonTypes.h"
#include "tools/SingleGatewayLib/VS_SlideShowDownloader.h"

#include <boost/filesystem/operations.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include <inttypes.h>

#include <cinttypes>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

void FillDate(std::ostream &out, const std::chrono::system_clock::time_point t) {
	const auto unix_tp = std::chrono::system_clock::to_time_t(t);
	tm tp_tm;
	if (gmtime_r(&unix_tp, &tp_tm) == nullptr)
		return;

	out << std::setw(4) << std::setfill('0') << tp_tm.tm_year + 1900 << "/"
		<< std::setw(2) << std::setfill('0') << tp_tm.tm_mon + 1 << "/"
		<< std::setw(2) << std::setfill('0') << tp_tm.tm_mday << " ";
}

void FillTime(std::ostream &out, const std::chrono::system_clock::time_point t) {
	using days = std::chrono::duration<int64_t, std::ratio<3600 * 24>>;

	auto tp = t.time_since_epoch();
	const auto days_c = std::chrono::duration_cast<days>(tp);
	const auto hours = std::chrono::duration_cast<std::chrono::hours>(tp - days_c);
	tp -= days_c;

	const auto min = std::chrono::duration_cast<std::chrono::minutes>(tp - hours);
	tp -= hours;

	const auto sec = std::chrono::duration_cast<std::chrono::seconds>(tp - min);
	tp -= min;

	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp - sec);

	out << std::setw(2) << std::setfill('0') << hours.count() << ":"
		<< std::setw(2) << std::setfill('0') << min.count() << ":"
		<< std::setw(2) << std::setfill('0') << sec.count() << "."
		<< std::setw(3) << std::setfill('0') << ms.count();
}

std::string GetFormatTime(bool withDays)
{
	auto now = std::chrono::system_clock::now();
	std::stringstream timeStr;
	if (withDays) {
		FillDate(timeStr, now);
	}
	FillTime(timeStr, now);
	return timeStr.str();
}

const int Mode2Perf[4][4] =
{
	{ 360,  180,  200, 1},
	{ 640,  360,  420, 2},
    {1280,  720, 1080, 3},
	{1920, 1080, 2240, 4}
};

const float Mode2LayerScales[4][4][2] =
{
	{ { 1.0f, 1.0f },{ 2.0f,  0.4f },{ 2.0f, 0.4f },{ 2.0f, 0.4f } },
	{ { 1.0f, 1.0f },{ 2.0f,  0.4f },{ 2.0f, 0.4f },{ 2.0f, 0.4f } },
	{ { 1.0f, 1.0f },{ 2.0f,  0.4f },{ 2.0f, 0.4f },{ 2.0f, 0.4f } },
	{ { 1.0f, 1.0f },{ 1.5f, 0.48f },{ 2.0f, 0.4f },{ 2.0f, 0.4f } },
};

const uint32_t AudioSamplerate[8] =
{
	8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000
};

VS_RelayMediaSource::VS_RelayMediaSource()
{
	m_svc_mode = 0x00000000;
	m_payloadVideo.clear();
	m_payloadVideo["VP8"] = VS_VCODEC_VPX;
	m_payloadVideo["H264"] = VS_VCODEC_H264;
	m_payloadAudio.clear();
	m_payloadAudio["L16"] = VS_ACODEC_PCM;
	m_payloadAudio["PCMA"] = VS_ACODEC_G711a;
	m_payloadAudio["PCMU"] = VS_ACODEC_G711mu;
	m_payloadAudio["GSM"] = VS_ACODEC_GSM610;
	m_payloadAudio["G723"] = VS_ACODEC_G723;
	m_payloadAudio["G728"] = VS_ACODEC_G728;
	m_payloadAudio["G729"] = VS_ACODEC_G729A;
	m_payloadAudio["G722"] = VS_ACODEC_G722;
	m_payloadAudio["G7221_24"] = VS_ACODEC_G7221_24;
	m_payloadAudio["G7221_32"] = VS_ACODEC_G7221_32;
	m_payloadAudio["speex"] = VS_ACODEC_SPEEX;
	m_payloadAudio["ISAC"] = VS_ACODEC_ISAC;
	m_payloadAudio["G7221C_24"] = VS_ACODEC_G7221C_24;
	m_payloadAudio["G7221C_32"] = VS_ACODEC_G7221C_32;
	m_payloadAudio["G7221C_48"] = VS_ACODEC_G7221C_48;
	m_payloadAudio["opus"] = VS_ACODEC_OPUS_B0914;
	m_payloadAudio["MP3"] = VS_ACODEC_MP3;
	m_payloadAudio["AAC"] = VS_ACODEC_AAC;
	m_logStat = 0;
	m_conf_subtype = GCST_FULL;
	PrepareConference();
}

VS_RelayMediaSource::~VS_RelayMediaSource()
{
	CleanConference();
	if (m_logStat) {
		fclose(m_logStat);
		m_logStat = 0;
	}
}

void VS_RelayMediaSource::PrepareConference()
{
	const int32_t max_quality(video_presets::id(video_presets::mode::fhd));
	int32_t video_quality(max_quality); /// 0 - 180p, 1 - 360p, 2 - 720p, 3 - 1080p
	int32_t record_quality(2); /// from 0 (min quality) to max
	int32_t webrtc_quality(2);
	int32_t rtsp_quality(2);
	int32_t sip_quality(2);
	int32_t video_framerate(15);
	int32_t video_bitrate(0);
	int32_t type_hw_codec(0);
	int32_t enable_hw_lb(0);
	int32_t enable_sw_lb(0);
	int32_t sip_layout(0);
	int32_t load_decompressor(1);
	int32_t enableVadLayouts(0);
	int32_t enableDsLayout(1);

	VS_RegistryKey reg_key(false, CONFIGURATION_KEY);
	reg_key.GetValue(&video_quality, sizeof(video_quality), VS_REG_INTEGER_VT, "mixer video quality");
	reg_key.GetValue(&record_quality, sizeof(record_quality), VS_REG_INTEGER_VT, "record video quality");
	reg_key.GetValue(&webrtc_quality, sizeof(webrtc_quality), VS_REG_INTEGER_VT, "webrtc video quality");
	reg_key.GetValue(&rtsp_quality, sizeof(rtsp_quality), VS_REG_INTEGER_VT, "rtsp video quality");
	reg_key.GetValue(&sip_quality, sizeof(sip_quality), VS_REG_INTEGER_VT, "sip video quality");
	reg_key.GetValue(&video_framerate, sizeof(video_framerate), VS_REG_INTEGER_VT, "mixer max framerate");
	reg_key.GetValue(&video_bitrate, sizeof(video_bitrate), VS_REG_INTEGER_VT, "mixer max bitrate");
	reg_key.GetValue(&type_hw_codec, sizeof(type_hw_codec), VS_REG_INTEGER_VT, "mixer vcodec type");
	reg_key.GetValue(&enable_hw_lb, sizeof(enable_hw_lb), VS_REG_INTEGER_VT, "enable load balancing");
	reg_key.GetValue(&enable_sw_lb, sizeof(enable_sw_lb), VS_REG_INTEGER_VT, "enable cpu balancing");
	reg_key.GetValue(&sip_layout, sizeof(sip_layout), VS_REG_INTEGER_VT, "enable sip layouts");
	reg_key.GetValue(&load_decompressor, sizeof(load_decompressor), VS_REG_INTEGER_VT, "enable load decompressor");
	reg_key.GetValue(&enableVadLayouts, sizeof(enableVadLayouts), VS_REG_INTEGER_VT, "enable vad layouts");
	reg_key.GetValue(&enableDsLayout, sizeof(enableDsLayout), VS_REG_INTEGER_VT, "enable ds layouts");

	m_enableVadLayouts = (enableVadLayouts != 0);
	m_enableDsLayouts = (enableDsLayout != 0);
	m_sip_layout = (sip_layout > 0);
	m_load_decompressor = (load_decompressor > 0);
	int32_t vq(0);
	vq = std::max(vq, record_quality);
	vq = std::max(vq, webrtc_quality);
	vq = std::max(vq, rtsp_quality);
	vq = std::max(vq, sip_quality);
	video_quality = std::min(std::min(vq, max_quality), video_quality);
	video_presets::mode video_mode(static_cast<video_presets::mode>(video_quality));
	int numLayers = video_presets::layers(video_mode);
	record_quality = std::max(0, std::min(record_quality, numLayers - 1));
	webrtc_quality = std::max(0, std::min(webrtc_quality, numLayers - 1));
	rtsp_quality = std::max(0, std::min(rtsp_quality, numLayers - 1));
	sip_quality = std::max(0, std::min(sip_quality, numLayers - 1));
	video_framerate = std::max(1, std::min(video_framerate, 31));
	int width = video_presets::width(video_mode);
	int height = video_presets::height(video_mode);
	int bitrate = video_presets::bitrate(video_mode);
	if (video_bitrate > 0) {
		bitrate = video_bitrate;
	}
	type_hw_codec = std::max(0, type_hw_codec);
	{
		float k = 1.0f;
		int32_t w = width;
		int32_t h = height;
		int32_t mb = 0;
		m_svcProperty.Clear();
		for (int32_t i = 0; i < numLayers; i++) {
			auto ks = Mode2LayerScales[video_quality][i][0];
			auto kb = Mode2LayerScales[video_quality][i][1];
			w = static_cast<int32_t>(static_cast<float>(w) / ks) &~1;
			h = static_cast<int32_t>(static_cast<float>(h) / ks) &~1;
			mb = w * h / 256;
			k *= kb;
			m_svcProperty.mbSpatial[mb].w = w;
			m_svcProperty.mbSpatial[mb].h = h;
			m_svcProperty.mbSpatial[mb].k = k;
			m_svcProperty.mbSpatial[mb].bitrate = static_cast<int32_t>(k * bitrate + 0.5);
			m_svcProperty.sl2mb.push_back(mb);
			{
				VS_MediaFormat mf(m_mf);
				mf.SetVideo(w, h, VS_VCODEC_VPX, video_framerate);
				if (record_quality == (numLayers - 1 - i)) {
					m_moduleFormat[vs_media_peer_type_record] = mf;
				}
				if (webrtc_quality == (numLayers - 1 - i)) {
					m_moduleFormat[vs_media_peer_type_webrtc] = mf;
				}
				if (rtsp_quality == (numLayers - 1 - i)) {
					m_moduleFormat[vs_media_peer_type_rtsp] = mf;
				}
				if (sip_quality == (numLayers - 1 - i)) {
					m_moduleFormat[vs_media_peer_type_mcu] = mf;
				}
			}
		}
		for (int32_t i = 0; i < sizeof(AudioSamplerate) / sizeof(AudioSamplerate[0]); i++) {
			m_svcProperty.srAudio.push_back(AudioSamplerate[i]);
		}
	}
	VS_MediaFormat mf;
	m_conf_bitrate = bitrate;
	m_svc_mode = 0x00000100; /// temporal
	mf.SetAudio(16000, VS_ACODEC_ISAC);
	mf.SetVideo(width, height, VS_VCODEC_VPX, video_framerate, MMFMT_DEFAULT_VIDEOSTEREO, m_svc_mode.load(), type_hw_codec);
	m_mf = mf;
	m_videoQuality = video_quality;
	m_webrtcQuality = webrtc_quality;
	m_rtspQuality = rtsp_quality;
	m_recordQuality = record_quality;
	m_sipQuality = sip_quality;
	LoadBalancingHardware::GetLoadBalancing().Enable(enable_hw_lb, enable_sw_lb);
	m_av_mixer.Init(&mf, m_svcProperty, this);

}

void VS_RelayMediaSource::CleanConference()
{
	m_av_mixer.Release();
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
		m_peerScalablePool.clear();
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		m_partScalablePool.clear();
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerAudioLock);
		m_peerAudioPool.clear();
	}
	{
		VS_AutoLock lock(this);
		m_mediaStreamPool.clear();
	}
	m_svc_mode = 0x00000000;
	m_mf.ReSet();
	m_moduleFormat.clear();
}

void VS_RelayMediaSource::StartConference(const char *conf_name)
{

}

void VS_RelayMediaSource::StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)
{
	m_conf_type = type;
	m_conf_subtype = sub_type;
	m_owner_conf.assign(owner_name);
	{
		char hash[0xff] = { 0 };
		VS_GenKeyByMD5(hash);
		m_podium_parts_hash.assign(hash, std::strlen(hash));
		m_podium_parts_hash += "-<%%>-";
	}
	m_conf = conf_name;

	const auto log_dir = vs::GetLogDirectory() + ts::LOG_DIRECTORY_NAME;
	boost::system::error_code ec;
	boost::filesystem::create_directories(log_dir, ec);
	if (ec) {
		dstream0 << "StartConference: Can't create directory '" << log_dir << "': " << ec.message();
	}

	if (!ec) {
		char name[512] = {0};
		sprintf(name, "%s/vs_stat_relay_%s.txt", log_dir.c_str(), conf_name);
		m_logStat = fopen(name, "a");
		if (m_logStat) {
			std::string str = GetFormatTime(true);
			fprintf(m_logStat, "\n %24s | CREATE CONFERENCE : %s", str.c_str(), conf_name);
			fprintf(m_logStat, "\n %24s | vq = %2d, hw = %2d, max = %4d x %4d @ %2d", "", m_videoQuality, m_mf.dwHWCodec, m_mf.dwVideoWidht, m_mf.dwVideoHeight, m_mf.dwFps);
			{
				const auto &mf_ = m_moduleFormat[vs_media_peer_type_record];
				fprintf(m_logStat, "\n %24s |    rec = %2d, max = %4d x %4d @ %2d", "", m_recordQuality, mf_.dwVideoWidht, mf_.dwVideoHeight, mf_.dwFps);
			}
			{
				const auto &mf_ = m_moduleFormat[vs_media_peer_type_mcu];
				fprintf(m_logStat, "\n %24s |    sip = %2d, max = %4d x %4d @ %2d", "", m_sipQuality, mf_.dwVideoWidht, mf_.dwVideoHeight, mf_.dwFps);
			}
			{
				const auto &mf_ = m_moduleFormat[vs_media_peer_type_rtsp];
				fprintf(m_logStat, "\n %24s |   rtsp = %2d, max = %4d x %4d @ %2d", "", m_rtspQuality, mf_.dwVideoWidht, mf_.dwVideoHeight, mf_.dwFps);
			}
			{
				const auto &mf_ = m_moduleFormat[vs_media_peer_type_webrtc];
				fprintf(m_logStat, "\n %24s | webrtc = %2d, max = %4d x %4d @ %2d", "", m_webrtcQuality, mf_.dwVideoWidht, mf_.dwVideoHeight, mf_.dwFps);
			}
			fprintf(m_logStat, "\n %24s | tl = 0x%08x, sl = ", "", m_mf.dwSVCMode);
			for (const auto & it : m_svcProperty.mbSpatial) {
				fprintf(m_logStat, " [mb = %u, %4d x %4d, btr = %d]", it.first, it.second.w, it.second.h, it.second.bitrate);
			}
		}
	}
}

void VS_RelayMediaSource::StopConference(const char *conf_name)
{
	if (m_logStat) {
		std::string str = GetFormatTime(true);
		fprintf(m_logStat, "\n %24s | KILL CONFERENCE : %s", str.c_str(), conf_name);
	}
	m_conf_type = CT_PRIVATE;
	m_conf_subtype = GCST_FULL;
	m_owner_conf.clear();
	m_conf.clear();
}

VS_Conference_Type VS_RelayMediaSource::GetConfType() const
{
	return m_conf_type;
}

VS_GroupConf_SubType VS_RelayMediaSource::GetConfSubType() const
{
	return m_conf_subtype;
}

std::string VS_RelayMediaSource::GetConfOwner() const
{
	return m_owner_conf;
}

std::string VS_RelayMediaSource::GetPodiumPartsConf() const
{
	return m_podium_parts_hash;
}

void VS_RelayMediaSource::GetPeerMediaFormat(VS_MediaFormat *mf, VS_MediaPeer_Type type) const
{
	auto it = m_moduleFormat.find(type);
	if (it != m_moduleFormat.end()) {
		*mf = it->second;
	}
}

void VS_RelayMediaSource::RestrictMediaFormat(VS_MediaFormat &mf, VS_MediaPeer_Type type) const
{
	auto it = m_moduleFormat.find(type);
	if (it != m_moduleFormat.end()) {
		mf.dwVideoWidht = std::min(mf.dwVideoWidht, it->second.dwVideoWidht);
		mf.dwVideoHeight = std::min(mf.dwVideoHeight, it->second.dwVideoHeight);
	}
}

void VS_RelayMediaSource::ManageLayout(const json::Object & data)
{
	std::uintptr_t handle(0);
	avmuxer::LayoutControl lc;
	if (ParseJsonMessage(data, lc)) {
		if (!lc.toPeer.empty()) {
			std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
			auto lp = m_peerScalablePool.find(lc.toPeer);
			if (lp != m_peerScalablePool.end()) {
				handle = lp->second->handle;
			}
		}
		else {
			handle = GetHandleMixerSymmetric();
		}
	}
	if (handle) {
		m_av_mixer.ManageLayout(handle, lc);
	}
}

void VS_RelayMediaSource::SetFixedLayout(const json::Object & data)
{
	SetFixedLayoutFormat(data);
	if (m_fixedLayoutFormat.layout != avmuxer::VideoLayout::none) {
		ChangePeersLayoutFormat();
	}
}

void VS_RelayMediaSource::SetFixedLayout(const char * layout)
{
	if (!layout)
		return;

	if (!*layout) {
		ResetFixedLayout();
		return;
	}

	json::Object obj;
	std::string	sd(layout);
	std::stringstream ss(sd);
	try {
		json::Reader::Read(obj, ss);
	}
	catch (json::Exception &) {
		ResetFixedLayout();
		return;
	}

	SetFixedLayout(obj);
}

void VS_RelayMediaSource::ResetFixedLayout()
{
	avmuxer::LayoutFormat fixedLayout(m_fixedLayoutFormat);
	ResetLayoutFormat(&m_fixedLayoutFormat);
	if (fixedLayout.layout != avmuxer::VideoLayout::none) {
		VS_AutoLock lock(this);
		mapPeerScalablePool peers;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
			peers = m_peerScalablePool;
		}
		for (auto &it : peers) {
			if (!it.second->lf.mixerDesc.layout.empty()) {
				ChangePeerLayoutFormat(it.first);
			}
		}
	}
}

void VS_RelayMediaSource::SetAsymmetricLayout(const std::set<std::string> &view_peers)
{
	if (view_peers.empty()) {
		return;
	}
	avmuxer::LayoutFormat lf(avmuxer::VideoLayout::special, view_peers);
	{
		VS_AutoLock lock(this);
		m_asymmetricLayoutFormat = lf;
		m_asymmetricLayoutFormat.Hash();
	}
	if (m_fixedLayoutFormat.layout == avmuxer::VideoLayout::none) {
		ChangePeersLayoutFormat();
	}
}

void VS_RelayMediaSource::ResetAsymmetricLayout()
{
	avmuxer::LayoutFormat asymmLayout(m_asymmetricLayoutFormat);
	ResetLayoutFormat(&m_asymmetricLayoutFormat);
	if (m_fixedLayoutFormat.layout != avmuxer::VideoLayout::none) {
		return;
	}
	if (asymmLayout.layout != avmuxer::VideoLayout::none) {
		ChangePeersLayoutFormat();
	}
}

static const boost::regex command_param_re("([^\r\n=]+)=([^\r\n=]+)", boost::regex::optimize);

void VS_RelayMediaSource::SetSlideCmd(string_view conf, string_view from, string_view cmd)
{
	string_view command_v(cmd);

	if (boost::starts_with(command_v, string_view(SHOW_SLIDE_COMMAND)))
	{
		std::string url;
		for (boost::cregex_iterator param_it(command_v.begin() + strlen(SHOW_SLIDE_COMMAND), command_v.end() - strlen(SHOW_SLIDE_COMMAND), command_param_re); param_it != boost::cregex_iterator(); ++param_it)
		{
			if (param_it->empty())
				continue;
			if ((*param_it)[1] == "URL_PARAM")
				url = (*param_it)[2];
		}
		if (url.empty())
			return;
		UpdateSlide(conf, from, url);
	}
	else if (boost::starts_with(command_v, string_view(END_SLIDESHOW_COMMAND)))
		StopSlide(conf, from);

}

inline bool VS_RelayMediaSource::ParticipantPaired(string_view part_name) const
{
	return part_name.find(VS_RCV_FUNC_SEPARATOR) != part_name.npos;
}

void VS_RelayMediaSource::AddParticipantTrack(string_view part_name, stream::Track track, stream::TrackType type)
{
	string_view uniqueId;
	string_view dn;
	if (type == stream::TrackType::slide) {
		uniqueId = relay::default_content_id;
		dn = relay::default_content_prefix_dn;
	}
	m_av_mixer.AddReceiverTrack(part_name, uniqueId, dn, track, type);
}

void VS_RelayMediaSource::RemoveParticipantTrack(string_view part_name, stream::Track track, stream::TrackType type)
{
	string_view uniqueId;
	if (type == stream::TrackType::slide) {
		uniqueId = relay::default_content_id;
	}
	m_av_mixer.RemoveReceiverTrack(part_name, uniqueId, track);
}

void VS_RelayMediaSource::StopSlide(string_view conf_name, string_view part_name)
{
	RemoveParticipantTrack(part_name, relay::default_content_track, stream::TrackType::slide);
}

void VS_RelayMediaSource::UpdateSlide(string_view conf_name, string_view part_name, string_view url)
{
	auto res = vs::download_slide(std::string(url), [weak_self = weak_from_this(),
								  url = std::string(url),
								  conf = std::string(conf_name),
								  part = std::string(part_name)] (std::vector<uint8_t>& image, const size_t w, const size_t h) -> void
	{
		auto self = weak_self.lock();
		if (!self) {
			return;
		}
		if (image.empty()) {
			return;
		}
		else {
			assert(w != 0);
			assert(h != 0);
		}
		self->AddParticipantTrack(part, relay::default_content_track, stream::TrackType::slide);
		uint8_t dim[8];
		*reinterpret_cast<uint32_t*>(&dim[0]) = w;
		*reinterpret_cast<uint32_t*>(&dim[4]) = h;
		auto data = std::move(image);
		data.insert(data.end(), dim, dim + 8);
		self->TransmitFrame(conf.c_str(), part.c_str(), data.data(), data.size(), relay::default_content_track, stream::TrackType::slide);
	});
}

void VS_RelayMediaSource::ParticipantConnect(const char *conf_name, const char *part_name)
{
	if (ParticipantPaired(part_name)) {
		return;
	}
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		fprintf(m_logStat, "\n %13.13s |            connect part | %s ", str.c_str(), part_name);
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		auto part = m_partScalablePool.find(part_name);
		if (part == m_partScalablePool.end()) {
			m_partScalablePool[part_name] = std::make_shared<relay::PartScalablePayload>();
		}
	}
}

void VS_RelayMediaSource::ParticipantDisconnect(const char *conf_name, const char *part_name)
{
	if (ParticipantPaired(part_name)) {
		return;
	}
	m_av_mixer.ReceiverDisconnect(part_name);
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		auto part = m_partScalablePool.find(part_name);
		if (part != m_partScalablePool.end()) {
			m_partScalablePool.erase(part);
		}
	}
	LeavePodiumParticipant(part_name);
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		fprintf(m_logStat, "\n %13.13s |         disconnect part | %s ", str.c_str(), part_name);
	}
}

void VS_RelayMediaSource::GenerateNewPeerId(std::string &peer_id)
{
	while(true)
	{
		char tmp[0xff] = {0};
		VS_GenKeyByMD5(tmp);
		{
			VS_AutoLock lock(this);
			if(m_partsByPeers.find(tmp) == m_partsByPeers.end())
			{
				peer_id = tmp;
				return;
			}
		}
	}
}

void VS_RelayMediaSource::LoggingMixerLayout(const std::uintptr_t handleMixer, const avmuxer::LayoutFormat *lf)
{
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		if (handleMixer == 0) {
			fprintf(m_logStat, "\n %13.13s |         incorrect mixer | %15" PRIxPTR " |", str.c_str(), handleMixer);
		}
		else if (lf != nullptr && lf->layout != avmuxer::VideoLayout::none) {
			std::string desc("symmetric");
			if (lf->layout == avmuxer::VideoLayout::sip) {
				desc = "sip";
				if (!lf->peers.empty()) {
					desc += ", peer = ";
					desc += *lf->peers.begin();
				}
			}
			else if (lf->layout == avmuxer::VideoLayout::special) {
				if (!lf->mixerDesc.layout.empty()) {
					desc = "special fixed";
					desc += ", peers = [";
					for (const auto & p : lf->mixerDesc.layout) {
						if (!p.first.empty()) {
							desc += p.first + " {";
							desc += std::to_string(p.second.rect.offset.x)
								+ " " + std::to_string(p.second.rect.offset.y)
								+ " " + std::to_string(p.second.rect.size.width)
								+ " " + std::to_string(p.second.rect.size.height) + "}; ";
						}
					}
					desc += "]";
				}
				else if (!lf->peers.empty()) {
					desc = "special";
					desc += ", peers = [";
					for (const auto & p : lf->peers) {
						desc += p + "; ";
					}
					desc += "]";
				}
			}
			if (lf->mb > 0) {
				desc += ", force mb = ";
				desc += std::to_string(lf->mb);
			}
			fprintf(m_logStat, "\n %13.13s |            create mixer | %15" PRIxPTR " | %10s ", str.c_str(), handleMixer, desc.c_str());
		}
		else {
			fprintf(m_logStat, "\n %13.13s |           destroy mixer | %15" PRIxPTR " |", str.c_str(), handleMixer);
		}
	}
}

void VS_RelayMediaSource::LoggingVideoLayout(const relay::VideoPayload *mp, const std::uintptr_t handleMixer, uint32_t fourcc)
{
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		if (fourcc != 0) {
			fprintf(m_logStat, "\n %13.13s |       create video pool | %15" PRIxPTR " | mixer = %15" PRIxPTR ", fourcc = %s", str.c_str(), reinterpret_cast<std::uintptr_t>(mp), handleMixer, std::to_string(fourcc).c_str());
		}
		else {
			fprintf(m_logStat, "\n %13.13s |      destroy video pool | %15" PRIxPTR " | mixer = %15" PRIxPTR "", str.c_str(), reinterpret_cast<std::uintptr_t>(mp), handleMixer);
		}
	}
}

std::shared_ptr<VS_MediaPeerBase> VS_RelayMediaSource::GetPeer(const std::string & id)
{
	std::shared_ptr<VS_MediaPeerBase> peer;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
		auto it = m_peerScalablePool.find(id);
		if (it != m_peerScalablePool.end()) {
			peer = it->second->mp;
		}
	}
	return peer;
}

std::shared_ptr<relay::PeerScalablePayload> VS_RelayMediaSource::GetVideoPeer(const std::string & id)
{
	std::shared_ptr<relay::PeerScalablePayload> peer;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
		auto it = m_peerScalablePool.find(id);
		if (it != m_peerScalablePool.end()) {
			peer = it->second;
		}
	}
	return peer;
}

std::shared_ptr<relay::PeerAudioPayload> VS_RelayMediaSource::GetAudioPeer(const std::string & id)
{
	std::shared_ptr<relay::PeerAudioPayload> peer;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerAudioLock);
		auto it = m_peerAudioPool.find(id);
		if (it != m_peerAudioPool.end()) {
			peer = it->second;
		}
	}
	return peer;
}

void VS_RelayMediaSource::RestrictBitrateSVC(const char * conferenceName, const char * participantName, long v_bitrate, long bitrate, long old_bitrate)
{
	std::shared_ptr<VS_MediaPeerBase> peer(GetPeer(participantName));
	if (peer)
		peer->SetPeerBitrate(v_bitrate);
}

void VS_RelayMediaSource::RequestKeyFrame(const char * conferenceName, const char * participantName)
{
	std::shared_ptr<VS_MediaPeerBase> peer(GetPeer(participantName));
	if (peer)
		peer->RequestKeyFrameFromPeer();
}

void VS_RelayMediaSource::RequestKeyFrameFromParticipant(const std::string &id, relay::PartScalablePayload *part_payload)
{
	auto ct = avmuxer::getTickMs();
	if (ct - part_payload->keyt > 2000) {
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |   request key from part | %s ", str.c_str(), id.c_str());
		}
		m_fireKeyFrameReq(m_conf.c_str(), id.c_str());
		part_payload->keyt = ct;
	}
}

void VS_RelayMediaSource::LoggingAudioLayout(const relay::AudioPayload *mp, const std::uintptr_t handleMixer, uint32_t twocc)
{
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		if (twocc != 0) {
			fprintf(m_logStat, "\n %13.13s |       create audio pool | %15" PRIxPTR " | mixer = %15" PRIxPTR ", twocc = %s", str.c_str(), reinterpret_cast<std::uintptr_t>(mp), handleMixer, std::to_string(twocc).c_str());
		}
		else {
			fprintf(m_logStat, "\n %13.13s |      destroy audio pool | %15" PRIxPTR " | mixer = %15" PRIxPTR "", str.c_str(), reinterpret_cast<std::uintptr_t>(mp), handleMixer);
		}
	}
}

bool VS_RelayMediaSource::CreateVideoStreamPeer(std::uintptr_t handle, boost::shared_ptr<relay::VideoPayload> &vp, VS_MediaFormat &mf, const std::shared_ptr<VS_MediaPeerBase> &mediaPeer, const avmuxer::LayoutFormat &lf)
{
	uint32_t correctmb(lf.mb);
	if (m_av_mixer.AddVideoStream(handle, mf, &correctmb)) {
		std::shared_ptr<relay::PeerScalablePayload> peer;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
			auto pl = m_peerScalablePool.find(mediaPeer->GetPeerId());
			if (pl == m_peerScalablePool.end()) {
				const auto &mf = m_moduleFormat[mediaPeer->GetPeerType()];
				auto maxbr = m_svcProperty.mbSpatial[mf.GetFrameSizeMB()].bitrate;
				int32_t ibr(maxbr / 2);
				if (mediaPeer->GetPeerType() == vs_media_peer_type_record ||
					mediaPeer->GetPeerType() == vs_media_peer_type_rtsp ||
					mediaPeer->GetPeerType() == vs_media_peer_type_mcu) {
					ibr = maxbr;
				}
				peer = std::make_shared<relay::PeerScalablePayload>(lf, handle, mf.dwVideoCodecFCC, correctmb, mf, maxbr, ibr);
				peer->mp = mediaPeer;
				m_peerScalablePool[mediaPeer->GetPeerId()] = peer;
			}
			else {
				peer = pl->second;
				peer->mf = mf;
				peer->fourcc = mf.dwVideoCodecFCC;
				peer->mbmax = correctmb;
				peer->mbn = correctmb;
				peer->lf = lf;
			}
		}
		vp->peers.push_back(peer);
		vp->mbPeers[correctmb]++;
		/// correct out video format
		auto it = m_svcProperty.mbSpatial.find(correctmb);
		if (it != m_svcProperty.mbSpatial.end()) {
			mf.SetVideo(static_cast<uint32_t>(it->second.w),
						static_cast<uint32_t>(it->second.h),
						mf.dwVideoCodecFCC);
		}
		return true;
	}
	return false;
}

bool VS_RelayMediaSource::CreateAudioStreamPeer(std::uintptr_t handle, boost::shared_ptr<relay::AudioPayload> &ap, VS_MediaFormat &mf, const std::shared_ptr<VS_MediaPeerBase> &mediaPeer, const avmuxer::LayoutFormat &lf)
{
	if (m_av_mixer.AddAudioStream(handle, mf)) {
		std::shared_ptr<relay::PeerAudioPayload> peer;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxPeerAudioLock);
			auto pl = m_peerAudioPool.find(mediaPeer->GetPeerId());
			if (pl == m_peerAudioPool.end()) {
				peer = std::make_shared<relay::PeerAudioPayload>(lf, handle, mf.dwAudioCodecTag, mf.dwAudioSampleRate, mf);
				peer->mp = mediaPeer;
				m_peerAudioPool[mediaPeer->GetPeerId()] = peer;
			}
			else {
				peer = pl->second;
				peer->mf = mf;
				peer->twocc = mf.dwAudioCodecTag;
				peer->samplerate = mf.dwAudioSampleRate;
				peer->lf = lf;
			}
		}
		ap->peers.push_back(peer);
		ap->srPeers[mf.dwAudioSampleRate]++;
		return true;
	}
	return false;
}

boost::shared_ptr<relay::VideoPayload> VS_RelayMediaSource::CreateVideoPayloadPeer(std::uintptr_t handle, uint32_t fourcc, std::map<uint32_t, boost::shared_ptr<relay::VideoPayload>> &vp, const avmuxer::LayoutFormat &lf)
{
	boost::shared_ptr<relay::VideoPayload> mp;
	auto cdc = vp.find(fourcc);
	if (cdc == vp.end()) {
		int32_t numtl(0);
		if (m_av_mixer.CreateVideoPool(handle, fourcc, &numtl)) {
			mp = boost::make_shared<relay::VideoPayload>();
			mp->realMixerBitrate = m_conf_bitrate;
			mp->numtl = numtl;
			if (lf.mb == 0) {
				mp->numsl = m_svcProperty.mbSpatial.size();
				for (const auto & it : m_svcProperty.mbSpatial) {
					mp->layerSpatialSize[it.first] = 0;
				}
			}
			else {
				mp->numsl = 1;
				mp->layerSpatialSize[lf.mb] = 0;
			}
			vp[fourcc] = mp;
			LoggingVideoLayout(mp.get(), handle, fourcc);
			UpdateLayersCoefs(mp.get());
		}
	}
	else {
		mp = cdc->second;
	}
	return mp;
}

boost::shared_ptr<relay::AudioPayload> VS_RelayMediaSource::CreateAudioPayloadPeer(std::uintptr_t handle, uint32_t twocc, std::map<uint32_t, boost::shared_ptr<relay::AudioPayload>> &ap, const avmuxer::LayoutFormat &lf)
{
	boost::shared_ptr<relay::AudioPayload> mp;
	auto cdc = ap.find(twocc);
	if (cdc == ap.end()) {
		if (m_av_mixer.CreateAudioPool(handle, twocc)) {
			mp = boost::make_shared<relay::AudioPayload>();
			ap[twocc] = mp;
			LoggingAudioLayout(mp.get(), handle, twocc);
		}
	}
	else {
		mp = cdc->second;
	}
	return mp;
}

boost::shared_ptr<relay::MediaPayload> VS_RelayMediaSource::CreateMixer(const avmuxer::LayoutFormat & lf, std::uintptr_t &handle)
{
	auto mp = m_mediaStreamPool.find(lf);
	if (mp == m_mediaStreamPool.end()) {
		m_mediaStreamPool[lf] = boost::make_shared<relay::MediaPayload>();
		mp = m_mediaStreamPool.find(lf);
		handle = reinterpret_cast<std::uintptr_t>(mp->second.get());
		if (!m_av_mixer.CreateMixer(handle, lf)) {
			handle = 0;
		}
		LoggingMixerLayout(handle, &lf);
	}
	else {
		handle = reinterpret_cast<std::uintptr_t>(mp->second.get());
	}
	return mp->second;
}

std::uintptr_t VS_RelayMediaSource::GetHandleMixerSymmetric()
{
	std::uintptr_t handle(0);
	avmuxer::LayoutFormat lf(avmuxer::VideoLayout::symmetric, {});
	{
		VS_AutoLock lock(this);
		auto mp = m_mediaStreamPool.find(lf);
		if (mp != m_mediaStreamPool.end()) {
			handle = reinterpret_cast<std::uintptr_t>(mp->second.get());
		}
	}
	return handle;
}

void VS_RelayMediaSource::GetHandleMixersPriority(const std::string &idPriority, std::vector<std::uintptr_t> & handleMixers)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
	for (const auto &it : m_peerScalablePool) {
		const auto &lf = it.second->lf;
		if ((lf.layout == avmuxer::VideoLayout::special) && !lf.mixerDesc.layout.empty()) {
			/// fixed layout
			continue;
		}
		if (lf.layout == avmuxer::VideoLayout::sip && it.first == idPriority) {
			/// sip layout
			continue;
		}
		handleMixers.push_back(it.second->handle);
	}
}

std::shared_ptr<relay::PeerScalablePayload> VS_RelayMediaSource::RemoveVideoPeerPayload(const char *peer_id, uint32_t fourcc, uint8_t &op)
{
	op = relay::MixerOp::none;
	std::shared_ptr<relay::PeerScalablePayload> peer;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
		auto lp = m_peerScalablePool.find(peer_id);
		if (lp != m_peerScalablePool.end()) {
			peer = lp->second;
			if (fourcc == 0) {
				m_peerScalablePool.erase(lp);
			}
		}
	}
	if (peer.get()) {
		auto mp = m_mediaStreamPool.find(peer->lf);
		if (mp != m_mediaStreamPool.end()) {
			auto cdc = mp->second->videoPayload.find(peer->fourcc);
			if (cdc != mp->second->videoPayload.end()) {
				cdc->second->peers.remove(peer);
				assert(cdc->second->mbPeers[peer->mbn] > 0);
				cdc->second->mbPeers[peer->mbn]--;
				if (cdc->second->mbPeers[peer->mbn] <= 0) {
					op |= relay::MixerOp::remove_layer;
				}
				if (peer->fourcc != fourcc) {
					if (cdc->second->peers.empty()) {
						LoggingVideoLayout(cdc->second.get(), peer->handle, 0);
						mp->second->videoPayload.erase(cdc);
						op |= relay::MixerOp::remove_pool;
					}
					if (fourcc == 0) {
						if (mp->second->videoPayload.empty() && mp->second->audioPayload.empty()) {
							LoggingMixerLayout(peer->handle, nullptr);
							m_mediaStreamPool.erase(mp);
							op |= relay::MixerOp::remove_mixer;
						}
					}
				}
				op |= relay::MixerOp::remove_stream;
			}
		}
	}
	return peer;
}

std::shared_ptr<relay::PeerAudioPayload> VS_RelayMediaSource::RemoveAudioPeerPayload(const char *peer_id, uint32_t twocc, uint8_t &op)
{
	op = relay::MixerOp::none;
	std::shared_ptr<relay::PeerAudioPayload> peer;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerAudioLock);
		auto lp = m_peerAudioPool.find(peer_id);
		if (lp != m_peerAudioPool.end()) {
			peer = lp->second;
			if (twocc == 0) {
				m_peerAudioPool.erase(lp);
			}
		}
	}
	if (peer.get()) {
		auto mp = m_mediaStreamPool.find(peer->lf);
		if (mp != m_mediaStreamPool.end()) {
			auto cdc = mp->second->audioPayload.find(peer->twocc);
			if (cdc != mp->second->audioPayload.end()) {
				cdc->second->peers.remove(peer);
				assert(cdc->second->srPeers[peer->samplerate] > 0);
				cdc->second->srPeers[peer->samplerate]--;
				if (cdc->second->srPeers[peer->samplerate] <= 0) {
					op |= relay::MixerOp::remove_layer;
				}
				if (peer->twocc != twocc) {
					if (cdc->second->peers.empty()) {
						LoggingAudioLayout(cdc->second.get(), peer->handle, 0);
						mp->second->audioPayload.erase(cdc);
						op |= relay::MixerOp::remove_pool;
					}
					if (twocc == 0) {
						if (mp->second->videoPayload.empty() && mp->second->audioPayload.empty()) {
							LoggingMixerLayout(peer->handle, nullptr);
							m_mediaStreamPool.erase(mp);
							op |= relay::MixerOp::remove_mixer;
						}
					}
				}
				op |= relay::MixerOp::remove_stream;
			}
		}
	}
	return peer;
}

void VS_RelayMediaSource::MixerVideoPoolClean(const uint8_t &op, std::uintptr_t handle, uint32_t fourcc, uint32_t mb)
{
	if (op & relay::MixerOp::remove_layer) {
		m_av_mixer.EnableVideoLayer(false, handle, fourcc, mb);
	}
	if (op & relay::MixerOp::remove_stream) {
		m_av_mixer.RemoveVideoStream(handle, fourcc, mb);
	}
	if (op & relay::MixerOp::remove_pool) {
		m_av_mixer.DestroyVideoPool(handle, fourcc);
	}
	if (op & relay::MixerOp::remove_mixer) {
		m_av_mixer.DestroyMixer(handle);
	}
}

void VS_RelayMediaSource::MixerAudioPoolClean(const uint8_t &op, std::uintptr_t handle, uint32_t twocc, uint32_t samplerate)
{
	if (op & relay::MixerOp::remove_layer) {
		m_av_mixer.EnableAudioLayer(false, handle, twocc, samplerate);
	}
	if (op & relay::MixerOp::remove_stream) {
		m_av_mixer.RemoveAudioStream(handle, twocc, samplerate);
	}
	if (op & relay::MixerOp::remove_pool) {
		m_av_mixer.DestroyAudioPool(handle, twocc);
	}
	if (op & relay::MixerOp::remove_mixer) {
		m_av_mixer.DestroyMixer(handle);
	}
}

avmuxer::LayoutFormat VS_RelayMediaSource::GetVideoLayoutFormat(VS_MediaPeerBase *peer, int32_t forceWidth, int32_t forceHeight)
{
	avmuxer::LayoutFormat lf(avmuxer::VideoLayout::symmetric, {});
	if (m_fixedLayoutFormat.layout != avmuxer::VideoLayout::none) {
		lf = m_fixedLayoutFormat;
	}
	else {
		if (peer->GetPeerType() == vs_media_peer_type_record) {

		}
		else if (peer->GetPeerType() == vs_media_peer_type_rtsp) {
			if (peer->GetPartId() == m_podium_parts_hash) {
				if (m_asymmetricLayoutFormat.layout != avmuxer::VideoLayout::none) {
					lf = m_asymmetricLayoutFormat;
				}
			}
			else if (!peer->GetPartId().empty()) {
				lf.layout = avmuxer::VideoLayout::special;
				lf.peers.insert(peer->GetPartId());
			}
			else {

			}
		}
		else {
			bool onPodium = CheckPeerPodium(peer->GetPeerId());
			if (m_asymmetricLayoutFormat.layout != avmuxer::VideoLayout::none) {
				auto it = m_asymmetricLayoutFormat.peers.find(peer->GetPeerId());
				if (it == m_asymmetricLayoutFormat.peers.end()) {
					lf = m_asymmetricLayoutFormat;
				}
			}
			else if (!peer->GetPartId().empty()) {
				lf.layout = avmuxer::VideoLayout::special;
				lf.peers.insert(peer->GetPartId());
			}
			else if (onPodium && m_sip_layout && peer->GetPeerType() == vs_media_peer_type_mcu) {
				lf.layout = avmuxer::VideoLayout::sip;
				lf.peers.insert(peer->GetPeerId());
			}
			else {

			}
		}
	}
	if (forceWidth != 0 && forceHeight != 0) {
		/// for forced resolution we create a separate mixer,
		/// due to differences in aspect ratio.
		lf.width = forceWidth;
		lf.height = forceHeight;
		lf.mb = forceWidth * forceHeight / 256;
	}
	lf.Hash();
	return lf;
}

avmuxer::LayoutFormat VS_RelayMediaSource::GetAudioLayoutFormat(VS_MediaPeerBase *peer)
{
	avmuxer::LayoutFormat lf(avmuxer::VideoLayout::symmetric, {});
	if (peer->GetPeerType() == vs_media_peer_type_record) {

	}
	else {
		bool onPodium = CheckPeerPodium(peer->GetPeerId());
		if (m_fixedLayoutFormat.layout != avmuxer::VideoLayout::none) {
			if (onPodium) {
				lf.layout = avmuxer::VideoLayout::sip;
				lf.peers.insert(peer->GetPeerId());
			}
		}
		else if (peer->GetPeerType() == vs_media_peer_type_rtsp) {
			if (peer->GetPartId() == m_podium_parts_hash) {
				if (m_asymmetricLayoutFormat.layout != avmuxer::VideoLayout::none) {
					lf = m_asymmetricLayoutFormat;
				}
			}
			else if (!peer->GetPartId().empty()) {
				lf.layout = avmuxer::VideoLayout::special;
				lf.peers.insert(peer->GetPartId());
			}
			else {

			}
		}
		else {
			if (m_asymmetricLayoutFormat.layout != avmuxer::VideoLayout::none) {
				auto it = m_asymmetricLayoutFormat.peers.find(peer->GetPeerId());
				if (it == m_asymmetricLayoutFormat.peers.end()) {
					lf = m_asymmetricLayoutFormat;
				}
				else {
					lf.layout = avmuxer::VideoLayout::sip;
					lf.peers.insert(peer->GetPeerId());
				}
			}
			else if (!peer->GetPartId().empty()) {
				lf.layout = avmuxer::VideoLayout::special;
				if (peer->GetPartId() != peer->GetPeerId()) {
					lf.peers.insert(peer->GetPartId());
				}
			}
			else if (onPodium) {
				lf.layout = avmuxer::VideoLayout::sip;
				lf.peers.insert(peer->GetPeerId());
			}
		}
	}
	lf.Hash();
	return lf;
}

bool VS_RelayMediaSource::PeerConnect(const std::shared_ptr<VS_MediaPeerBase> &peer)
{
	if (!peer) {
		return false;
	}
	VS_AutoLock lock(this);
	if (m_partsByPeers.find(peer->GetPeerId()) != m_partsByPeers.end()) {
		return false;
	}
	std::shared_ptr<VS_ParticipantPoolItem> part;
	auto it = m_part_pool.find(peer->GetPartId());
	if (it == m_part_pool.end()) {
		std::shared_ptr<VS_ParticipantPoolItem> ptr(new VS_ParticipantPoolItem);
		ptr->part_id = peer->GetPartId();
		m_part_pool[peer->GetPartId()] = ptr;
		part = m_part_pool.find(peer->GetPartId())->second;
	}
	else {
		part = it->second;
	}
	if (!part) {
		return false;
	}
	if (part->peers.find(peer->GetPeerId()) != part->peers.end()) {
		return false;
	}
	peer->SetObserver(this);
	part->peers[peer->GetPeerId()] = peer;
	m_partsByPeers[peer->GetPeerId()] = peer->GetPartId();
	/// prepare video
	VS_MediaFormat mf = m_moduleFormat[peer->GetPeerType()];
	std::uintptr_t handle(0);
	uint32_t fourcc = m_mf.dwVideoCodecFCC;
	auto lf = GetVideoLayoutFormat(peer.get(), 0, 0);
	auto mp = CreateMixer(lf, handle);
	if (!handle) {
		return false;
	}
	auto vp = CreateVideoPayloadPeer(handle, fourcc, mp->videoPayload, lf);
	if (!vp) {
		return false;
	}
	CreateVideoStreamPeer(handle, vp, mf, peer, lf);
	/// prepare audio
	uint32_t twocc = m_mf.dwAudioCodecTag;
	lf = GetAudioLayoutFormat(peer.get());
	mp = CreateMixer(lf, handle);
	if (!handle) {
		return false;
	}
	auto ap = CreateAudioPayloadPeer(handle, twocc, mp->audioPayload, lf);
	if (!ap) {
		return false;
	}
	CreateAudioStreamPeer(handle, ap, m_mf, peer, lf);
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		fprintf(m_logStat, "\n %13.13s |            connect peer | %15.15s | video pool = %15" PRIxPTR ", audio pool = %15" PRIxPTR "", str.c_str(), peer->GetPeerId().c_str(), reinterpret_cast<std::uintptr_t>(vp.get()), reinterpret_cast<std::uintptr_t>(ap.get()));
		fflush(m_logStat);
	}
	return true;
}


void VS_RelayMediaSource::PeerDisconnect(const char *peer_name)
{
	VS_AutoLock lock(this);
	{
		/// destroy video payloads
		uint8_t op(relay::MixerOp::none);
		auto peer = RemoveVideoPeerPayload(peer_name, 0, op);
		if (peer.get()) {
			if (peer->mp.get()) {
				peer->mp->Close();
			}
			MixerVideoPoolClean(op, peer->handle, peer->fourcc, peer->mbn);
		}
	}
	{
		/// destroy audio payloads
		uint8_t op(relay::MixerOp::none);
		auto peer = RemoveAudioPeerPayload(peer_name, 0, op);
		if (peer.get()) {
			if (peer->mp.get()) {
				peer->mp->Close();
			}
			MixerAudioPoolClean(op, peer->handle, peer->twocc, peer->samplerate);
		}
	}
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		fprintf(m_logStat, "\n %13.13s |         disconnect peer | %s", str.c_str(), peer_name);
	}
}

unsigned long GetCapsFourcc(const VS_ClientCaps & caps)
{
	unsigned long nmy(9);
	size_t nfrom(0);
	caps.GetVideoCodecs(0, nfrom);
	if (nfrom <= 0) {
		return 0;
	}
	unsigned long capsFourcc(0);
	auto pFourcc = std::make_unique<uint32_t[]>(nfrom);
	caps.GetVideoCodecs(pFourcc.get(), nfrom);
	for (unsigned long imy = 0; imy < nmy; imy++) {
		for (unsigned long ifrom = 0; ifrom < nfrom; ifrom++) {
			if (pFourcc[ifrom] == VS_EnumVCodecs[imy]) {
				capsFourcc |= (1 << imy);
			}
		}
	}
	return capsFourcc;
}

void VS_RelayMediaSource::SetParticipantCaps(const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long buf_sz)
{
	int32_t svc(0);
	VS_MediaFormat fmt;
	VS_ClientCaps cc(caps_buf, buf_sz);
	cc.GetMediaFormat(fmt);
	if (m_conf_type != 0) {
		svc = cc.GetStreamsDC() & VSCC_STREAM_CAN_USE_SVC;
		if (svc != 0) {
			fmt.dwSVCMode = 0x00070100;
		}
	}
	unsigned long capsFourcc = GetCapsFourcc(cc);
	m_av_mixer.ReceiverInit(part_name, &fmt, capsFourcc, part_name);
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		auto it = m_partScalablePool.find(part_name);
		if (it != m_partScalablePool.end()) {
			it->second->use_svc.store(svc != 0);
		}
	}
	if (m_conf_type == VS_Conference_Type::CT_PRIVATE) {
		m_av_mixer.ReceiverSetActive(part_name, true);
	}
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		std::string fourcc = std::to_string(fmt.dwVideoCodecFCC);
		fprintf(m_logStat, "\n %13.13s |           set caps part | %15.15s | %4d x %4d, fourcc = %s, svc = %d", str.c_str(), part_name, fmt.dwVideoWidht, fmt.dwVideoHeight, fourcc.c_str(), svc);
	}
}


void VS_RelayMediaSource::UpdateFilter(const char *conf_name, const char *part_name, const long fltr, const int32_t role)
{
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		fprintf(m_logStat, "\n %13.13s |      update filter part | %15.15s | fltr = %d", str.c_str(), part_name, static_cast<int32_t>(fltr));
	}
	std::set<std::string> leaders;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		auto it = m_partScalablePool.find(part_name);
		if (it != m_partScalablePool.end()) {
			it->second->role = role;
		}
		if (m_conf_subtype == GCST_ALL_TO_OWNER)
			for (auto &l : m_partScalablePool)
				if (l.second->role == PR_LEADER)
					leaders.insert(l.first);
	}

	m_av_mixer.ReceiverSetActive(part_name, fltr!=0);

	if (leaders.size() > 0)
		SetAsymmetricLayout(leaders);

	if ((fltr & VS_RcvFunc::FLTR_RCV_VIDEO) == 0) {
		LeavePodiumParticipant(part_name);
	}
	else {
		TakePodiumParticipant(part_name);
	}
}

void VS_RelayMediaSource::SetParticipantDisplayname(const char *conf_name, const char *part_name, const char *displayname)
{
	if (displayname == nullptr) displayname = "";
	if (m_logStat) {
		std::string str = GetFormatTime(false);
		fprintf(m_logStat, "\n %13.13s |         set displayname | %15.15s | %s", str.c_str(), part_name, displayname);
	}
	m_av_mixer.ReceiverChangeName(part_name, displayname);
}

void VS_RelayMediaSource::TakePodiumParticipant(const std::string & id)
{
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		auto it = m_partScalablePool.find(id);
		if (it != m_partScalablePool.end()) {
			it->second->podium.store(true);
		}
	}
	{
		VS_AutoLock lock(this);
		ChangePeerLayoutFormat(id);
	}
}

void VS_RelayMediaSource::LeavePodiumParticipant(const std::string & id)
{
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		auto it = m_partScalablePool.find(id);
		if (it != m_partScalablePool.end()) {
			it->second->podium.store(false);
		}
	}
	pairForceLayout fl;
	{
		VS_AutoLock lock(this);
		ChangePeerLayoutFormat(id);
		fl = RemoveActiveDesktopCapture(id.c_str());
	}
	ChangePriority(fl);
}

bool VS_RelayMediaSource::CheckPeerPodium(const std::string & id)
{
	bool onPodium(false);
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		auto it = m_partScalablePool.find(id);
		if (it != m_partScalablePool.end()) {
			onPodium = it->second->podium.load();
		}
	}
	return onPodium;
}

pairForceLayout VS_RelayMediaSource::GetActiveDesktopCapture(const char *part_name, relay::PartScalablePayload *part_payload, int sl)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxActiveDesktop);
	pairForceLayout fl;
	fl.second = false;
	fl.first.clear();
	if (sl == 0) {
		std::string op("");
		bool ds = (part_payload->max_mb > 3600);
		auto itds = std::find_if(m_mapPartDS.begin(), m_mapPartDS.end(), [part_name](const std::pair<uint32_t, std::string> &pair) -> bool { return (strcmp(pair.second.c_str(), part_name) == 0); });
		if (ds) {
			if (itds == m_mapPartDS.end() || itds->first != part_payload->max_mb) {
				if (itds != m_mapPartDS.end()) {
					op = "--++";
					m_mapPartDS.erase(itds);
				}
				else {
					op = "++++";
				}
				m_mapPartDS.emplace(part_payload->max_mb, part_name);
			}
		}
		else {
			if (itds != m_mapPartDS.end()) {
				op = "----";
				m_mapPartDS.erase(itds);
			}
		}
		if (!op.empty()) {
			if (m_logStat) {
				std::string str = GetFormatTime(false);
				fprintf(m_logStat, "\n %13.13s |		 desktop capture | %15.15s | %s, mb = %d", str.c_str(), part_name, op.c_str(), part_payload->max_mb);
			}
			fl.second = true;
		}
	}
	fl.first = (m_mapPartDS.empty()) ? "" : m_mapPartDS.crbegin()->second;
	if (m_logStat) {
		if (fl.second && !fl.first.empty()) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |		 desktop capture | %15.15s | active", str.c_str(), fl.first.c_str());
		}
	}
	return fl;
}

pairForceLayout VS_RelayMediaSource::RemoveActiveDesktopCapture(const char *part_name)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxActiveDesktop);
	pairForceLayout fl;
	fl.second = false;
	fl.first.clear();
	auto itds = std::find_if(m_mapPartDS.begin(), m_mapPartDS.end(), [part_name](const std::pair<uint32_t, std::string> &pair) -> bool { return (strcmp(pair.second.c_str(), part_name) == 0); });
	if (itds != m_mapPartDS.end()) {
		m_mapPartDS.erase(itds);
		fl.first = (m_mapPartDS.empty()) ? "" : m_mapPartDS.crbegin()->second;
		fl.second = true;
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |		 desktop capture | %15.15s | leave podium", str.c_str(), part_name);
			if (!fl.first.empty()) {
				fprintf(m_logStat, "\n %13.13s |		 desktop capture | %15.15s | active", str.c_str(), fl.first.c_str());
			}
		}
	}
	return fl;
}

int VS_RelayMediaSource::GetOptimalSpatialLayer(relay::PartScalablePayload *part_payload, uint8_t *pack, int size, const char *part_name, bool &key, bool &key_request, pairForceLayout &fl)
{
	int32_t sl(0);
	bool use_svc(part_payload->use_svc.load());
	int32_t load_sl(part_payload->load_sl.load());
	int32_t mixer_mb(part_payload->mixer_mb.load());
	int32_t mixer_sl(part_payload->mixer_sl.load());
	int32_t num_sl(part_payload->num_sl.load());
	key = false;
	fl.second = false;
	if (use_svc) {
		/// parse for key frame
		stream::SliceHeader* sh = reinterpret_cast<stream::SliceHeader*>(pack + size - sizeof(stream::SliceHeader) - sizeof(stream::SVCHeader));
		if (sh->id == sh->first_id) {
			key = pack[0] != 0;
		}
		sl = (0x30 & pack[size - 1]) >> 4;
		if (key) {
			int numsl = 0x03 & pack[size - 1];
			//{
			//	std::string str = GetFormatTime(false);
			//	fprintf(m_logStat, "\n %13.13s |     SVC KEY PART [%4.4s] : %s, numsl = %2d, sl = %2d, ol = %2d, mbpart = %5d",
			//						str.c_str(), codec, part_name, it->second->numSLayers, sl, it->second->optSLayer, m_partFrameMB);
			//}
			if (sl == 0) {
				int mb(part_payload->max_mb), w(0), h(0), nth(0);
				if (ResolutionFromBitstream_VPX(pack + 5, size - 5, w, h, nth) == 0) {
					mb = w * h / 256;
				}
				else if (ResolutionFromBitstream_H264(pack + 5, size - 5, w, h) == 0) {
					mb = w * h / 256;
				}
				std::string str = GetFormatTime(false);
				if (numsl + 1 != num_sl) {
					if (m_logStat) {
						fprintf(m_logStat, "\n %13.13s |             num sl part | %15.15s | %2d -> %2d (ml = %2d)",
											str.c_str(), part_name, num_sl, numsl + 1, mixer_sl);
					}
					num_sl = numsl + 1;
					part_payload->threshold_mb.resize(numsl + 1, 0);
					part_payload->max_mb = 0;
				}
				if (mb != part_payload->max_mb) {
					if (m_logStat) {
						fprintf(m_logStat, "\n %13.13s |             max mb part | %15.15s | %2d -> %2d (nl = %2d, ml = %2d)",
											str.c_str(), part_name, part_payload->max_mb, mb, num_sl, mixer_sl);
					}
					part_payload->max_mb = mb;
					part_payload->threshold_mb[0] = mb;
					for (size_t i = 0; i < part_payload->threshold_mb.size(); i++) {
						part_payload->threshold_mb[i] = (part_payload->max_mb / (1 << (2 * i))) / 2;
					}
				}
			}
			fl = GetActiveDesktopCapture(part_name, part_payload, sl);
			int mixersl(0);
			if (strcmp(fl.first.c_str(), part_name) != 0) {
				for (mixersl = 0; mixersl < num_sl - 1; mixersl++) {
					if (part_payload->threshold_mb[mixersl] <= mixer_mb) {
						break;
					}
				}
			}
			if (mixersl != mixer_sl/* && optsl == sl*/) {
				if (m_logStat) {
					std::string str = GetFormatTime(false);
					fprintf(m_logStat, "\n %13.13s |           mixer mb part | %15.15s | snd = (%d mb, %d sl), peer = (%d mb, %d ml)",
										str.c_str(), part_name, part_payload->max_mb, num_sl, mixer_mb, mixersl);
				}
				part_payload->mixer_sl.store(mixersl);
			}
			part_payload->num_sl.store(num_sl);
		}
	}
	else {
		stream::SliceHeader* sh = reinterpret_cast<stream::SliceHeader*>(pack + size - sizeof(stream::SliceHeader));
		if (sh->id == sh->first_id) {
			key = pack[0] != 0;
		}
	}
	/// set optimal spatial layer
	mixer_sl = part_payload->mixer_sl.load();
	auto opt_sl(load_sl);
	if (opt_sl != -1) {
		opt_sl = std::max(mixer_sl, opt_sl);
	}
	if (opt_sl != part_payload->opt_sl.load()) {
		if ((key && (opt_sl == sl)) || opt_sl == -1) {
			if (m_logStat) {
				std::string str = GetFormatTime(false);
				fprintf(m_logStat, "\n %13.13s |             set sl part | %15.15s | %d -> %d, peer = (%d ll, %d ml)",
									str.c_str(), part_name, part_payload->opt_sl.load(), opt_sl, load_sl, mixer_sl);
			}
			part_payload->opt_sl.store(opt_sl);
			key_request = false;
		}
		else {
			key_request = true;
		}
	}
	return part_payload->opt_sl.load();
}

void VS_RelayMediaSource::UpdateStatistics(relay::VideoPayload *mp, uint32_t mb, uint8_t tl, int size, const uint64_t ct)
{
	if (mp->m_updateStatistic == 0) {
		mp->m_updateStatistic = ct;
	}
	auto dt = ct - mp->m_updateStatistic;
	if (dt >= 2000) {
		int64_t s(0);
		for (const auto & it : mp->layerTemporalSize) {
			s += it;
		}
		float ktl = static_cast<float>(mp->layerTemporalSize[0]) / static_cast<float>(s);
		mp->ktl = (mp->ktl + ktl) / 2.0f;
		UpdateLayersCoefs(mp);
		UpdateMixerBitrate(mp, dt);
		mp->m_updateStatistic = ct;
	}
	assert(tl < mp->numtl);
	mp->layerTemporalSize[tl] += size;
	mp->layerSpatialSize[mb] += size;
}

void VS_RelayMediaSource::UpdateLayersCoefs(relay::VideoPayload *mp)
{
	auto oldcoefs = mp->layerCoefs;
	mp->layerCoefs.clear();
	mp->layerCoefs.resize(mp->numtl * mp->numsl + 1, 0);
	float ks(1.0f);
	int32_t idx(0);
	for (int s = 0; s < mp->numsl; s++) {
		float kt(1.0f);
		for (int t = 0; t < mp->numtl; t++) {
			mp->layerCoefs[idx] = std::min(static_cast<int32_t>(ks * kt * 100), 100);
			kt *= mp->ktl;
			idx++;
		}
		ks *= mp->ksl;
	}
	if (m_logStat) {
		if (oldcoefs != mp->layerCoefs) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |        update svc coefs | %15" PRIxPTR " | ", str.c_str(), reinterpret_cast<std::uintptr_t>(mp));
			for (size_t i = 0; i < mp->layerCoefs.size() - 1; i++) {
				fprintf(m_logStat, "%3d, ", mp->layerCoefs[i]);
			}
		}
	}
	mp->layerTemporalSize.clear();
	mp->layerTemporalSize.resize(mp->numtl, 0);
}

void VS_RelayMediaSource::UpdateMixerBitrate(relay::VideoPayload *mp, uint64_t dt)
{
	auto oldbitrates = mp->mixerBitrate;
	mp->mixerBitrate.clear();
	int32_t bitrate(0);
	for (const auto & mb : mp->layerSpatialSize) {
		int32_t btr = static_cast<int32_t>(static_cast<double>(mb.second - mp->layerSpatialLastSize[mb.first]) * 8.0 / static_cast<double>(dt) + 0.5);
		mp->mixerBitrate[mb.first] = btr;
		bitrate += btr;
	}
	float k(0.0f);
	if (mp->numsl > 1) {
		for (const auto & mb : mp->mbPeers) {
			if (mb.second > 0) {
				k += m_svcProperty.mbSpatial[mb.first].k;
			}
		}
	}
	else {
		k = 1.0f;
	}
	if (k > 0.0f) {
		mp->realMixerBitrate = std::max(static_cast<int32_t>(static_cast<float>(bitrate) / k), 30);
		int thr = mp->realMixerBitrate * 100 / m_conf_bitrate;
		if (thr < 40) {
			mp->schemeBitrate = eSchemeBitrate::real;
		}
		else {
			mp->schemeBitrate = eSchemeBitrate::rough;
		}
	}
	if (m_logStat) {
		if (oldbitrates != mp->mixerBitrate) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |    update mixer bitrate | %15" PRIxPTR " | ", str.c_str(), reinterpret_cast<std::uintptr_t>(mp));
			for (const auto & mb : mp->mixerBitrate) {
				fprintf(m_logStat, "[mb = %5d, btr = %5d], ", mb.first, mb.second);
			}
			fprintf(m_logStat, " rb = %5d [%s]", mp->realMixerBitrate, (mp->schemeBitrate == eSchemeBitrate::real) ? "real" : "rough");
		}
	}
	mp->layerSpatialLastSize = mp->layerSpatialSize;
}

void VS_RelayMediaSource::UpdateBitratePeers(const uint64_t ct)
{
	auto dt = ct - m_updatePeersState;
	if (dt >= 2000) {
		VS_AutoLock lock(this);
		for (auto & lf : m_mediaStreamPool) {
			for (auto & cdc : lf.second->videoPayload) {
				std::map<uint32_t /* mb */, int32_t /* num peers*/> mixermbs;
				for (const auto & mb : cdc.second->layerSpatialSize) {
					mixermbs[mb.first] = 0;
				}
				for (auto & peer : cdc.second->peers) {
					relay::BitrateInfo *bi = &peer->bi;
					/// recalc bitrate
					int32_t lastSetBitrate = bi->last_snd_bitrate; /// async for SetRates
					int32_t maxSetBitrate = std::min(lastSetBitrate, bi->max_snd_bitrate);
					int32_t minSetBitrate = std::max(lastSetBitrate, 10);
					int32_t btr(0);
					if (lastSetBitrate > bi->snd_bitrate) {
						btr = (lastSetBitrate + bi->snd_bitrate * 3 + 4) / 4;
					}
					else {
						btr = (lastSetBitrate + bi->snd_bitrate + 2) / 2;
					}
					if (btr > (125 * bi->snd_bitrate / 100)) {
						btr = 125 * bi->snd_bitrate / 100;
					}
					if (btr < (5 * bi->snd_bitrate / 10)) {
						btr = 5 * bi->snd_bitrate / 10;
					}
					int32_t dbtr(btr - bi->snd_bitrate);
					if (dbtr < 0) {
						btr = bi->snd_bitrate + std::min(-10, dbtr);
						if (btr > maxSetBitrate) {
							btr = maxSetBitrate;
						}
					}
					else {
						btr = bi->snd_bitrate + std::max(10, dbtr);
						if (btr > maxSetBitrate) {
							btr = maxSetBitrate;
						}
					}
					/// recalc k load
					int32_t maxBitrate = (cdc.second->schemeBitrate == eSchemeBitrate::rough) ? bi->max_snd_bitrate : cdc.second->realMixerBitrate;
					int32_t k = std::min(btr * 100 / maxBitrate, 100);
					uint32_t idx(1);
					for (idx = 1; idx < cdc.second->layerCoefs.size(); idx++) {
						if (k >= cdc.second->layerCoefs[idx]) {
							break;
						}
					}
					uint8_t sl = (idx - 1) / cdc.second->numtl;
					uint8_t tl = (cdc.second->numtl - 1) - ((idx - 1) % cdc.second->numtl);
					uint32_t mb = std::min(m_svcProperty.sl2mb[sl], peer->mbmax);
					if (m_logStat) {
						std::string str = GetFormatTime(false);
						if (btr != bi->snd_bitrate) {
							fprintf(m_logStat, "\n %13.13s |            bitrate peer | %15.15s | b = (%5d <- %5d, %5d)", str.c_str(), peer->mp->GetPeerId().c_str(), btr, bi->snd_bitrate, lastSetBitrate);
						}
						if (peer->mbn != mb || peer->tln != tl) {
							fprintf(m_logStat, "\n %13.13s |             layers peer | %15.15s | k = %3d, [sln = %2d, tln = %2d], mbn = %5d", str.c_str(), peer->mp->GetPeerId().c_str(), k, sl, tl, mb);
						}
					}
					else {
						if (btr != bi->snd_bitrate) {
							dprint3("%p BITRATE %.5s : b = (%5d <- %5d, %5d)\n", this, peer->mp->GetPeerId().c_str(), btr, bi->snd_bitrate, lastSetBitrate);
						}
					}
					peer->mbn = mb;
					peer->tln = tl;
					bi->snd_bitrate = btr;
					mixermbs[peer->mbn]++;
					if (peer->mbc != peer->mbn) {
						mixermbs[peer->mbc]++;
					}
				}
				if (mixermbs != cdc.second->mbPeers) {
					std::uintptr_t handle = reinterpret_cast<std::uintptr_t>(lf.second.get());
					for (const auto & mb : mixermbs) {
						m_av_mixer.EnableVideoLayer(mb.second > 0, handle, cdc.first, mb.first);
					}
					if (m_logStat) {
						std::string str = GetFormatTime(false);
						fprintf(m_logStat, "\n %13.13s |     switch mixer layers | %15" PRIxPTR " | ", str.c_str(), reinterpret_cast<std::uintptr_t>(cdc.second.get()));
						for (const auto & it : mixermbs) {
							fprintf(m_logStat, "[%5d : %2d], ", it.first, it.second);
						}
					}
					cdc.second->mbPeers = mixermbs;
				}
			}
		}
		m_updatePeersState = ct;
		if (m_logStat) {
			fflush(m_logStat);
		}
	}
}

void VS_RelayMediaSource::TransmitFrame(const char *conf_name, const char *part_name, const stream::FrameHeader *frame_head, const void *frame_data)
{
	stream::TrackType type(stream::TrackType::undef);
	if (frame_head->track == stream::Track::audio) {
		type = stream::TrackType::audio;
	}
	else if (frame_head->track == stream::Track::video) {
		type = stream::TrackType::video;
	}
	TransmitFrame(conf_name, part_name, frame_data, frame_head->length, frame_head->track, type);
}

void VS_RelayMediaSource::TransmitFrame(const char* conf_name, const char* part_name, const void* frame_data, size_t size, stream::Track track, stream::TrackType type)
{
	if (m_conf_type == VS_Conference_Type::CT_PRIVATE) {
		if (track == stream::Track::command) {
			const auto& cmd = *static_cast<const stream::Command*>(frame_data);
			if (cmd.type == stream::Command::Type::ChangeRcvMFormat && cmd.sub_type == stream::Command::Info) {
				const auto& mf = *reinterpret_cast<const VS_MediaFormat*>(cmd.data);
				if (mf.IsAudioValid()) {
					m_av_mixer.ReinitAudioRcv(part_name, mf);
				}
			}
		}
	}
	if (type != stream::TrackType::audio
		&& type != stream::TrackType::video
		&& type != stream::TrackType::slide) {
		return;
	}
	unsigned char* pack = (unsigned char*) frame_data;
	unsigned long sz = size;
	pairForceLayout fl;
	{
		VS_AutoLock lock(this);
		if (m_mediaStreamPool.empty()) {
			ChangePriority(fl);
			return;
		}
	}
	if (type == stream::TrackType::video) {
		std::shared_ptr<relay::PartScalablePayload> part;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
			auto it = m_partScalablePool.find(part_name);
			if (it != m_partScalablePool.end()) {
				part = it->second;
			}
		}
		if (part) {
			bool key(false), key_request(part->key_request.load());
			int sl = GetOptimalSpatialLayer(part.get(), pack, sz, part_name, key, key_request, fl);
			if (key_request) {
				RequestKeyFrameFromParticipant(part_name, part.get());
				part->key_request.store(false);
			}
			if (part->use_svc.load()) {
				/// with svc
				int sl_ = (0x30 & pack[sz - 1]) >> 4;
				if ((sl == -1 && key) || (sl == sl_)) {
					m_av_mixer.PutData(part_name, track, pack, sz - 1);
				}
			}
			else {
				/// without svc
				if (sl >= 0 || key) {
					m_av_mixer.PutData(part_name, track, pack, sz);
				}
			}
		}
	}
	else if (type == stream::TrackType::audio || type == stream::TrackType::slide) {
		m_av_mixer.PutData(part_name, track, pack, sz);
	}
	ChangePriority(fl);
}

void VS_RelayMediaSource::KeyFrameRequestForPeer(const char *part_id, const char *peer_id)
{
	std::shared_ptr<relay::PeerScalablePayload> peer;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
		auto it = m_peerScalablePool.find(peer_id);
		if (it != m_peerScalablePool.end()) {
			peer = it->second;
		}
	}
	if (peer.get()) {
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |        request key peer | %15.15s |", str.c_str(), peer_id);
		}
		else {
			dprint3("RequestKeyFrame from WebRTC part = %s, peer = %s\n", part_id, peer_id);
		}
		m_av_mixer.RequestKeyFrame(peer->handle, peer->fourcc);
	}
}

void VS_RelayMediaSource::SetRates(const char *part_id, const char *peer_name, unsigned int bitrate, unsigned int frame_rate)
{
	std::shared_ptr<relay::PeerScalablePayload> peer;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
		auto it = m_peerScalablePool.find(peer_name);
		if (it != m_peerScalablePool.end()) {
			peer = it->second;
		}
	}
	if (peer.get()) {
		int32_t db = std::abs(peer->bi.last_set_snd_bitrate - static_cast<int32_t>(bitrate));
		int32_t thr = std::max<int32_t>(5, peer->bi.last_set_snd_bitrate / 100);
		if (db >= thr || (db >= 0 && bitrate == peer->bi.max_snd_bitrate)) {
			if (m_logStat) {
				std::string str = GetFormatTime(false);
				fprintf(m_logStat, "\n %13.13s |    request bitrate peer | %15.15s | b = %5u", str.c_str(), peer_name, bitrate);
			}
			peer->bi.last_set_snd_bitrate = bitrate;
			peer->bi.last_snd_bitrate = std::min(static_cast<int32_t>(bitrate), peer->bi.max_snd_bitrate);
		}
	}
}

void VS_RelayMediaSource::SetSndFrameResolution(const char *part_id, const char *peer_name, const char *plname, uint8_t pltype, unsigned short width, unsigned short height)
{
	if (plname == nullptr || plname[0] == 0) {
		return;
	}
	auto it = m_payloadVideo.find(plname);
	if (it != m_payloadVideo.end()) {
		VS_MediaFormat mf;
		mf.SetVideo(m_mf.dwVideoWidht, m_mf.dwVideoHeight, it->second, m_mf.dwFps);
		ChangeVideoSendPayload(part_id, peer_name, vs_media_peer_type_webrtc, mf);
	}
	dprint3("SetSndFrameResolution %dx%d\n", width, height);
}

static bool AudioCodecMatches(int base_payload, const char *base_plname, int payload, const char *plname)
{
	return (payload <= 95) ? (base_payload == payload) : (strcasecmp(base_plname, plname) == 0);
}

void VS_RelayMediaSource::ChangeAudioSendPayload(const char *part_id, const char *peer_id, const char *plname, int plfreq)
{
	VS_AutoLock lock(this);
	auto apl = m_payloadAudio.find(plname);
	if (apl == m_payloadAudio.end()) {
		return;
	}
	uint32_t twocc(apl->second);
	uint8_t op(relay::MixerOp::none);
	auto peer = RemoveAudioPeerPayload(peer_id, twocc, op);
	if (!peer.get()) {
		return;
	}
	else {
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerAudioLock);
		m_peerAudioPool[peer_id] = peer;
	}
	MixerAudioPoolClean(op, peer->handle, peer->twocc, peer->samplerate);
	VS_MediaFormat mf;
	mf.SetAudio(plfreq, twocc);
	if (mf.IsAudioValid()) {
		auto mp = CreateMixer(peer->lf, peer->handle);
		auto ap = CreateAudioPayloadPeer(peer->handle, twocc, mp->audioPayload, peer->lf);
		if (!ap) {
			return;
		}
		if (CreateAudioStreamPeer(peer->handle, ap, mf, peer->mp, peer->lf)) {
			if (m_logStat) {
				std::string str = GetFormatTime(false);
				fprintf(m_logStat, "\n %13.13s |    change audio pl peer | %15.15s | audio pool = %15" PRIxPTR ", sr = %u", str.c_str(), peer_id, reinterpret_cast<std::uintptr_t>(ap.get()), mf.dwAudioSampleRate);
			}
		}
		else {
			if (m_logStat) {
				std::string str = GetFormatTime(false);
				fprintf(m_logStat, "\n %13.13s |   can't change apl peer | %15.15s | video pool = %15" PRIxPTR "", str.c_str(), peer_id, reinterpret_cast<std::uintptr_t>(ap.get()));
			}
		}
	}
	else {
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |    remove audio pl peer | %15.15s |", str.c_str(), peer_id);
		}
	}
}

void VS_RelayMediaSource::ChangeVideoSendPayload(const char *part_id, const char *peer_id, VS_MediaPeer_Type peer_type, const VS_MediaFormat& mf)
{
	bool forceFormat(false);
	VS_MediaFormat out;
	VS_MediaFormat mf_ = mf;
	//if (forceFormat) {
	//	mf_.dwVideoWidht = 640;
	//	mf_.dwVideoHeight = 480;
	//}
	ChangeVideoSendPayload(part_id, peer_id, mf_, out, forceFormat);
}

void VS_RelayMediaSource::ChangeVideoSendPayload(const char *part_id, const char *peer_id, const VS_MediaFormat& mf, VS_MediaFormat &out, bool forceFormat)
{
	VS_AutoLock lock(this);
	uint32_t fourcc(mf.dwVideoCodecFCC);
	uint8_t op(relay::MixerOp::none);
	auto peer = RemoveVideoPeerPayload(peer_id, (forceFormat) ? 0 : fourcc, op);
	if (!peer.get()) {
		return;
	}
	MixerVideoPoolClean(op, peer->handle, peer->fourcc, peer->mbn);
	if (mf.IsVideoValid()) {
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
			m_peerScalablePool[peer_id] = peer;
		}
		if (forceFormat) {
			peer->lf = GetVideoLayoutFormat(peer->mp.get(), mf.dwVideoWidht, mf.dwVideoHeight);
		}
		auto mp = CreateMixer(peer->lf, peer->handle);
		auto vp = CreateVideoPayloadPeer(peer->handle, fourcc, mp->videoPayload, peer->lf);
		if (!vp) {
			return;
		}
		out.SetVideo(mf.dwVideoWidht, mf.dwVideoHeight, fourcc);
		if (!forceFormat) {
			RestrictMediaFormat(out, peer->mp->GetPeerType());
		}
		if (CreateVideoStreamPeer(peer->handle, vp, out, peer->mp, peer->lf)) {
			if (m_logStat) {
				std::string str = GetFormatTime(false);
				fprintf(m_logStat, "\n %13.13s |    change video pl peer | %15.15s | video pool = %15" PRIxPTR ", %4u x %4u, %s", str.c_str(), peer_id, reinterpret_cast<std::uintptr_t>(vp.get()), out.dwVideoWidht, out.dwVideoHeight, (forceFormat) ? "force" : "");
			}
		} else {
			if (m_logStat) {
				std::string str = GetFormatTime(false);
				fprintf(m_logStat, "\n %13.13s |   can't change vpl peer | %15.15s | video pool = %15" PRIxPTR "", str.c_str(), peer_id, reinterpret_cast<std::uintptr_t>(vp.get()));
			}
		}
	}
	else {
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |    remove video pl peer | %15.15s |", str.c_str(), peer_id);
		}
	}
}

void VS_RelayMediaSource::ChangeVideoLayoutFormat(const avmuxer::LayoutFormat & lf, const std::string& peer_id)
{
	VS_AutoLock lock(this);
	uint32_t correctmb(0);
	uint8_t op(relay::MixerOp::none);
	auto peer = RemoveVideoPeerPayload(peer_id.c_str(), 0, op);
	if (!peer.get()) {
		return;
	}
	else {
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
		m_peerScalablePool[peer_id] = peer;
	}
	MixerVideoPoolClean(op, peer->handle, peer->fourcc, peer->mbn);
	auto mp = CreateMixer(lf, peer->handle);
	auto vp = CreateVideoPayloadPeer(peer->handle, peer->fourcc, mp->videoPayload, lf);
	if (!vp) {
		return;
	}
	if (CreateVideoStreamPeer(peer->handle, vp, peer->mf, peer->mp, lf)) {
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |   new video layout peer | %s ", str.c_str(), peer_id.c_str());
		}
	}
}

void VS_RelayMediaSource::ChangeAudioLayoutFormat(const avmuxer::LayoutFormat & lf, const std::string& peer_id)
{
	VS_AutoLock lock(this);
	uint8_t op(relay::MixerOp::none);
	auto peer = RemoveAudioPeerPayload(peer_id.c_str(), 0, op);
	if (!peer.get()) {
		return;
	}
	else {
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerAudioLock);
		m_peerAudioPool[peer_id] = peer;
	}
	MixerAudioPoolClean(op, peer->handle, peer->twocc, peer->samplerate);
	auto mp = CreateMixer(lf, peer->handle);
	auto ap = CreateAudioPayloadPeer(peer->handle, peer->twocc, mp->audioPayload, lf);
	if (!ap) {
		return;
	}
	if (CreateAudioStreamPeer(peer->handle, ap, peer->mf, peer->mp, lf)) {
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s |   new audio layout peer | %s ", str.c_str(), peer_id.c_str());
		}
	}
}

void VS_RelayMediaSource::ChangePeerLayoutFormat(const relay::PeerScalablePayload *videoPeer, const relay::PeerAudioPayload *audioPeer)
{
	if (videoPeer) {
		auto mediaPeer = videoPeer->mp.get();
		if (mediaPeer) {
			auto lf = GetVideoLayoutFormat(mediaPeer, videoPeer->lf.width, videoPeer->lf.height);
			if (lf != videoPeer->lf) {
				ChangeVideoLayoutFormat(lf, mediaPeer->GetPeerId());
			}
		}
	}
	if (audioPeer) {
		auto mediaPeer = audioPeer->mp.get();
		if (mediaPeer) {
			auto lf = GetAudioLayoutFormat(mediaPeer);
			if (lf != audioPeer->lf) {
				ChangeAudioLayoutFormat(lf, mediaPeer->GetPeerId());
			}
		}
	}
	ResetPriority();
}

void VS_RelayMediaSource::ChangePeerLayoutFormat(const std::string & id)
{
	auto videoPeer = GetVideoPeer(id);
	auto audioPeer = GetAudioPeer(id);
	if (videoPeer || audioPeer) {
		ChangePeerLayoutFormat(videoPeer.get(), audioPeer.get());
	}
}

void VS_RelayMediaSource::ChangePeersLayoutFormat()
{
	VS_AutoLock lock(this);
	mapPeerScalablePool peers;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
		peers = m_peerScalablePool;
	}
	for (auto &it : peers) {
		ChangePeerLayoutFormat(it.first);
	}
}

void VS_RelayMediaSource::ReadyToDie(const char *part_id, const char *peer_id)
{
	VS_AutoLock lock(this);
	m_partsByPeers.erase(peer_id);
	std::map<std::string, std::shared_ptr<VS_ParticipantPoolItem>>::iterator part_i = m_part_pool.find(part_id);
	if(part_i!=m_part_pool.end())
	{
		std::map<std::string, std::shared_ptr<VS_MediaPeerBase>>::iterator peer_i = part_i->second->peers.find(peer_id);
		if (peer_i != part_i->second->peers.end()) {
			peer_i->second->Close();
			part_i->second->peers.erase(peer_i);
		}
		if(part_i->second->peers.empty())
			m_part_pool.erase(part_i);
	}
}

void VS_RelayMediaSource::CallbackVideo(const avmuxer::LayoutPayload &payload, uint8_t* data, int size, uint8_t tl, bool key, uint32_t tm)
{
	if (!data || size < 0) {
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s | bad params mix cb | fourcc = %x", str.c_str(), payload.fourcc);
		}
		else {
			dprint1("BAD PARAMS in mixer callback, fourcc = %x\n", payload.fourcc);
		}
		return;
	}
	//else {
	//	if (m_logStat) {
	//		std::string str = GetFormatTime(false);
	//		fprintf(m_logStat, "\n %13.13s |           receive frame | tl = %2d, mb = %4d, key = %d, tm = %10u", str.c_str(), tl, payload.mb, key ? 1 : 0, tm);
	//	}
	//}
	auto ct = avmuxer::getTickMs();
	std::list<std::shared_ptr<VS_MediaPeerBase>> peers_for_video;
	{
		VS_AutoLock lock(this);
		if (m_peerScalablePool.empty()) {
			return;
		}
		auto lf = m_mediaStreamPool.find(payload.lf);
		if (lf == m_mediaStreamPool.end()) {
			return;
		}
		auto cdc = lf->second->videoPayload.find(payload.fourcc);
		if (cdc == lf->second->videoPayload.end()) {
			return;
		}
		auto pl = cdc->second;
		UpdateStatistics(pl.get(), payload.mb, tl, size, ct);
		for (auto & peer : pl->peers) {
			if (peer->mbn != peer->mbc) {
				if (peer->mbn == payload.mb && key) {
					if (m_logStat) {
						std::string str = GetFormatTime(false);
						fprintf(m_logStat, "\n %13.13s |          change peer sl | %15.15s | %2d -> %2d", str.c_str(), peer->mp->GetPeerId().c_str(), peer->mbc, peer->mbn);
					}
					peer->mbc = peer->mbn;
				}
			}
			if (peer->mbc != payload.mb) {
				continue;
			}
			if (peer->tln != peer->tl) {
				if (tl == 0) {
					if (m_logStat) {
						std::string str = GetFormatTime(false);
						fprintf(m_logStat, "\n %13.13s |          change peer tl | %15.15s | %2d -> %2d", str.c_str(), peer->mp->GetPeerId().c_str(), peer->tl,peer->tln);
					}
					peer->tl = peer->tln;
				}
			}
			if (tl > peer->tl) {
				continue;
			}
			peers_for_video.push_back(peer->mp);
		}
	}
	for (auto & mp : peers_for_video) {
		//if (m_logStat) {
		//	std::string str = GetFormatTime(false);
		//	fprintf(m_logStat, "\n %13.13s |               put frame | %15.15s | tl = %2d, mb = %4d, key = %d, tm = %10u, s = %8d", str.c_str(), mp->GetPeerId().c_str(), tl, payload.mb, key ? 1 : 0, tm, size);
		//}
		mp->PutVideo(data, size, key, tm);
	}
	ct = avmuxer::getTickMs();
	UpdateBitratePeers(ct);
}

void VS_RelayMediaSource::CallbackAudio(const avmuxer::LayoutFormat &format, uint32_t twocc, uint32_t samplerate, uint8_t* data, int size, uint32_t samples, uint32_t tm)
{
	if (!data || size < 0) {
		if (m_logStat) {
			std::string str = GetFormatTime(false);
			fprintf(m_logStat, "\n %13.13s | bad params mix cb | twocc = %x", str.c_str(), twocc);
		}
		else {
			dprint1("BAD PARAMS in mixer callback audio, twocc = %x\n", twocc);
		}
		return;
	}
	std::list<std::shared_ptr<VS_MediaPeerBase>> peers_for_audio;
	{
		VS_AutoLock lock(this);
		if (m_peerAudioPool.empty()) {
			return;
		}
		auto lf = m_mediaStreamPool.find(format);
		if (lf == m_mediaStreamPool.end()) {
			return;
		}
		auto cdc = lf->second->audioPayload.find(twocc);
		if (cdc == lf->second->audioPayload.end()) {
			return;
		}
		auto pl = cdc->second;
		for (auto & peer : cdc->second->peers) {
			if (peer->samplerate != samplerate) {
				continue;
			}
			peers_for_audio.push_back(peer->mp);
		}
	}
	for (auto & mp : peers_for_audio) {
		mp->PutCompressedAudio(data, size, samples, tm);
	}
}

void VS_RelayMediaSource::NewLayoutList(const std::uintptr_t handle, const avmuxer::LayoutControl &lc)
{
	json::Object data;
	if (CreateJsonMessage(lc, data)) {
		std::stringstream ss;
		json::Writer::Write(data, ss);
		if (ss.str().length() > 0) {
			if (lc.toPeer.empty()) {
				mapPeerScalablePool peers;
				{
					std::lock_guard<std::recursive_mutex> lock(m_mtxPeerScalableLock);
					peers = m_peerScalablePool;
				}
				for (auto &it : peers) {
					if (it.second->handle == handle) {
						m_fireDataForWebPeer(it.first.c_str(), m_conf.c_str(), ss.str().c_str());
					}
				}
			}
			else {
				m_fireDataForWebPeer(lc.toPeer.c_str(), m_conf.c_str(), ss.str().c_str());
			}
		}
	}
}

void VS_RelayMediaSource::UpdateParticipantsMb(const std::map<std::string /* part id */, int32_t /* optimal part mb */ > &mbs)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
	for (const auto &it : mbs) {
		auto partpl = m_partScalablePool.find(it.first);
		if (partpl != m_partScalablePool.end()) {
			partpl->second->mixer_mb.store(it.second);
		}
	}
}

void VS_RelayMediaSource::UpdateParticipantKeyRequest(const std::string &id)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
	auto partpl = m_partScalablePool.find(id);
	if (partpl != m_partScalablePool.end()) {
		partpl->second->key_request.store(true);
	}
}

void VS_RelayMediaSource::UpdateLoadDecompressor(const avmuxer::LoadStatistic &load)
{
	if (!m_load_decompressor) {
		return;
	}
	if (load.li.load >= 0.95f) {
		m_perfInfo.state = performanceState::danger;
		if (load.li.vp >= 100) {
			m_perfInfo.state = performanceState::critical;
		}
	}
	else {
		if (m_perfInfo.state == performanceState::ok) {
			if (load.li.load >= 0.75f) {
				m_perfInfo.state = performanceState::warning; /// prev - ok
			}
		}
		else if (load.li.load <= 0.85f) {
			if (m_perfInfo.state == performanceState::critical) {
				if (load.li.vp == 0) {
					m_perfInfo.state = performanceState::danger;  /// prev - critical
				}
			}
			else if (m_perfInfo.state == performanceState::danger) {
				m_perfInfo.state = performanceState::warning;  /// prev - danger
			}
			else if (load.li.load <= 0.65f) {
				m_perfInfo.state = performanceState::ok;  /// prev - warning
			}
		}
	}
	std::string str;
	if (m_logStat) {
		str = GetFormatTime(false);
		std::string info("ok");
		if (m_perfInfo.state == performanceState::warning) {
			info = "warning";
		}
		else if (m_perfInfo.state == performanceState::danger) {
			info = "danger";
		}
		else if (m_perfInfo.state == performanceState::critical) {
			info = "critical";
		}
		fprintf(m_logStat, "\n %13.13s |    load decompress loop | %15.15s | l = %5.3f, dt = %6" PRIu64 ", vt = %6" PRIu64 ", at = %6" PRIu64 ", vp = %8" PRIu64 "", str.c_str(), info.c_str(), load.li.load, load.dt, load.li.vt, load.li.at, load.li.vp);
	}
	auto ct = avmuxer::getTickMs();
	if ((m_perfInfo.state == performanceState::danger || m_perfInfo.state == performanceState::critical) && load.li.load >= 0.95f) {
		/// downgrade quality
		if (m_perfInfo.calculateDownTm == 0) {
			m_perfInfo.calculateDownTm = ct;
		}
		auto dt = ct - m_perfInfo.calculateDownTm;
		if (dt >= 10000) {
			auto downgradeLayer = [&str, &ct, this](relay::PartScalablePayload *pl, const std::string &id, uint64_t vt) -> bool
			{
				int32_t load_sl(pl->load_sl.load());
				int32_t mixer_sl(pl->mixer_sl.load());
				int32_t num_sl(pl->num_sl.load());
				if (load_sl != -1) {
					int32_t sl_(load_sl);
					load_sl = std::max(load_sl, mixer_sl);
					load_sl++;
					if (load_sl == num_sl) {
						load_sl = -1;
					}
					pl->vt = vt;
					if (m_logStat) {
						fprintf(m_logStat, "\n %13.13s |    load decompress loop | %15.15s | lsl : %2d -> %2d, [ml = %2d, nl = %2d]", str.c_str(), id.c_str(), sl_, load_sl, mixer_sl, num_sl);
					}
					pl->load_sl.store(load_sl);
					m_perfInfo.calculateDownTm = ct;
					m_perfInfo.calculateUpTm = ct;
					return true;
				}
				return false;
			};
			uint64_t dsvt = std::numeric_limits<uint64_t>::max();
			std::string dsname;
			{
				std::lock_guard<std::recursive_mutex> lock(m_mtxActiveDesktop);
				if (!m_mapPartDS.empty()) {
					dsname = m_mapPartDS.crbegin()->second;
				}
			}
			{
				std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
				for (const auto &it : load.mli) {
					if (it.first == dsname) {
						dsvt = it.second.vt;
						continue;
					}
					auto p = m_partScalablePool.find(it.first);
					if (p != m_partScalablePool.end()) {
						if (downgradeLayer(p->second.get(), p->first, it.second.vt)) {
							break;
						}
					}
				}
				if ((m_perfInfo.calculateDownTm == ct) && !dsname.empty() && (dsvt != std::numeric_limits<uint64_t>::max())) {
					auto p = m_partScalablePool.find(dsname);
					if (p != m_partScalablePool.end()) {
						downgradeLayer(p->second.get(), p->first, dsvt);
					}
				}
			}
		}
	}
	else if (load.li.load <= 0.9f) {
		/// upgrade quality
		if (m_perfInfo.calculateUpTm == 0) {
			m_perfInfo.calculateUpTm = ct;
		}
		bool increase(false);
		std::lock_guard<std::recursive_mutex> lock(m_mtxPartScalableLock);
		for (const auto &p : m_partScalablePool) {
			int32_t load_sl(p.second->load_sl.load());
			int32_t opt_sl(p.second->opt_sl.load());
			if ((load_sl > opt_sl) || (load_sl == -1 && opt_sl != -1)) {
				m_perfInfo.calculateUpTm = ct;
				break;
			}
		}
		auto dt = ct - m_perfInfo.calculateUpTm;
		if ((dt >= 10000) || (m_perfInfo.state == performanceState::ok)) {
			for (auto &p : m_partScalablePool) {
				int32_t load_sl(p.second->load_sl.load());
				int32_t num_sl(p.second->num_sl.load());
				int32_t mixer_sl(p.second->mixer_sl.load());
				float load_(load.li.load);
				if (load_sl == 0) {
					continue;
				}
				else {
					{
						auto cmp = [p](const std::pair<std::string, avmuxer::LoadStatisticItem> &a) { return a.first == p.first; };
						auto it = std::find_if(load.mli.begin(), load.mli.end(), cmp);
						if (it != load.mli.end()) {
							load_ = static_cast<float>((load.li.vt - it->second.vt + p.second->vt) + load.li.at) / static_cast<float>(load.dt);
						}
					}
					if (load_ <= 0.95f) {
						if (load_sl == -1) {
							load_sl = num_sl - 1;
							increase = true;
						}
						else if (load_sl > mixer_sl) {
							load_sl--;
							increase = true;
						}
						else {
							load_sl = 0;
						}
					}
				}
				p.second->load_sl.store(load_sl);
				if (increase) {
					if (m_logStat) {
						fprintf(m_logStat, "\n %13.13s |    load decompress loop | %15.15s | lsl : %2d -> %2d, load : %5.3f -> %5.3f [ml = %2d, nl = %2d]",
							str.c_str(), p.first.c_str(), load_sl, p.second->load_sl.load(), load.li.load, load_, mixer_sl, num_sl);
					}
					m_perfInfo.calculateUpTm = ct;
					break;
				}
			}
		}
	}
}

void VS_RelayMediaSource::ChangePriority(pairForceLayout fl)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPriorityList);

	std::string handle;
	if (fl.second) {
		m_dsHandle = fl.first;
	}
	auto ct = avmuxer::getTickMs();
	if (m_enableVadLayouts && (ct - m_lastVadTime) > 500) {
		std::string loudestHandle = m_av_mixer.GetLoudestHandle();
		if (!loudestHandle.empty()) {
			m_vadHandle = loudestHandle;
		}
		m_lastVadTime = ct;
	}
	if (m_enableDsLayouts && !m_dsHandle.empty()) {
		handle = m_dsHandle;
	}
	else if (m_enableVadLayouts) {
		handle = m_vadHandle;
	}
	if (m_currHandle != handle) {
		avmuxer::LayoutControl lc;
		if (!handle.empty()) {
			lc.function = avmuxer::SetP1LayoutFunction;
			lc.userId1 = handle;
		}
		else {
			lc.function = avmuxer::SetP0LayoutFunction;
		}
		std::vector<std::uintptr_t> handleMixers;
		GetHandleMixersPriority(handle, handleMixers);
		for (const auto &h : handleMixers) {
			m_av_mixer.ManageLayout(h, lc);
			lc.function = avmuxer::PriorityTypeLayoutFunction;
			lc.type = VS_WindowGrid::PRIORITY_LAYOUT_OVERLAY;
			if (!m_dsHandle.empty()) {
				lc.type = VS_WindowGrid::PRIORITY_LAYOUT_TOP;
			}
			m_av_mixer.ManageLayout(h, lc);
		}

		m_currHandle = handle;
	}
}

void VS_RelayMediaSource::ResetPriority()
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPriorityList);
	m_currHandle = "";
}

bool VS_RelayMediaSource::ParseJsonMessage(const json::Object & data, avmuxer::LayoutControl &lc)
{
	bool ret(false);
	try {
		std::string method;
		json::Object::const_iterator it = data.Find("method");
		if (it != data.End()) {
			method = (const json::String&) it->element;
		}
		if (method.empty() || method != "ManageLayout") {
			throw false;
		}
		it = data.Find("func");
		if (it != data.End()) {
			lc.function = (const json::String&) it->element;
		}
		else {
			throw false;
		}
		if (lc.function == avmuxer::PriorityTypeLayoutFunction) {
			it = data.Find("type");
			if (it != data.End()) {
				lc.type = (VS_WindowGrid::ePriorityLayoutType)(long)(0.5 + (const json::Number) it->element);
			}
		}
		else {
			it = data.Find("UserName");
			if (it != data.End()) {
				json::Array list = it->element;
				if (list.Size() > 0) {
					lc.userId1 = (const json::String) list[0];
				}
				if (list.Size() > 1) {
					lc.userId2 = (const json::String) list[1];
				}
			}
			if (lc.function == avmuxer::GetLayoutFunction) {
				it = data.Find("my_peer_id");
				if (it != data.End()) {
					lc.toPeer = (const json::String&) it->element;
				}
				else {
					throw false;
				}
			}
		}
		ret = true;
	}
	catch (json::Exception &) {
		dprint2("Bad json params");
	}
	catch (bool) {
		dprint3("Too few params for ManageLayout");
	}
	return ret;
}

bool VS_RelayMediaSource::CreateJsonMessage(const avmuxer::LayoutControl &lc, json::Object & data)
{
	data["type"] = json::String("layout");
	data["width"] = json::Number(lc.mixerWidht);
	data["height"] = json::Number(lc.mixerHeight);
	data["fixed"] = json::Boolean(m_fixedLayoutFormat.layout == avmuxer::VideoLayout::special);
	json::Array part_list;
	for (auto & it : lc.grid) {
		json::Object part;
		part[layout_json::id] = json::String(it.userId);
		part[layout_json::priority] = json::Boolean(it.priority == VS_WindowGrid::PRIORITY_HIGH);
		json::Array coord;
		coord.Insert(json::Number(it.rect.left));
		coord.Insert(json::Number(it.rect.top));
		coord.Insert(json::Number(it.rect.right));
		coord.Insert(json::Number(it.rect.bottom));
		part[layout_json::rect] = coord;
		part_list.Insert(part);
	}
	data["list"] = part_list;
	return true;
}

void VS_RelayMediaSource::SetFixedLayoutFormat(const json::Object & data)
{
	int emptySlotCount(0);
	avmuxer::LayoutFormat lf;
	lf.layout = avmuxer::VideoLayout::special;
	lf.mixerDesc.width = json::Number(data[layout_json::width]);
	lf.mixerDesc.height = json::Number(data[layout_json::height]);

	std::string displayNamePOsition = json::String(data[layout_json::display_name_position]);
	if (displayNamePOsition == layout_json::none)
		lf.mixerDesc.displayNamePosition = DNP_NONE;
	else if (displayNamePOsition == layout_json::top)
		lf.mixerDesc.displayNamePosition = DNP_TOP;
	else if (displayNamePOsition == layout_json::bottom)
		lf.mixerDesc.displayNamePosition = DNP_BOTTOM;

	json::Array slots = data[layout_json::slots];
	for (auto it = slots.Begin(); it != slots.End(); ++it) {
		json::Object slot = *it;
		json::Object rect = slot[layout_json::rect];
		{
			VSRayInfo info;
			info.rect.offset.x = json::Number(rect[layout_json::x]);
			info.rect.offset.y = json::Number(rect[layout_json::y]);
			info.rect.size.width = json::Number(rect[layout_json::width]);
			info.rect.size.height = json::Number(rect[layout_json::height]);
			info.displayname = json::String(slot[layout_json::display_name]);
			info.priority = json::Boolean(slot[layout_json::priority]);
			std::string type = json::String(slot[layout_json::type]);
			if (type == layout_json::empty) {
				info.type = VSRayInfo::RT_EMPTY;
			}
			else if (type == layout_json::content) {
				info.type = VSRayInfo::RT_CONTENT;
			}
			std::string id = json::String(slot[layout_json::instance]);
			if (info.type == VSRayInfo::RT_CONTENT) {
				id = relay::default_content_id;
			}
			if (!id.empty()) {
				lf.peers.insert(id);
			}
			else {
				id = "empty" + std::to_string(emptySlotCount++);
			}
			lf.mixerDesc.layout[id] = info;
		}
	}
	{
		VS_AutoLock lock(this);
		m_fixedLayoutFormat = lf;
		m_fixedLayoutFormat.Hash();
	}
}

void VS_RelayMediaSource::ResetLayoutFormat(avmuxer::LayoutFormat *lf)
{
	if (!lf) {
		return;
	}
	{
		VS_AutoLock lock(this);
		*lf = { avmuxer::VideoLayout::none, {} };
		lf->Hash();
	}
}

void VS_RelayMediaSource::TestChangePriority()
{
	static uint64_t lastTime = 0;
	auto ct = avmuxer::getTickMs();
	if (lastTime == 0) {
		lastTime = ct;
	}
	if (ct - lastTime >= 30000) {
		if (m_fixedLayoutFormat.layout == avmuxer::VideoLayout::none) {
			json::Object jsonLayout;
			std::string layoutsDir = std::string(MULTI_CONFERENCES_KEY) + '\\' + "remote_layout" + '\\' + "VideoLayouts";
			VS_RegistryKey reg_key(false, layoutsDir);
			if (reg_key.IsValid()) {
				std::string jsonText;
				if (reg_key.GetString(jsonText, "CommonLayoutJSON")) {
					json::Reader reader;
					std::stringstream ss(jsonText);
					reader.Read(jsonLayout, ss);
				}
			}
			if (!jsonLayout.Empty()) {
				SetFixedLayout(jsonLayout);
			}
		}
		else {
			ResetFixedLayout();
		}
		lastTime = ct;
	}
}

void VS_RelayMediaSource::TestAsymmetricLayout()
{
	static uint64_t lastTime = 0;
	auto ct = avmuxer::getTickMs();
	if (lastTime == 0) {
		lastTime = ct;
	}
	if (ct - lastTime >= 15000) {
		if (m_asymmetricLayoutFormat.layout == avmuxer::VideoLayout::none) {
			std::set<std::string> peers;
			peers.insert("1@ru2qm.trueconf.ru");
			peers.insert("2@ru2qm.trueconf.ru");
			SetAsymmetricLayout(peers);
		}
		else {
			ResetAsymmetricLayout();
		}
		lastTime = ct;
	}
}

void VS_RelayMediaSource::TestSlide()
{
	static uint64_t counter(0), scenario(0);
	static std::string id1, id2;
	static uint64_t lastTime(0), lastShow(0);
	auto ct = avmuxer::getTickMs();
	if (lastTime == 0) {
		lastTime = ct;
	}
	auto dt = ct - lastTime;
	if (dt >= 10000) {
		if (scenario % 2 == 0) {
			if (dt < 20000) {
				id1 = "1@ru2qm.trueconf.name";
			}
			else if (dt < 30000) {
				id2 = "2@ru2qm.trueconf.name";
			}
			else if (dt < 40000) {
				StopSlide("", id1);
				id1.clear();
			}
			else {
				StopSlide("", id2);
				id2.clear();
				lastTime = ct;
				scenario++;
			}
		}
		else {
			if (dt < 20000) {
				id1 = "1@ru2qm.trueconf.name";
			}
			else if (dt < 30000) {
				id2 = "2@ru2qm.trueconf.name";
			}
			else if (dt < 40000) {
				StopSlide("", id2);
				id2.clear();
			}
			else {
				StopSlide("", id1);
				id1.clear();
				lastTime = ct;
				scenario++;
			}
		}
	}
	auto dt_show = ct - lastShow;
	if (dt_show >= 1000) {
		if (counter % 2 == 0 && !id1.empty()) {
			UpdateSlide("", id1, "https://developer.nvidia.com/sites/default/files/akamai/designworks/images-videocodec/VCSDK_006a.png");
		}
		else if (!id2.empty()) {
			UpdateSlide("", id2, "https://developer.nvidia.com/sites/default/files/akamai/designworks/images-videocodec/nvedec_9.1_1080p_002.png");
		}
		counter++;
		lastShow = ct;
	}
}

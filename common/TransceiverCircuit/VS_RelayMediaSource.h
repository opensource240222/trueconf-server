#pragma once
#include "../streams/Relay/VS_TransmitFrameInterface.h"
#include "../streams/Relay/VS_ConfControlInterface.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <mutex>
#include <boost/signals2.hpp>
#include <atomic>
#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/json/elements.h"
#include "../Transcoder/VS_AVMixerIface.h"
#include "VS_MediaPeerBase.h"
#include "Transcoder/CMixerPool.h"

enum eSchemeBitrate
{
	rough = 0,
	real = 1
};

struct VS_ParticipantPoolItem
{
	std::string part_id; // TrueConfID: a@kt.pca.ru
	std::map<std::string, std::shared_ptr<VS_MediaPeerBase>> peers;
};

namespace relay {

	static const std::string default_content_id = "Content-<%%>-";
	static const std::string default_content_prefix_dn = "Content";
	static const stream::Track default_content_track = stream::track(250);

	enum MixerOp {
		none = 0x00,
		remove_layer = 0x01,
		remove_stream = 0x02,
		remove_pool = 0x04,
		remove_mixer = 0x08
	};

	struct BitrateInfo
	{
		int32_t max_snd_bitrate = 0;
		int32_t last_set_snd_bitrate = 0;
		int32_t last_snd_bitrate = 0;
		int32_t snd_bitrate = 0;
		int32_t rcv_bitrate = 0;
		uint32_t rcv_bytes = 0;
		uint32_t tm_rcv_bitrate = 0;
	};

	struct PeerScalablePayload
	{
		avmuxer::LayoutFormat lf;
		std::uintptr_t handle = 0;
		uint32_t fourcc = 0;
		uint32_t mbmax = 0;
		uint32_t mbc = 0;
		uint32_t mbn = 0;
		int32_t tl = 1;
		int32_t tln = 1;
		VS_MediaFormat mf;
		BitrateInfo bi;
		std::shared_ptr<VS_MediaPeerBase> mp;
		PeerScalablePayload() {};
		PeerScalablePayload(const avmuxer::LayoutFormat & lf_, std::uintptr_t h, uint32_t fourcc_, uint32_t mb_, const VS_MediaFormat & mf_, int32_t maxbr, int32_t ibr) : lf(lf_), handle(h), fourcc(fourcc_), mbmax(mb_), mbc(mb_), mbn(mb_), mf(mf_)
		{
			bi.max_snd_bitrate = maxbr;
			bi.snd_bitrate = ibr;
			bi.last_snd_bitrate = ibr;
		};
	};

	struct PartScalablePayload
	{
		/// svc
		std::atomic<bool> use_svc { false };
		std::atomic<int32_t> mixer_sl { 0 };
		std::atomic<int32_t> num_sl { 1 };
		std::atomic<int32_t> opt_sl { 0 };
		int32_t max_mb = 3600;
		std::vector<int32_t> threshold_mb;
		/// mixer size
		std::atomic<int32_t> mixer_mb { 900 };
		/// load decompressor
		std::atomic<int32_t> load_sl { 0 };
		uint64_t vt = 0;
		/// key frame manage
		std::atomic<bool> key_request { false };
		uint64_t keyt = 0;
		// role
		std::atomic<bool> podium { false };
		int32_t	role = 0;
		PartScalablePayload()
		{
			threshold_mb.resize(num_sl, 0);
		};
	};

	struct VideoPayload
	{
		std::list<std::shared_ptr<relay::PeerScalablePayload>> peers;
		eSchemeBitrate schemeBitrate = eSchemeBitrate::rough;
		int32_t realMixerBitrate = 0;
		std::vector<int32_t> layerCoefs;
		std::vector<int64_t> layerTemporalSize;
		std::map<uint32_t /* mb */, int64_t /* bytes */> layerSpatialSize;
		std::map<uint32_t /* mb */, int64_t /* bytes */> layerSpatialLastSize;
		std::map<uint32_t /* mb */, int32_t /* kbps */> mixerBitrate;
		std::map<uint32_t /* mb */, int32_t /* num peers */> mbPeers;
		float ktl = 0.6f;
		float ksl = 0.4f;
		int32_t numsl = 3;
		int32_t numtl = 2;
		uint64_t m_updateStatistic = 0;
	};

	struct PeerAudioPayload
	{
		avmuxer::LayoutFormat lf;
		std::uintptr_t handle = 0;
		uint32_t twocc = 0;
		uint32_t samplerate = 0;
		VS_MediaFormat mf;
		std::shared_ptr<VS_MediaPeerBase> mp;
		PeerAudioPayload() {};
		PeerAudioPayload(const avmuxer::LayoutFormat & lf_, std::uintptr_t h, uint32_t twocc_, uint32_t sr_, const VS_MediaFormat & mf_) : lf(lf_), handle(h), twocc(twocc_), samplerate(sr_), mf(mf_) {};
	};

	struct AudioPayload
	{
		std::list<std::shared_ptr<relay::PeerAudioPayload>> peers;
		std::map<uint32_t /* samplerate */, int32_t /* num peers */> srPeers;
	};

	struct MediaPayload
	{
		std::map<uint32_t /* fourcc */, boost::shared_ptr<relay::VideoPayload>> videoPayload;
		std::map<uint32_t /*  twocc */, boost::shared_ptr<relay::AudioPayload>> audioPayload;
	};

}

typedef std::unordered_map<avmuxer::LayoutFormat /* layout */, boost::shared_ptr<relay::MediaPayload>, avmuxer::LayoutFormatHasher> mapMediaStreamPool;
typedef std::map <std::string /* id peer */, std::shared_ptr<relay::PeerScalablePayload> > mapPeerScalablePool;
typedef std::map <std::string /* id part */, std::shared_ptr<relay::PartScalablePayload> > mapPartScalablePool;
typedef std::map <std::string /* id peer */, std::shared_ptr<relay::PeerAudioPayload> > mapPeerAudioPool;
typedef std::multimap <uint32_t /* mb */, std::string /* part id */> mapPartDS;
typedef std::pair<std::string /* part id */, bool /* force layout */> pairForceLayout;

class VS_RelayMediaSource :  VS_Lock,
                             public vs::enable_shared_from_this<VS_RelayMediaSource>,
							 public VS_PeerPoolItemObserver,
							 public VS_ConfMixerCallback,
							 public VS_ConfControlInterface,
							 public VS_TransmitFrameInterface
{
	typedef boost::signals2::signal<void (const char *, const char *part_name)> KeyFrameReqSig;
	typedef boost::signals2::signal<void (const char *, const char *, const long , const long, const long)>  RestrictBitrateSVCSig;
	typedef boost::signals2::signal<void(const char*, const char *, const char *)>	DataForWebPeerSig;

	KeyFrameReqSig			m_fireKeyFrameReq;
	RestrictBitrateSVCSig	m_fireRestrictBitrateSVC;
	DataForWebPeerSig		m_fireDataForWebPeer;

	avmuxer::ScalableProperty m_svcProperty;
	uint64_t m_updatePeersState = 0;

	std::recursive_mutex	m_mtxPeerScalableLock;
	std::recursive_mutex	m_mtxPartScalableLock;
	std::recursive_mutex	m_mtxPeerAudioLock;
	std::recursive_mutex	m_mtxPriorityList;
	std::recursive_mutex	m_mtxActiveDesktop;
	CMixerPool				m_av_mixer;
	mapMediaStreamPool		m_mediaStreamPool;
	mapPeerScalablePool		m_peerScalablePool;
	mapPartScalablePool		m_partScalablePool;
	mapPeerAudioPool		m_peerAudioPool;
	std::map <std::string, std::shared_ptr<VS_ParticipantPoolItem>>	m_part_pool;
	std::map <std::string, std::string>									m_partsByPeers;

	FILE					*m_logStat;
	VS_MediaFormat			m_mf;
	std::atomic<uint32_t>   m_svc_mode;
	VS_Conference_Type		m_conf_type;
	VS_GroupConf_SubType	m_conf_subtype;
	int						m_conf_bitrate;
	bool					m_sip_layout;
	bool					m_load_decompressor = true;
	avmuxer::LayoutFormat	m_fixedLayoutFormat;
	avmuxer::LayoutFormat	m_asymmetricLayoutFormat;
	std::string				m_owner_conf;
	std::string				m_podium_parts_hash;
	std::string				m_conf;
	std::map <std::string, uint32_t>									m_payloadAudio;
	std::map <std::string, uint32_t>									m_payloadVideo;
	std::map<VS_MediaPeer_Type /* peer type */, VS_MediaFormat /* media formats */> m_moduleFormat;
	int32_t m_videoQuality = 0;
	int32_t m_recordQuality = 0;
	int32_t m_webrtcQuality = 0;
	int32_t m_rtspQuality = 0;
	int32_t m_sipQuality = 0;

private:

	enum class performanceState : int8_t
	{
		ok = 0,
		warning = 1,
		danger = 2,
		critical = 3,
	};

	struct performanceInfo
	{
		performanceState state = performanceState::ok;
		uint64_t calculateDownTm = 0;
		uint64_t calculateUpTm = 0;
	};

	mapPartDS				m_mapPartDS;
	bool					m_enableDsLayouts = true;
	bool					m_enableVadLayouts = false;
	std::string				m_vadHandle;
	std::string				m_dsHandle;
	std::string				m_currHandle;
	performanceInfo			m_perfInfo;
	uint64_t				m_lastVadTime;

public:
	virtual ~VS_RelayMediaSource();

	void UpdateFilter(const char *conf_name, const char *part_name, const long fltr, const int32_t role);
	void SetParticipantDisplayname(const char *conf_name, const char *part_name, const char *displayname);
	bool PeerConnect(const std::shared_ptr<VS_MediaPeerBase> &item);
	void PeerDisconnect(const char *peer_name);
	void GenerateNewPeerId(std::string &peer_id);
	VS_Conference_Type GetConfType() const;
	VS_GroupConf_SubType GetConfSubType() const;
	std::string GetConfOwner() const;
	std::string GetPodiumPartsConf() const;
	void GetPeerMediaFormat(VS_MediaFormat *mf, VS_MediaPeer_Type type) const;

	boost::signals2::connection ConnectToKeyFrameReq(const VS_RelayMediaSource::KeyFrameReqSig::slot_type &slot)	/// request from tc participant
	{
		return m_fireKeyFrameReq.connect(slot);
	}
	boost::signals2::connection ConnectToRestrictBitrateSVC(const RestrictBitrateSVCSig::slot_type & slot)	//request from tc participant
	{
		return m_fireRestrictBitrateSVC.connect(slot);
	}
	boost::signals2::connection ConnectToDataForWebPeer(const DataForWebPeerSig::slot_type & slot)	// info for web peers
	{
		return m_fireDataForWebPeer.connect(slot);
	}

protected:
	VS_RelayMediaSource();
	static void PostConstruct(std::shared_ptr<VS_RelayMediaSource>& /*p*/) {}

public:
	void ManageLayout(const json::Object & data);
	void SetFixedLayout(const json::Object & data);
	void SetFixedLayout(const char *layout);
	void ResetFixedLayout();
	void SetAsymmetricLayout(const std::set<std::string> &view_peers);
	void ResetAsymmetricLayout();
	void SetSlideCmd(string_view conf, string_view from, string_view cmd);

private:
	/// VS_ConfControlInterface
	virtual void StartConference(const char *conf_name) override;
	virtual void StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)  override;
	virtual void StopConference(const char *conf_name)  override;
	virtual void ParticipantConnect(const char *conf_name, const char *part_name)  override;
	virtual void ParticipantDisconnect(const char *conf_name, const char *part_name)  override;
	virtual void SetParticipantCaps(const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long buf_sz) override;
	virtual void RestrictBitrateSVC(const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate) override;
	virtual void RequestKeyFrame(const char *conferenceName, const char *participantName) override;

private:
	/// VS_TransmitFrameInterface
	void TransmitFrame(const char *conf_name, const char *part_name, const stream::FrameHeader *frame_head, const void *frame_data) override;

private:
	////VS_PeerPoolItemObserver
	void KeyFrameRequestForPeer(const char *part_id, const char *peer_id) override;
	void SetRates(const char *part_id, const char *peer_name, unsigned int bitrate, unsigned int frame_rate) override;
	void SetSndFrameResolution(const char *part_id, const char *peer_name, const char *plname, uint8_t pltype, unsigned short width, unsigned short height) override;
	void ChangeAudioSendPayload(const char *part_id, const char *peer_id, const char *plname, int plfreq) override;
	void ChangeVideoSendPayload(const char *part_id, const char *peer_id, VS_MediaPeer_Type peer_type, const VS_MediaFormat& mf) override;
	void ChangeVideoSendPayload(const char *part_id, const char *peer_id, const VS_MediaFormat& mf, VS_MediaFormat &out, bool forceFormat) override;

private:
	void ReadyToDie(const char *part_id, const char *peer_id) override;

	/// VS_ConfMixerCallback interface
	void CallbackVideo(const avmuxer::LayoutPayload &payload, uint8_t* data, int size, uint8_t tl, bool key, uint32_t tm) override;
	void CallbackAudio(const avmuxer::LayoutFormat &format, uint32_t twocc, uint32_t samplerate, uint8_t* data, int size, uint32_t samples, uint32_t tm) override;
	void NewLayoutList(const std::uintptr_t handle, const avmuxer::LayoutControl &lc) override;
	void UpdateParticipantsMb(const std::map<std::string /* part id */, int32_t /* optimal part mb */ > &mbs) override;
	void UpdateParticipantKeyRequest(const std::string &id) override;
	void UpdateLoadDecompressor(const avmuxer::LoadStatistic &load) override;

private:
	void RequestKeyFrameFromParticipant(const std::string &id, relay::PartScalablePayload *part_payload);

private:
	void PrepareConference();
	void CleanConference();
	bool ParticipantPaired(string_view part_name) const;
	void TransmitFrame(const char* conf_name, const char* part_name, const void* frame_data, size_t size, stream::Track track, stream::TrackType type);
	void AddParticipantTrack(string_view part_name, stream::Track track, stream::TrackType type);
	void RemoveParticipantTrack(string_view part_name, stream::Track track, stream::TrackType type);
	avmuxer::LayoutFormat GetVideoLayoutFormat(VS_MediaPeerBase *peer, int32_t forceWidth, int32_t forceHeight);
	avmuxer::LayoutFormat GetAudioLayoutFormat(VS_MediaPeerBase *peer);
	boost::shared_ptr<relay::MediaPayload> CreateMixer(const avmuxer::LayoutFormat & lf, std::uintptr_t &handle);
	boost::shared_ptr<relay::VideoPayload> CreateVideoPayloadPeer(std::uintptr_t handle, uint32_t fourcc, std::map<uint32_t, boost::shared_ptr<relay::VideoPayload>> &vp, const avmuxer::LayoutFormat &lf);
	boost::shared_ptr<relay::AudioPayload> CreateAudioPayloadPeer(std::uintptr_t handle, uint32_t twocc, std::map<uint32_t, boost::shared_ptr<relay::AudioPayload>> &ap, const avmuxer::LayoutFormat &lf);
	bool CreateVideoStreamPeer(std::uintptr_t handle, boost::shared_ptr<relay::VideoPayload> &vp, VS_MediaFormat &mf, const std::shared_ptr<VS_MediaPeerBase> &mediaPeer, const avmuxer::LayoutFormat &lf);
	bool CreateAudioStreamPeer(std::uintptr_t handle, boost::shared_ptr<relay::AudioPayload> &ap, VS_MediaFormat &mf, const std::shared_ptr<VS_MediaPeerBase> &mediaPeer, const avmuxer::LayoutFormat &lf);
	std::uintptr_t GetHandleMixerSymmetric();
	void GetHandleMixersPriority(const std::string &idPriority, std::vector<std::uintptr_t> &handleMixers);
	int GetOptimalSpatialLayer(relay::PartScalablePayload *part_payload, uint8_t *pack, int size, const char *part_name, bool &key, bool &key_request, pairForceLayout &fl);
	pairForceLayout GetActiveDesktopCapture(const char *part_name, relay::PartScalablePayload *part_payload, int sl);
	pairForceLayout RemoveActiveDesktopCapture(const char *part_name);
	void UpdateBitratePeers(const uint64_t ct);
	void UpdateStatistics(relay::VideoPayload *mp, uint32_t mb, uint8_t tl, int size, const uint64_t ct);
	void UpdateLayersCoefs(relay::VideoPayload *mp);
	void UpdateMixerBitrate(relay::VideoPayload *mp, uint64_t dt);
	void LoggingVideoLayout(const relay::VideoPayload *mp, const std::uintptr_t handleMixer, uint32_t fourcc);
	void LoggingAudioLayout(const relay::AudioPayload *mp, const std::uintptr_t handleMixer, uint32_t twocc);
	void LoggingMixerLayout(const std::uintptr_t handleMixer, const avmuxer::LayoutFormat *lf);
	std::shared_ptr<relay::PeerScalablePayload> RemoveVideoPeerPayload(const char *peer_id, uint32_t fourcc, uint8_t &op);
	std::shared_ptr<relay::PeerAudioPayload> RemoveAudioPeerPayload(const char *peer_id, uint32_t twocc, uint8_t &op);
	void MixerVideoPoolClean(const uint8_t &op, std::uintptr_t handle, uint32_t fourcc, uint32_t mb);
	void MixerAudioPoolClean(const uint8_t &op, std::uintptr_t handle, uint32_t twocc, uint32_t samplerate);
	void TakePodiumParticipant(const std::string & id);
	void LeavePodiumParticipant(const std::string & id);
	void RestrictMediaFormat(VS_MediaFormat &mf, VS_MediaPeer_Type type) const;

private:
		void StopSlide(string_view conf_name, string_view part_name);
		void UpdateSlide(string_view conf_name, string_view part_name, string_view url);

private:
	void ChangePriority(pairForceLayout fl);
	void ResetPriority();
	std::shared_ptr<VS_MediaPeerBase> GetPeer(const std::string & id);
	std::shared_ptr<relay::PeerScalablePayload> GetVideoPeer(const std::string & id);
	std::shared_ptr<relay::PeerAudioPayload> GetAudioPeer(const std::string & id);
	bool CheckPeerPodium(const std::string & id);
	void ChangePeerLayoutFormat(const relay::PeerScalablePayload *videoPeer, const relay::PeerAudioPayload *audioPeer);
	void ChangePeerLayoutFormat(const std::string & id);
	void ChangePeersLayoutFormat();
	void ChangeVideoLayoutFormat(const avmuxer::LayoutFormat & lf, const std::string& peer_id);
	void ChangeAudioLayoutFormat(const avmuxer::LayoutFormat & lf, const std::string& peer_id);
	void SetFixedLayoutFormat(const json::Object & data);
	void ResetLayoutFormat(avmuxer::LayoutFormat *lf);
	bool ParseJsonMessage(const json::Object & data, avmuxer::LayoutControl &lc);
	bool CreateJsonMessage(const avmuxer::LayoutControl &lc, json::Object & data);

private:
	/// tests
	void TestChangePriority();
	void TestAsymmetricLayout();
	void TestSlide();

};

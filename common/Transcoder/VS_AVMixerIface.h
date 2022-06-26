
#ifndef VS_AVMIXERIFACE_H
#define VS_AVMIXERIFACE_H

class VS_ConferenceMixer;
class ThreadDecompressor;
class VS_MixerReceiver;
class MediaSynch;

#include "mixer/VS_MultiMixerVideo.h"
#include "../streams/fwd.h"
#include "../std/cpplib/VS_Lock.h"
#include <vector>
#include <string>
#include <map>
#include <set>

namespace avmuxer {

	inline uint64_t getTickMs() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	enum class VideoLayout : int8_t {
		none = -1,
		symmetric = 0,
		sip = 1,
		special = 2,
		asym = 3
	};

	struct LayoutFormat
	{
		std::size_t hash = 0;
		VideoLayout layout = VideoLayout::none;
		std::set<std::string> peers;
		VSLayoutDesc mixerDesc;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mb = 0;
		LayoutFormat() {};
		LayoutFormat(VideoLayout layout_, const std::set<std::string> & peers_) : layout(layout_), peers(peers_), width(0), height(0), mb(0) { Hash(); };
		LayoutFormat(VideoLayout layout_, const std::set<std::string> & peers_, uint32_t w, uint32_t h) : layout(layout_), peers(peers_), width(w), height(h) { mb = width * height / 256; Hash(); }
		bool IsContainedPeer(const std::string & peer, bool audio = false) const
		{
			if (layout == VideoLayout::none) {
				return false;
			} else if (layout == VideoLayout::symmetric) {
				return true;
			}
			else if (layout == VideoLayout::sip) {
				return (!audio && (peers.find(peer) == peers.end()));
			}
			else if (layout == VideoLayout::special || layout == avmuxer::VideoLayout::asym) {
				return (peers.find(peer) != peers.end());
			}
			else {
				assert(0);
			}
			return false;
		}
		void Hash()
		{
			hash = 0;
			boost::hash_combine(hash, static_cast<int8_t>(layout));
			boost::hash_range(hash, peers.begin(), peers.end());
			boost::hash_combine(hash, width);
			boost::hash_combine(hash, height);
			boost::hash_combine(hash, mixerDesc.Hash());
		}
		bool operator == (const LayoutFormat& other) const
		{
			assert(hash != 0 && other.hash != 0);
			return hash == other.hash;
		}
		bool operator != (const LayoutFormat& other) const
		{
			assert(hash != 0 && other.hash != 0);
			return hash != other.hash;
		}
	};

	struct LayoutPayload
	{
		LayoutFormat lf;
		uint32_t fourcc = 0;
		uint32_t mb = 0;
		LayoutPayload() {};
		LayoutPayload(const LayoutFormat & lf_, uint32_t fourcc_, uint32_t mb_) : lf(lf_), fourcc(fourcc_), mb(mb_) {};
	};

	struct LayoutFormatHasher
	{
		std::size_t operator()(const LayoutFormat& k) const
		{
			assert(k.hash != 0);
			return k.hash;
		}
	};

	struct ScalableProperty
	{
		struct SpatialProperty
		{
			float k = 0.0;
			int32_t w = 0;
			int32_t h = 0;
			int32_t bitrate = 0;
		};
		std::map<uint32_t /* mb */, SpatialProperty> mbSpatial;
		std::vector<uint32_t /* mb */> sl2mb;
		std::vector<uint32_t /* samplerate */> srAudio;
		void Clear() {
			mbSpatial.clear();
			sl2mb.clear();
			srAudio.clear();
		}
	};

	struct LoadStatisticItem
	{
		float load = 0.0f;
		uint64_t vt = 0;
		uint64_t at = 0;
		uint64_t vp = 0;
	};

	struct LoadStatistic
	{
		LoadStatisticItem li;
		std::vector<std::pair<std::string /* rcv id */, LoadStatisticItem>> mli;
		uint64_t dt = 0;
		void Reset() {
			li.load = 0.0f;
			li.vt = 0;
			li.at = 0;
			li.vp = 0;
			dt = 0;
			mli.clear();
		}
	};

	struct LayoutControl
	{
		std::string function;
		std::string userId1;
		std::string userId2;
		std::string toPeer;
		VS_WindowGrid::ePriorityLayoutType type = VS_WindowGrid::PRIORITY_LAYOUT_CORNER;
		std::vector<VS_WindowGrid::sOneWindow> grid;
		int32_t mixerWidht = 0;
		int32_t mixerHeight = 0;
	};

	const std::string GetLayoutFunction = "Get";
	const std::string SwapLayoutFunction = "Swap";
	const std::string SetP0LayoutFunction = "SetP0";
	const std::string SetP1LayoutFunction = "SetP1";
	const std::string SetP2LayoutFunction = "SetP2";
	const std::string PriorityTypeLayoutFunction = "PriorityType";

}

class  VS_ConfMixerCallback
{
public:
	VS_ConfMixerCallback() {};
	virtual ~VS_ConfMixerCallback() {};
	virtual void CallbackVideo(const avmuxer::LayoutPayload &payload, uint8_t* data, int size, uint8_t tl, bool key, uint32_t tm) = 0;
	virtual void CallbackAudio(const avmuxer::LayoutFormat &format, uint32_t twocc, uint32_t samplerate, uint8_t* data, int size, uint32_t samples, uint32_t tm) = 0;
	virtual void NewLayoutList(const std::uintptr_t handle, const avmuxer::LayoutControl &lc) = 0;
	virtual void UpdateParticipantsMb(const std::map<std::string /* part id */, int32_t /* optimal part mb */ > &mbs) = 0;
	virtual void UpdateParticipantKeyRequest(const std::string & /* part id */) = 0;
	virtual void UpdateLoadDecompressor(const avmuxer::LoadStatistic &load) = 0;
};

#endif
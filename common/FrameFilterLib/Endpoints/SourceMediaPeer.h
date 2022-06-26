#pragma once

#include "FrameFilterLib/Base/AbstractSource.h"
#include "TransceiverCircuit/VS_MediaPeerBase.h"

class VS_RelayMediaSource;

namespace ffl {
	class SourceMediaPeer : public VS_MediaPeerBase, public AbstractSource
	{
	public:
		static std::shared_ptr<SourceMediaPeer> Create(const char* peer_id, const char* part_id, const std::shared_ptr<VS_RelayMediaSource>& media_source);

		SourceMediaPeer(const char* peer_id, const char* part_id, const std::shared_ptr<VS_RelayMediaSource>& media_source);
		~SourceMediaPeer();

		bool ProcessCommand(const FilterCommand& cmd) override;

		void SetAudioFormat(const VS_MediaFormat& mf);
		void SetVideoFormat(const VS_MediaFormat& mf, bool force);
		void SetBitrate(unsigned int bitrate);
		void Stop();

		void PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned timestamp) override;
		void PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int samples, unsigned int timestamp) override;
		void PutUncompressedAudio(unsigned char *buf, unsigned long sz, unsigned int timestamp) override;
		void ReceiveAudio(const char *peer_name, const unsigned char *buf, int sz, unsigned int timestamp) override;
		void ChangeAudioRcvPayload(const char *plname, const int plfreq) override;
		void ReceiveVideo(const char *peer_name, const char *stream_id, const unsigned char *pFrame, int size, bool isKey, unsigned int timestamp) override;
		void ChangeRcvFrameResolution(const char *peer_name, const char *stream_id, const char *plname, uint8_t pltype, unsigned short width, unsigned short height) override;

	private:
		std::weak_ptr<VS_RelayMediaSource> m_media_source;
		bool m_force_video_format;
		uint32_t m_last_timestamp;
	};
}
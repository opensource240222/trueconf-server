#pragma once

#include <cstddef>
#include <cstdint>

class VS_MediaFormat;

namespace stream {

#pragma pack(push, 1)
struct Command
{
	static const size_t header_size = 4;

	enum class Type : uint8_t
	{
		RequestKeyFrame    = 1,
		RestrictBitrate    = 2,
		SetFPSvsQ          = 3,
		ChangeSndMFormat   = 4,
		ChangeRcvMFormat   = 5,
		Stat               = 6,
		RequestPacket      = 7,
		TimeDelay          = 8,
		Ping               = 9,
		BrokerStat         = 10,
		RestrictBitrateSVC = 11,
	};
	Type type;
	static const int last_type = 11;

	enum SubType : uint8_t
	{
		Request = 1,
		Reply   = 2,
		Ack     = 3,
		Info    = 4,
	};
	SubType sub_type;

	enum Result : uint8_t
	{
		OK = 0,
		UnknownError = 255,
	};
	Result result;

	uint8_t data_size;
	uint8_t data[255];

	// cppcheck-suppress uninitMemberVar symbolName=Command::data ; user code should always check data_size before accessing data
	Command()
		: type(static_cast<Type>(0))
		, sub_type(static_cast<SubType>(0))
		, result(static_cast<Result>(0))
		, data_size(0)
	{
	}
	Command(const void* buffer, size_t size);

	Command(const Command&);
	Command& operator=(const Command&);

	// Clear state
	void Clear()
	{
		type = {};
		sub_type = {};
		result = {};
		data_size = 0;
	}
	// Return actual size of command
	size_t Size() const
	{
		return header_size + data_size;
	}

	// Request Key Frame from video compressor
	void RequestKeyFrame();
	// Restrict bitrate to point value
	void RestrictBitrate(uint32_t value);
	// Set FPS vs quality factor
	void SetFPSvsQ(uint32_t value);
	// Request change of send media format
	void ChangeSndMFormat(const VS_MediaFormat& mf);
	// Request permission to change our send media format
	void ChangeRcvMFormat(const VS_MediaFormat& mf);
	// Notify about change in our send media format
	void InfoRcvMFormat(const VS_MediaFormat& mf);
	// Nhp stat
	void Stat(const void* p, size_t size);
	// Request to packet resend
	void RequestPacket(const void* p, size_t size);
	// Request to determine time delay
	void TimeDelay(uint32_t id);
	// Pure ping, do nothing
	void Ping();
	// Inner comand to sent broker statistics from receiver to sender
	void BrokerStat(const void* p, size_t size);
	// Restrict bitrate for SVC
	void RestrictBitrateSVC(uint32_t bitrate, uint32_t video_bitrate);

	// Form a reply to this command
	void MakeReply(Result result_ = OK)
	{
		sub_type = Reply;
		result = result_;
	}
};
#pragma pack(pop)

}

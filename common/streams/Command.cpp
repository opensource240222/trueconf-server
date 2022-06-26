#include "Command.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "../std/cpplib/numerical.h"

#include <algorithm>
#include <cstring>

namespace stream {

Command::Command(const void* buffer, size_t size)
{
	if (!buffer || size < header_size)
	{
		Clear();
		return;
	}

	const auto p_cmd = static_cast<const Command*>(buffer);
	type = p_cmd->type;
	sub_type = p_cmd->sub_type;
	result = p_cmd->result;
	data_size = p_cmd->data_size;
	const auto real_data_size = std::min<size_t>(data_size, size - header_size);
	std::memcpy(data, p_cmd->data, real_data_size);
	if (real_data_size < data_size)
		std::memset(data + real_data_size, 0, data_size - real_data_size);
}

Command::Command(const Command& x)
	: type(x.type)
	, sub_type(x.sub_type)
	, result(x.result)
	, data_size(x.data_size)
{
	std::memcpy(data, x.data, data_size);
}

Command& Command::operator=(const Command& x)
{
	type = x.type;
	sub_type = x.sub_type;
	result = x.result;
	data_size = x.data_size;
	std::memcpy(data, x.data, data_size);
	return *this;
}

void Command::RequestKeyFrame()
{
	type = Type::RequestKeyFrame;
	sub_type = Info;
}

void Command::RestrictBitrate(uint32_t value)
{
	type = Type::RestrictBitrate;
	sub_type = Request;
	data_size = sizeof(uint32_t);
	*reinterpret_cast<uint32_t*>(data) = value;
}

void Command::SetFPSvsQ(uint32_t value)
{
	type = Type::SetFPSvsQ;
	sub_type = Request;
	data_size = sizeof(uint32_t);
	*reinterpret_cast<uint32_t*>(data) = value;
}

void Command::ChangeSndMFormat(const VS_MediaFormat& mf)
{
	type = Type::ChangeSndMFormat;
	sub_type = Request;
	data_size = sizeof(VS_MediaFormat);
	std::memcpy(data, &mf, data_size);
}

void Command::ChangeRcvMFormat(const VS_MediaFormat& mf)
{
	type = Type::ChangeRcvMFormat;
	sub_type = Request;
	data_size = sizeof(VS_MediaFormat);
	std::memcpy(data, &mf, data_size);
}

void Command::InfoRcvMFormat(const VS_MediaFormat& mf)
{
	type = Type::ChangeRcvMFormat;
	sub_type = Info;
	data_size = sizeof(VS_MediaFormat);
	std::memcpy(data, &mf, data_size);
}

void Command::Stat(const void* p, size_t size)
{
	type = Type::Stat;
	sub_type = Info;
	data_size = clamp_cast<uint8_t>(std::min(static_cast<size_t>(size), sizeof(data)));
	std::memcpy(data, p, data_size);
}

void Command::RequestPacket(const void* p, size_t size)
{
	type = Type::RequestPacket;
	sub_type = Info;
	data_size = clamp_cast<uint8_t>(std::min(static_cast<size_t>(size), sizeof(data)));
	std::memcpy(data, p, data_size);
}

void Command::TimeDelay(uint32_t id)
{
	type = Type::TimeDelay;
	sub_type = Request;
	data_size = sizeof(uint32_t);
	*reinterpret_cast<uint32_t*>(data) = id;
}

void Command::Ping()
{
	type = Type::Ping;
	sub_type = Info;
}

void Command::BrokerStat(const void* p, size_t size)
{
	type = Type::BrokerStat;
	sub_type = Info;
	data_size = clamp_cast<uint8_t>(std::min(static_cast<size_t>(size), sizeof(data)));
	std::memcpy(data, p, data_size);
}

void Command::RestrictBitrateSVC(uint32_t bitrate, uint32_t video_bitrate)
{
	type = Type::RestrictBitrateSVC;
	sub_type = Request;
	data_size = 2 * sizeof(uint32_t);
	reinterpret_cast<uint32_t*>(data)[0] = bitrate;
	reinterpret_cast<uint32_t*>(data)[1] = video_bitrate;
}

}

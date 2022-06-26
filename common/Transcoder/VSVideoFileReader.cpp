#include "VSVideoFileReader.h"

VSVideoFileReader::VSVideoFileReader()
{
}

VSVideoFileReader::~VSVideoFileReader()
{
}

bool VSVideoFileReader::Init(std::string fileName)
{
	if (avformat_open_input(&m_FormatContext,
		fileName.c_str(), NULL, NULL) < 0)
		return false;

	if (avformat_find_stream_info(m_FormatContext, 0) < 0)
	{
		avformat_free_context(m_FormatContext);

		return false;
	}

	for (unsigned int i = 0; i < m_FormatContext->nb_streams; i++)
	{
		switch (m_FormatContext->streams[i]->codecpar->codec_type)
		{
			case AVMEDIA_TYPE_VIDEO:
			{
				m_VideoStream = m_FormatContext->streams[i];
				break;
			}

			case AVMEDIA_TYPE_AUDIO:
			{
				m_AudioStream = m_FormatContext->streams[i];
				break;
			}
		}
	}

	return true;
}

void VSVideoFileReader::Release()
{
	if (m_FormatContext)
	{
		avio_closep(&m_FormatContext->pb);

		avformat_free_context(m_FormatContext);
	}

	m_FormatContext = nullptr;
	m_AudioStream = nullptr;
	m_VideoStream = nullptr;
	m_CurrVideoTime = 0;
	m_CurrAudioTime = 0;
}

int VSVideoFileReader::ReadVideo(void* data, bool* isKey)
{
	if (!m_VideoStream)
		return -1;

	SPacket packet;

	// We need next frame to compute m_CurrVideoTime
	while (m_VideoQueue.size() < 2)
	{
		if (!ReadNextFrameToQueue())
		{
			// But if we can't read last frame when we
			// have frame in queue, it is not error
			if (m_VideoQueue.size())
				break;
			else
				return -1;
		}
	}

	packet = m_VideoQueue.front();

	m_VideoQueue.pop();

	memcpy(data, packet.Data, packet.Size);

	delete[] packet.Data;

	if (isKey)
		*isKey = packet.IsKey;

	if (m_VideoQueue.size())
		m_CurrVideoTime = m_VideoQueue.front().timeStamp; // set cuurent time to time when next frame started
	else
		m_CurrVideoTime += m_FPS; // approximation

	return packet.Size;
}

int VSVideoFileReader::ReadAudio(void* data)
{
	if (!m_AudioStream)
		return -1;

	SPacket packet;

	// We need next frame to compute m_CurrVideoTime
	while (m_AudioQueue.size() < 2)
	{
		if (!ReadNextFrameToQueue())
		{
			// But if we can't read last frame when we
			// have frame in queue, it is not error
			if (m_AudioQueue.size())
				break;
			else
				return -1;
		}
	}

	packet = m_AudioQueue.front();

	m_AudioQueue.pop();

	memcpy(data, packet.Data, packet.Size);

	delete[] packet.Data;

	if (m_AudioQueue.size())
		m_CurrAudioTime = m_AudioQueue.front().timeStamp; // set cuurent time to time when next frame started
	else
		m_CurrAudioTime += m_AudioStream->codecpar->frame_size * 1000 / m_AudioStream->codecpar->sample_rate; // approximation

	return packet.Size;
}

unsigned int VSVideoFileReader::GetVideoDuration() const
{
	return (unsigned int)av_rescale_q(
		m_FormatContext->duration,
		av_make_q(1, AV_TIME_BASE),
		av_make_q(1, 1000)
	);
}

unsigned int VSVideoFileReader::GetAudioDuration() const
{
	if (m_AudioStream)
		return (unsigned int)av_rescale_q(m_AudioStream->duration, m_AudioStream->time_base, av_make_q(1, 1000));
	else
		return 0;
}

bool VSVideoFileReader::SeekToTime(int ms)
{
	uint64_t timeStamp;

	timeStamp = uint64_t(ms) * 1000;

	if (av_seek_frame(m_FormatContext, -1, timeStamp, AVSEEK_FLAG_BACKWARD) < 0)
		return false;

	// We need to get first frame after setted time
	// First, clear queues
	while (m_AudioQueue.size())
	{
		delete[] m_AudioQueue.front().Data;
		m_AudioQueue.pop();
	}

	while (m_VideoQueue.size())
	{
		delete[] m_VideoQueue.front().Data;
		m_VideoQueue.pop();
	}

	if (m_VideoStream)
	{
		while (!m_VideoQueue.size())
		{
			if (!ReadNextFrameToQueue())
				break;
		}
	}

	if (m_AudioStream)
	{
		while (!m_AudioQueue.size())
		{
			if (!ReadNextFrameToQueue())
				break;
		}
	}

	if (m_AudioQueue.size())
		m_CurrAudioTime = m_AudioQueue.front().timeStamp;

	if (m_VideoQueue.size())
		m_CurrVideoTime = m_VideoQueue.front().timeStamp;

	return true;
}

bool VSVideoFileReader::ReadNextFrameToQueue()
{
	AVPacket pkt;
	SPacket packetToQueue;

	if (av_read_frame(m_FormatContext, &pkt) < 0)
		return false;

	// If readed frame not associated with our streams (e. g. subtitle-frame)
	if (!(
		(m_VideoStream && pkt.stream_index == m_VideoStream->index) ||
		(m_AudioStream && pkt.stream_index == m_AudioStream->index)
		))
	{
		av_free_packet(&pkt);

		return true; // We don't need this frame, but it is not error
	}

	packetToQueue.Size = pkt.size;
	packetToQueue.Data = new uint8_t[packetToQueue.Size];

	memcpy(packetToQueue.Data, pkt.data, pkt.size);

	if (m_VideoStream && pkt.stream_index == m_VideoStream->index)
	{
		packetToQueue.IsKey = (pkt.flags & AV_PKT_FLAG_KEY) ? true : false;
		packetToQueue.timeStamp = (unsigned int)av_rescale_q(pkt.pts, m_VideoStream->time_base, av_make_q(1, 1000));

		m_VideoQueue.push(packetToQueue);
	}
	else if (m_AudioStream && pkt.stream_index == m_AudioStream->index)
	{
		packetToQueue.IsKey = true;
		packetToQueue.timeStamp = (unsigned int)av_rescale_q(pkt.pts, m_AudioStream->time_base, av_make_q(1, 1000));

		m_AudioQueue.push(packetToQueue);
	}

	av_free_packet(&pkt);

	return true;
}

#pragma once

#include <cstdint>

namespace stream {

enum class Track : uint8_t;
enum class TrackType : uint8_t;
enum class ClientType : unsigned;

struct FrameHeader;
struct UDPFrameHeader;
struct SliceHeader;
struct SVCHeader;
struct Command;

struct TrackStatistics;
struct StreamStatistics;
struct ParticipantStatistics;
struct ParticipantBandwidthInfo;
struct ConferenceStatistics;
struct RouterStatistics;

class ParticipantStatisticsInterface;
class ConferencesConditions;

class Buffer;
class DefaultBuffer;
class Router;
class SVCBuffer;

struct ParticipantLoadInfo;
struct ParticipantFrameSizeInfo;

}

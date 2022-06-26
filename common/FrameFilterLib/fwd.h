#pragma once

namespace ffl {

class AbstractSource;
class AbstractSink;
class AbstractMultiSourceSink;
class AbstractSingleSourceSink;
template <class Sink>
class AbstractFilter;
template <class Sink>
class AbstractOrderedFilter;
template <class Sink>
class AbstractThreadedFilter;

struct FilterCommand;
struct FilterFormat;
struct FrameMetadata;

class TraceLog;

class SinkFECCChannel;
class SinkRTPChannel;
class SinkTransceiverParticipant;

class SourceFECCChannel;
class SourceMediaPeer;
class SourceRTPChannel;
class SourceStaticImage;
class SourceTransceiverParticipant;

class FilterActivityMonitor;
class FilterAudioAdjuster;
class FilterAudioDecoder;
class FilterAudioEncoder;
class FilterAudioFormatReader;
class FilterAudioTranscoder;
class FilterAudioVideoJoiner;
class FilterChangeFormatCommandFixer;
class FilterChangeFormatCommandInjector;
class FilterH264SpsPpsInjector;
class FilterH264StreamLayoutInjector;
class FilterKeyFrameRequester;
class FilterKeyFrameRequestLimiter;
class FilterNOP;
class FilterRTPSorter;
class FilterRTPUnwrapper;
class FilterRTPValidator;
class FilterRTPWrapper;
class FilterSlideEncoder;
class FilterSourceSwitcher;
class FilterStatisticsCalculator;
class FilterUniformTransmit;
class FilterVideoAdjuster;
class FilterVideoDecoder;
class FilterVideoEncoder;
class FilterVideoFormatReader;
class FilterVideoTranscoder;
class FilterVideoTranscoderWithResolutionLimits;
class FilterVSFrameSlicer;
class FilterVSFrameUnwrapper;
class FilterVSFrameWrapper;

class FilterDumpAudio;
class FilterDumpRTP;
class FilterDumpVideo;

}

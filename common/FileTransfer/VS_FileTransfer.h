#pragma once

#include <string>
#include <vector>
#include <memory>

#include "std-generic/cpplib/string_view.h"

class VS_FileTransfer
{
public:
	struct FileInfo
	{
		std::string name;
		std::uint64_t size;
		std::uint64_t done;
		time_t completedTime;
		int downloadRate;
		int uploadRate;
		int numPeers;
	};

	struct Info
	{
		std::vector<FileInfo> files;

		std::string name;
		std::string path;

		std::string infoHash;
		std::string lastError;
		std::int64_t totalDownload;
		std::int64_t totalUpload;
		std::int64_t totalPayloadDownload;
		std::int64_t totalPayloadUpload;
		std::int64_t totalWanted;
		std::int64_t totalWantedDone;
		time_t completedTime;

		int downloadRate;
		int uploadRate;

		int piecesTotal;
		int piecesIndexed;
		int numPeers;

		bool unchanged;
		bool paused;
		bool autoManaged;
		bool sequentialDownload;
		bool isSeeding;
		bool isFinished;
		bool isIndexing;

		Info();
	};

	struct Endpoint
	{
		std::string ip;
		unsigned short port;
	};

	struct Control
	{
		void ConfirmDownload(bool result, std::string path);
		void Remove(bool deleteFiles);
		void Connect(Endpoint address);
		void Pause();
		void Resume();
		void GetStatistics(Info &info, bool includeFilesStats = true);
		void AddTrackers(const std::vector<std::string> &uris);
		void AddTrackersFromURI(string_view uri);

		Control(std::string hash, void *impl)
			: m_impl(impl)
			, m_hash(std::move(hash))
		{}
	private:
		void *m_impl;
		std::string m_hash;
	};

	enum eFlags { NAT_PMP = 1, UPNP = 2, DHT = 4, LSD = 8, LockFiles = 16 };

	enum eErrorCode { FileError };
public:
	static constexpr unsigned int MAX_FILE_POOL_SIZE = 500;
	static const int DEFAULT_FLAGS = NAT_PMP | UPNP /*| DHT*/ | LSD;
	struct Events
	{
		virtual void onReadyToSend(const std::string &magnet, const std::string &to) = 0;
		virtual void onPeerConnected(const std::string &info_hash, const Endpoint &endpoint) = 0;
		virtual bool onAskUser(const std::string &hash, const std::string &filename, const std::string &from, uint64_t fileSize) = 0;
		virtual void onMetadataSignal(const std::string &infoHash, const std::string &to) = 0;
		virtual void onError(const std::string &/*info_hash*/, eErrorCode /*err*/) = 0;
	};

public:
	static std::string hash_str_from_uri(string_view uri);
	static std::string name_from_uri(string_view uri);
public:
	VS_FileTransfer(std::weak_ptr<Events> eventsListener) : m_eventsListener(std::move(eventsListener)) {}
	VS_FileTransfer(const VS_FileTransfer &other) = delete;
	VS_FileTransfer(VS_FileTransfer &&other) noexcept = default;
	VS_FileTransfer &operator=(const VS_FileTransfer &other) = delete;
	VS_FileTransfer &operator=(VS_FileTransfer &&other) noexcept = default;
	virtual ~VS_FileTransfer() = default;

	virtual Control GetControl(std::string hash) = 0;
	virtual void Shutdown() noexcept = 0;

	virtual unsigned short ListenPort() const = 0;
	virtual std::string PeerId() const = 0;
	virtual void SendFile(std::string path, std::string to, std::vector<std::string> trackers = {}) = 0;
	virtual void GetStatistics(std::vector<Info> &info, bool includeFilestats = true) = 0;
	virtual int GetServerProgress(string_view info) = 0;
	virtual uintmax_t GetServerProgressBytes(const std::string &info) = 0;
	virtual void StartDownload(std::string magnet, std::string from, std::string desPath = {}) = 0;
	virtual void SetLimits(unsigned int dwnLim, unsigned int upLim) = 0;
	virtual void SetFilePoolSize(unsigned int maxSize) = 0;

	static std::unique_ptr<VS_FileTransfer> MakeFileTransfer(const char* workdir, unsigned int filePoolSize,
		int flags, std::weak_ptr<Events> eventsListener);
protected:
	std::weak_ptr<Events> m_eventsListener;
};

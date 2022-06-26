#pragma once

#include <chrono>
#include <utility>
#include <vector>

#include "std-generic/compat/memory.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"
#include "std-generic/compat/set.h"
#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/VS_UserData.h"

#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "FileTransfer/VS_FileTransfer.h"
#include "http/handlers/Interface.h"
#include "acs_v2/Handler.h"

class VS_TRStorageInterface;

enum VS_TorrentResult{
	TR_NO_ERROR = 0,
	TR_MESSAGE_WITHOUT_MAGNET,
	TR_P2P_ONLY,
	TR_UNKNOWN_ERROR,
	TR_INVALID_FILELIST,
};

class VS_TorrentService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_FileTransfer::Events,
	public acs::Handler,
	public http::handlers::Interface
{
public:
	typedef std::chrono::steady_clock	clock_t;
	typedef clock_wrapper<clock_t>		clock_wrapper_t;

protected:
	VS_TorrentService(boost::asio::io_service &ios, const std::string &ourEndpoint,
		const std::string &fileStorageDir,
		const std::shared_ptr<VS_TRStorageInterface> &dbStorage,
		const std::function<std::string(const char *)> &getServerId);
	static void PostConstruct(std::shared_ptr<VS_TorrentService>& ptr);
public:
	void InitTrackersProperty(vs::set<std::pair<std::string, net::port>> trackersList);
	void ClearTrackersProperty();
	net::port ListenPort() const { return m_torrent->ListenPort(); }

	boost::optional<std::string> HandleRequest(string_view in, const net::address &fromAddr, const net::port fromPort) override;

protected:
	acs::Response Protocol(const stream_buffer& buffer, unsigned channelToken) override;
	void Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer) override;

	clock_wrapper_t& clock() const noexcept { return m_clock; }

private:
	struct TrackerRecord final
	{
		struct { net::address addr; net::port port; } endpoint;
		std::chrono::seconds expire;
		unsigned short numAnnounces;
		std::string peerId;
		bool complete;
	};

	typedef std::string DownloadEntry;

	struct TorrentState final
	{
		std::string infoHash;
		std::string magnet;
		VS_RealUserLogin lastInitiator;
		bool isFinished;
		bool isAccepted; // torrent came from share method
		std::vector<TrackerRecord> endpoints;
		std::vector<DownloadEntry> entries;
		std::string idParam;
		std::string httpLink;
		std::chrono::steady_clock::time_point lastChanged;
		std::chrono::steady_clock::time_point lastActive;
		time_t completionTime;
		int fileErrors;

		TorrentState(std::string infoHash) : infoHash(std::move(infoHash)), isFinished(false), isAccepted(false), completionTime(0), fileErrors(0) {}
	};

private:
	bool m_init; //thread safe because it is called through the strand (called by the Timer thread)

	bool Init(const char* /*ourEndpoint*/, const char* /*ourService*/, bool /*permittedAll*/) override
	{
		// nothing
		return true;
	}

	bool Timer(std::chrono::milliseconds tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void Share_Method(VS_Container &cnt);

	void onReadyToSend(const std::string &/*magnet*/, const std::string &/*to*/) override;

	void onPeerConnected(const std::string &/*infoHash*/, const VS_FileTransfer::Endpoint &/*endpoint*/) override;

	void onError(const std::string &infoHash, VS_FileTransfer::eErrorCode error) override;
	bool onAskUser(const std::string &infoHash, const std::string &filename, const std::string &from, uint64_t size) override;
	void onMetadataSignal(const std::string &infoHash, const std::string &to) override;

	boost::optional<std::string> ProcessTrackerRequest(string_view in, const net::address &fromAddr);

	std::string MakeTrackerResponse(const std::string &infoHash, const std::string &peerId, const std::string &evt, const net::address &fromAddr, net::port port, bool noPeerId);

	void SendError(const char *to, VS_TorrentResult res);
	void ResumeDownloads();

	void PostRequest(const char *toUser, const VS_Container& cnt);

	const std::string& FileStorageDir() const;

	// internal help methods, call only in service thread
	void CheckTorrents(const std::vector<VS_FileTransfer::Info>  &info);
	void MarkUselessEntities();
	void CheckUselessEntities();
	void RemoveUselessEntities();

private:
	typedef vs::fast_mutex mutex_t;

	boost::asio::io_service &m_ios;

	vs::map<std::string, std::shared_ptr<TorrentState>, vs::str_less> m_state;
	mutex_t m_stateLock;

	std::shared_ptr<VS_TRStorageInterface> m_dbStorage;
	std::unique_ptr<VS_FileTransfer> m_torrent;

	std::string m_ourEndpoint;
	std::string m_fileStorageDir;

	std::vector<std::string> m_notDeletedYet;
	vs::set<std::pair<std::string, net::port>> m_listenAddrs;

	std::function<std::string(const char *)> m_getServerId;

	std::chrono::milliseconds m_lastCheckTime;
	std::chrono::milliseconds m_lastCleanTime;
	std::chrono::milliseconds m_lastDeleteTime;
	const std::chrono::seconds m_checkUselessTimeout;

	net::port m_cacheListenPort; // thread safe because it is called through the strand

	mutable clock_wrapper_t m_clock;
};

#include "VS_FileTransfer.h"

#include <thread>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/join.hpp>

#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/debuglog/VS_Debug.h"

#include "libtorrent/announce_entry.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/create_torrent.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/extensions/ut_pex.hpp"
#include "libtorrent/peer_info.hpp"
#include "utils.h"

#include "VS_LibTorrentPlugin.h"

#if (LIBTORRENT_VERSION_NUM < 10200) /*< 1.2.0*/ && !defined(TORRENT_NO_DEPRECATE)
/**********************************************************************************/
// Build fix for boost 1.58, 1.59
// Bug: https://svn.boost.org/trac/boost/ticket/11702

#include <boost/version.hpp>

#if BOOST_VERSION >= 105800 && BOOST_VERSION < 106000
#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
namespace boost {
	namespace _bi {
		template<class A> struct list_add_cref<std::auto_ptr<A> > {
			typedef std::auto_ptr<A> &type;
		};
	}
}
#endif // !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
#endif // BOOST_VERSION >= 105800 && BOOST_VERSION < 106000
/**********************************************************************************/
#endif //(LIBTORRENT_VERSION_NUM < 10200) /*< 1.2.0*/ && !defined(TORRENT_NO_DEPRECATE)


#if (LIBTORRENT_VERSION_NUM < 10200) // < 1.2.0
#define LT_MOVE(x) (x)
#else // for new version libtorrent
#define LT_MOVE(x) (std::move(x))
#endif //LIBTORRENT_VERSION_NUM

#define DEBUG_CURRENT_MODULE VS_DM_FILETRANSFER

class VS_FileTransferImpl;

namespace
{
	constexpr char RESUME_DATA_EXT[] = ".resume_data";

	lt::settings_pack init_torrent_settings(int flags, unsigned int filePoolSize, bool enablePeerNotif)
	{
		lt::settings_pack settings;

		const auto alert_mask_val =
#ifdef TORRENT_DISABLE_LOGGING
			lt::alert::status_notification | lt::alert::storage_notification | lt::alert::error_notification
			| (enablePeerNotif ? lt::alert::peer_notification : decltype(lt::alert::peer_notification){})
#else
			lt::alert::all_categories
#endif //TORRENT_DISABLE_LOGGING
			;

		settings.set_int(lt::settings_pack::alert_mask, alert_mask_val);

		settings.set_bool(lt::settings_pack::enable_natpmp, bool(flags & VS_FileTransfer::NAT_PMP));
		settings.set_bool(lt::settings_pack::enable_upnp, bool(flags & VS_FileTransfer::UPNP));
#ifndef TORRENT_DISABLE_DHT
		settings.set_bool(lt::settings_pack::enable_dht, bool(flags & VS_FileTransfer::DHT));
		settings.set_int(lt::settings_pack::dht_announce_interval, 10);
#endif //TORRENT_DISABLE_DHT

		settings.set_bool(lt::settings_pack::enable_lsd, bool(flags & VS_FileTransfer::LSD));

#if (LIBTORRENT_VERSION_NUM < 10200) //< 1.2.0
		settings.set_bool(lt::settings_pack::lock_files, bool(flags & VS_FileTransfer::LockFiles));
#endif //(LIBTORRENT_VERSION_NUM < 10200) //< 1.2.0

		settings.set_bool(lt::settings_pack::prefer_udp_trackers, false);
		settings.set_bool(lt::settings_pack::enable_incoming_utp, false);
		settings.set_bool(lt::settings_pack::enable_outgoing_utp, false);
		settings.set_bool(lt::settings_pack::enable_incoming_tcp, true);
		settings.set_bool(lt::settings_pack::enable_outgoing_tcp, true);

		settings.set_int(lt::settings_pack::min_announce_interval, 1);
		settings.set_bool(lt::settings_pack::send_redundant_have, false);
		settings.set_bool(lt::settings_pack::allow_multiple_connections_per_ip, true);
		settings.set_int(lt::settings_pack::active_downloads, 512);
		settings.set_int(lt::settings_pack::active_seeds, 512);
		settings.set_int(lt::settings_pack::active_limit, 1024);
		settings.set_int(lt::settings_pack::auto_manage_interval, 5);
		settings.set_bool(lt::settings_pack::incoming_starts_queued_torrents, true);
		settings.set_bool(lt::settings_pack::announce_to_all_trackers, true);
		settings.set_int(lt::settings_pack::unchoke_interval, 5);

		settings.set_int(lt::settings_pack::file_pool_size, (filePoolSize > VS_FileTransfer::MAX_FILE_POOL_SIZE ? VS_FileTransfer::MAX_FILE_POOL_SIZE : filePoolSize));

#if (LIBTORRENT_VERSION_NUM < 10200) //< 1.2.0
		settings.set_int(lt::settings_pack::connection_speed, 20); // default is 10
		// Disable support for SSL torrents for now
		settings.set_int(lt::settings_pack::ssl_listen, 0);
		// To prevent ISPs from blocking seeding
		settings.set_bool(lt::settings_pack::lazy_bitfields, true);
		// Disk cache pool is rarely tested in libtorrent and doesn't free buffers
		// Soon to be deprecated there
		// More info: https://github.com/arvidn/libtorrent/issues/2251
		settings.set_bool(lt::settings_pack::use_disk_cache_pool, false);
#endif //(LIBTORRENT_VERSION_NUM < 10200)


		return settings;
	}

	inline VS_FileTransferImpl *get_libtorrent_impl(void *impl) noexcept
	{
		return static_cast<VS_FileTransferImpl *>(impl);
	}

}//anonymous namespace

class VS_FileTransferImpl : public VS_FileTransfer
{
	typedef VS_FileTransfer::Endpoint endpoint_t;
	typedef std::chrono::steady_clock clock_t;
	typedef clock_wrapper<clock_t> clock_wrapper_t;
	typedef boost::asio::basic_waitable_timer<clock_t> timer_t;
	typedef std::mutex mutex_t;
	typedef boost::asio::io_service io_service_t;
public:
	VS_FileTransferImpl(const char *workDir, int flags, unsigned int filePoolSize, std::weak_ptr<Events> eventListener);

	~VS_FileTransferImpl() override { Shutdown(); };

	unsigned short ListenPort() const override;
	std::string PeerId() const override;
	void SendFile(std::string path, std::string to, std::vector<std::string> trackers) override;
	void GetStatistics(std::vector<VS_FileTransfer::Info> &info, bool include_files_stats) override;
	int GetServerProgress(string_view info) override;
	uintmax_t GetServerProgressBytes(const std::string &info) override;
	void StartDownload(std::string magnet, std::string from, std::string destPath) override;
	Control GetControl(std::string hash) override;
	void Shutdown() noexcept override;
	void SetLimits(const unsigned int dwnLim, const unsigned int upLim) override;
	void SetFilePoolSize(unsigned int maxSize) override;

	//Control functions
	void ConfirmDownload(string_view hash, bool result, const std::string &path);
	void RemoveTorrent(string_view hash, bool deleteFiles);
	void Connect(string_view hash, const endpoint_t &endpoint);
	void Pause(string_view hash);
	void Resume(string_view hash);
	void GetStatistics(string_view hash, VS_FileTransfer::Info &info, bool include_files_stats);
	void AddTrackers(string_view hash, const std::vector<std::string> &uris);
	void AddTrackersFromURI(string_view hash, string_view uri);
	io_service_t &get_io_service() noexcept
	{
		return this->m_ios;
	}

protected:
	clock_wrapper_t &clock() const noexcept
	{
		return m_clock;
	}
	void GetStatistics(const lt::torrent_handle &h, VS_FileTransfer::Info &info, bool include_files_stats);
	void GetFilesStatistics(const lt::torrent_handle &h, VS_FileTransfer::Info &info);
	void SetServerProgress(const lt::sha1_hash &h, uint64_t p);
private:
	void WaitForMetadata(timer_t *timer, lt::add_torrent_params p, std::string from, clock_t::time_point timeout, uint64_t size);
	void DispachEvent();
	void ProcessEvent(lt::alert *alert);
	std::pair<uintmax_t, uintmax_t> GetServerProgressBytesImpl(string_view info);

private:
	struct AddTorrentParams : public lt::add_torrent_params
	{
		std::vector<endpoint_t> endpoints;
	};

	struct FilesProgressRecord final
	{
		std::vector<VS_FileTransfer::FileInfo> filesInfo;
		time_t timeStamp = 0;
		int dwnRate = 0;
		int upRate = 0;
		int numPeers = 0;
	};

	struct
	{
		std::string name;
		int piecesTotal = 0;
		int piecesDone = 0;
	} m_indexing;

private:
	boost::asio::io_service m_ios;
	boost::optional<boost::asio::io_service::work> m_worker;

	lt::session m_session;


	std::thread m_thread;
	mutex_t m_statLock;

	std::string m_workingDir;

	vs::map<std::string, FilesProgressRecord, vs::str_less> m_lastFilesProgress;
	std::map<lt::sha1_hash, uint64_t> m_serverProgress;

	std::vector<AddTorrentParams> m_notAddedYet;
	std::vector<AddTorrentParams> m_failedToStart;

	mutable clock_wrapper_t m_clock;

	std::atomic_bool m_isShuttingDown;
};

VS_FileTransferImpl::VS_FileTransferImpl(const char *workDir, int flags, unsigned int filePoolSize, std::weak_ptr<Events> eventListener)
	:
	VS_FileTransfer(std::move(eventListener))
	, m_session(init_torrent_settings(flags, filePoolSize, workDir == nullptr))
	, m_isShuttingDown(true)
{
	m_isShuttingDown.store(false, std::memory_order_release);

	m_worker.emplace(m_ios);

	m_thread = std::thread([this]()
		{
			vs::SetThreadName("LibTorrent");
			vs::FixThreadSystemMessages();
			m_ios.run();
		});

	if (workDir)
	{
		m_session.add_extension(CreateVSLibTorrentPluginServer);
	}
	else
	{
		m_session.add_extension(CreateVSLibTorrentPluginClient);
	}

	m_workingDir = workDir ? workDir : "";

	m_session.set_alert_notify([this]()
		{
			this->m_ios.post([this]()
				{
					this->DispachEvent();
				});
		});
}

void VS_FileTransferImpl::SendFile(std::string path, std::string to, std::vector<std::string> trackers) {

	m_ios.post([this, path = std::move(path), to = std::move(to), trackers = std::move(trackers)]
	{
		lt::file_storage fs;
		lt::add_files(fs, path);

		if (!fs.total_size()) {
			if (auto listener = m_eventsListener.lock())
				listener->onReadyToSend("", to);
			return;
		}

		lt::create_torrent torrent(fs);
		torrent.set_creator("TrueConf");

		for (const auto &tracker : trackers)
		{
			torrent.add_tracker(tracker);
		}

		lt::error_code ec;
		std::vector<char> result;
		lt::add_torrent_params p;

		auto parent_path = lt::vs_parent_path(path);

		m_statLock.lock();
		m_indexing.name = std::move(path);
		m_indexing.piecesDone = 0;
		m_indexing.piecesTotal = torrent.num_pieces();
		m_statLock.unlock();

		lt::set_piece_hashes(torrent, parent_path, [this](lt::piece_index_t apiecesDone) noexcept
			{
				this->m_indexing.piecesDone = lt::underlying_t<decltype(apiecesDone)>{ apiecesDone };
			}, ec);


		m_statLock.lock();
		m_indexing.name.clear();
		m_statLock.unlock();

		if (ec) {
			if (auto listener = m_eventsListener.lock())
				listener->onReadyToSend("", to);
			return;
		}

		lt::bencode(std::back_insert_iterator< std::vector<char> >(result), torrent.generate());

		p.ti = lt::make_shared<lt::torrent_info>(&result[0], result.size(), ec);
		if (p.ti->num_pieces() <= 0) return;

		p.save_path = std::move(parent_path);

		lt::torrent_handle _h = m_session.add_torrent(p, ec);

		if (auto listener = m_eventsListener.lock())
			listener->onReadyToSend(lt::vs_make_magnet_uri(_h), to);
	});
}

void VS_FileTransferImpl::GetStatistics(string_view hash, VS_FileTransfer::Info &info, bool include_files_stats) {
	std::lock_guard<decltype(m_statLock)> lock(m_statLock);

	for (const auto &i : m_failedToStart)
	{
		if (lt::hash_to_string(i.info_hash) != hash) continue;

		info.infoHash = lt::hash_to_string(i.info_hash);
		info.lastError = "Failed to start";
		info.name = i.name;
		return;
	}

	for (const auto &i : m_notAddedYet)
	{
		if (lt::hash_to_string(i.info_hash) != hash) continue;

		info.infoHash = lt::hash_to_string(i.info_hash);
		info.lastError = "Not added yet";
		info.name = i.name;
		if (i.ti) {
			info.totalWanted = i.ti->total_size();
		}
		else {
			info.totalWanted = 0;
		}
		return;
	}

	lt::error_code ec;
	auto infoHash = lt::string_to_hash(hash, ec);

	GetStatistics(m_session.find_torrent(infoHash), info, include_files_stats);
}

void VS_FileTransferImpl::GetStatistics(const lt::torrent_handle &h, VS_FileTransfer::Info &a, bool include_files_stats) {
	a = VS_FileTransfer::Info();

	lt::torrent_status s = h.status();

	auto t = s.torrent_file.lock();
	if (!t || !s.has_metadata)
	{
		dstream3 << "VS_FileTransferImpl::GetStatistics: torrent handle invalid or doesn't have metadata, " << "infoHash = " << lt::hash_to_string(h.info_hash());
		return;
	}

	assert(t);

	a.infoHash = lt::hash_to_string(h.info_hash());
	a.path = s.save_path + lt::FILE_SEPARATOR + s.name;
	a.name = s.name;
	a.totalDownload = s.total_download;
	a.totalUpload = s.total_upload;
	a.totalPayloadDownload = s.total_payload_download;
	a.totalPayloadUpload = s.total_payload_upload;
	a.downloadRate = s.download_payload_rate;
	a.uploadRate = s.upload_payload_rate;
	a.paused = lt::is_paused(s);
	a.autoManaged = lt::is_auto_managed(s);
	a.sequentialDownload = lt::is_sequential_download(s);
	a.isSeeding = s.is_seeding;
	a.isFinished = s.is_finished;
	a.totalWanted = s.total_wanted;
	a.totalWantedDone = s.total_wanted_done;
	a.numPeers = s.num_peers;
	a.completedTime = s.completed_time;

	if (s.errc) // error
	{
		a.lastError = lt::convert_from_native(s.errc.message());
	}
	else
	{
		a.lastError.clear();
	}

	a.isIndexing = false;
	a.piecesTotal = s.pieces.size();
	a.piecesIndexed = s.pieces.size();

	a.unchanged = false;

	if (include_files_stats) {
		GetFilesStatistics(h, a);
	}


	if (!m_workingDir.empty() && a.isFinished && a.lastError.empty()) {
		bool should_recheck = false;

		const auto &files = t->files();
		const lt::file_index_t num_files{ files.num_files() };
		for (lt::file_index_t i{ 0 }; i < num_files; ++i)
		{
			lt::error_code ec;
			VS_SCOPE_EXIT
			{
				if (ec) // if error
				{
					dstream3 << "VS_FileTransferImpl::GetStatistics: error msg = " << ec.message();
				}
			};

			lt::file_status stat{};
			lt::stat_file(s.save_path + lt::FILE_SEPARATOR + files.file_path(i), &stat, ec);

			if (ec || stat.file_size != files.file_size(i) || (files.mtime(i) && static_cast<decltype(stat.mtime)>(files.mtime(i)) != stat.mtime)) {
				should_recheck = true;
				break;
			}
		}

		if (should_recheck) {
			h.flush_cache();
			h.force_recheck();
			a.unchanged = false;
		}
	}
}


void VS_FileTransferImpl::GetStatistics(std::vector<VS_FileTransfer::Info> &info, bool include_files_stats) {
	info.clear();

	m_statLock.lock();

	if (!m_indexing.name.empty()) {
		VS_FileTransfer::Info a;
		a.name = m_indexing.name;
		a.isIndexing = true;
		a.piecesIndexed = m_indexing.piecesDone;
		a.piecesTotal = m_indexing.piecesTotal;
		info.push_back(a);
	}

	m_statLock.unlock();

	for (const auto &i : m_failedToStart)
	{
		VS_FileTransfer::Info a;
		GetStatistics(hash_to_string(i.info_hash), a, include_files_stats);
		info.push_back(a);
	}

	std::vector <lt::torrent_handle > torrents = m_session.get_torrents();

	for (const auto &torrent : torrents)
	{
		VS_FileTransfer::Info a;
		GetStatistics(torrent, a, include_files_stats);
		if (!a.infoHash.empty()) info.push_back(a);
	}
}

void VS_FileTransferImpl::GetFilesStatistics(const lt::torrent_handle &h, VS_FileTransfer::Info &info) {
	const auto now_time_point = clock().now();
	const auto now = std::chrono::duration_cast<std::chrono::seconds>(now_time_point.time_since_epoch()).count();

	auto t = h.torrent_file();
	assert(t);

	FilesProgressRecord new_record;
	new_record.timeStamp = now;

	std::vector<int64_t> fp;
	h.file_progress(fp, lt::torrent_handle::piece_granularity);

	const auto fp_size = fp.size();

	new_record.dwnRate = info.downloadRate;
	new_record.upRate = info.uploadRate;
	new_record.numPeers = info.numPeers;

	auto last_progress = m_lastFilesProgress.find(lt::hash_to_string(h.info_hash()));
	if (last_progress != m_lastFilesProgress.end() &&
		new_record.dwnRate == last_progress->second.dwnRate &&
		new_record.upRate == last_progress->second.upRate &&
		new_record.numPeers == last_progress->second.numPeers)
	{
		info.unchanged = true;
		info.files = last_progress->second.filesInfo;
		return;
	}

	std::vector<int> piece_availability;
	h.piece_availability(piece_availability);

	auto &files = t->files();
	const lt::file_index_t num_files{ files.num_files() };

	for (lt::file_index_t i{ 0 }; i < num_files; ++i)
	{
		const auto file_size = files.file_size(i);
		lt::peer_request pr = t->map_file(i, 0, int(file_size));
		const int num_pieces = t->num_pieces();

		auto numPeers = std::numeric_limits<lt::underlying_t<lt::piece_index_t>>::max();
		if (piece_availability.empty())
		{
			numPeers = 0;
		}
		else
		{
			auto end_p = decltype(pr.piece){ lt::underlying_t<decltype(pr.piece)>{ pr.piece } +num_pieces };
			for (lt::piece_index_t p = pr.piece; p < end_p; ++p)
			{
				if (p < decltype(p)(piece_availability.size()))
				{
					numPeers = std::min(numPeers, piece_availability[lt::underlying_t<decltype(p)>{ p }]);
				}
			}
		}

		new_record.filesInfo.emplace_back();
		auto &fi = new_record.filesInfo.back();
		fi.name = files.file_path(i);
		fi.size = file_size;

		const auto j = lt::underlying_t<decltype(i)>{ i };
		fi.done = fp_size > j ? fp[j] : file_size;

		fi.uploadRate = info.uploadRate / t->num_files();
		fi.downloadRate = 0;
		fi.numPeers = numPeers;

		if (last_progress != m_lastFilesProgress.end())
		{
			const double dt = ::difftime(now, last_progress->second.timeStamp);
			const auto d_rate = int((fi.done - last_progress->second.filesInfo[lt::underlying_t<decltype(i)>{ i }].done) / dt);
			new_record.filesInfo.back().downloadRate = d_rate > 0 ? d_rate : 0;
		}
	}

	m_lastFilesProgress[info.infoHash] = new_record;
	info.files = std::move(new_record.filesInfo);
}

int VS_FileTransferImpl::GetServerProgress(string_view info) {
	assert(m_workingDir.empty());
	auto p = GetServerProgressBytesImpl(info);
	return p.first == p.second ? 100 : int((100.0 * p.first) / p.second);
}

uintmax_t VS_FileTransferImpl::GetServerProgressBytes(const std::string& info)
{
	return GetServerProgressBytesImpl(info).first;
}

std::pair<uintmax_t, uintmax_t> VS_FileTransferImpl::GetServerProgressBytesImpl(string_view hash) {
	assert(m_workingDir.empty());
	lt::error_code ec;
	lt::sha1_hash infoHash = lt::string_to_hash(hash, ec);

	if (!ec) //no error
	{
		std::lock_guard<decltype(m_statLock)> _(m_statLock);
		auto it = m_serverProgress.find(infoHash);
		if (it != m_serverProgress.cend())
		{
			auto h = m_session.find_torrent(infoHash);
			auto tr_file = h.torrent_file();
			if (tr_file)
			{
				return std::make_pair(it->second, tr_file->total_size());
			}
			return std::make_pair(0, 0);
		}
	}
	return std::make_pair(0, 0);
}

void VS_FileTransferImpl::SetServerProgress(const lt::sha1_hash &h, uint64_t p) {
	assert(m_workingDir.empty());
	std::lock_guard<decltype(m_statLock)> _(m_statLock);
	m_serverProgress[h] = p;
}

void VS_FileTransferImpl::StartDownload(std::string magnet, std::string from, std::string destPath)
{
	m_ios.post([this, magnet = std::move(magnet), from = std::move(from), destPath = std::move(destPath)]
	{
		dstream4 << "VS_FileTransferImpl::StartDownload: magnet = " <<
		magnet << "; from = " << from;
	lt::add_torrent_params p;
	std::vector<char> resume_data_buf;

	lt::error_code ec;
	uint64_t xl;
	lt::vs_parse_magnet_uri(magnet, p, xl, ec);
	if (ec)
	{
		dstream3 << "VS_FileTransferImpl::StartDownload: parse_magnet_uri failed, err = " << ec;
		return; // invalid magnet uri
	}

	lt::torrent_handle h = m_session.find_torrent(p.info_hash);
	VS_FileTransfer::Info info;
	if (h.is_valid()) {
		GetStatistics(h, info, false);
	}

	bool path_check = !destPath.empty()
		? lt::exists(destPath + lt::FILE_SEPARATOR + p.name, ec)
		: !info.path.empty() ? lt::exists(info.path, ec) : true;

	bool already_downloaded = // need to add condition to check whether file was changed after downloading (by last write time or by hash)
		h.is_valid() &&
		info.lastError.empty() &&
		path_check;

	if (already_downloaded) {
		// already downloading this torrent
		dstream4 << "VS_FileTransferImpl::StartDownload: torrent is already downloaded;";
		if (auto listener = m_eventsListener.lock())
			listener->onAskUser(lt::hash_to_string(p.info_hash), p.name, from, xl);
	}
	else {

		if (xl > 0)
		{
			if (auto listener = m_eventsListener.lock())
				if (!listener->onAskUser(hash_to_string(p.info_hash), p.name, from, xl)) //if error
					return;
		}

		const bool prev_file_present = lt::exists(info.path, ec);
		RemoveTorrent(info.infoHash, !prev_file_present);

		const int TIMEOUT_MS = 300000;
		clock_t::time_point timeout = clock().now() + std::chrono::milliseconds(TIMEOUT_MS);

		std::string infoHash = lt::hash_to_string(p.info_hash);

		const std::string file_name = m_workingDir + infoHash + RESUME_DATA_EXT;
		const std::string backup_file_name = m_workingDir + infoHash + ".bak";

		std::ifstream f1(file_name, std::ios::binary);
		std::ifstream f2(backup_file_name, std::ios::binary);
		std::streamoff size1 = 0, size2 = 0;

		if (f1) {
			f1.seekg(0, std::ios::end); size1 = f1.tellg(); f1.seekg(0, std::ios::beg);
		}
		if (f2) {
			f2.seekg(0, std::ios::end); size2 = f2.tellg(); f2.seekg(0, std::ios::beg);
		}

		if (size1 || size2) {
			std::ifstream &f = (size1 > size2) ? f1 : f2;
			size_t size = static_cast<size_t>(std::max(size1, size2));

			resume_data_buf.resize(size);
			f.read(&resume_data_buf[0], size);
		}
		if (!resume_data_buf.empty()) {
			p = lt::resume_data(std::move(resume_data_buf), ec); // error code
		}
		// start in upload mode, so files won't be downloaded/allocated
		// ConfirmDownload will put it in normal mode
		p.save_path = m_workingDir + infoHash;

		//set 'upload_mode' for to download the metadata and turn off 'flag_autoManaged'
		//so that the libtorrent does not bring the torrent out of the state 'upload_mode'
		p.flags = (lt::torrent_flags::update_subscribe
			| lt::torrent_flags::apply_ip_filter
			| lt::torrent_flags::upload_mode)
			& ~lt::torrent_flags::auto_managed
			& ~lt::torrent_flags::paused;

		p.storage_mode = lt::storage_mode_allocate;
		lt::torrent_handle h = m_session.add_torrent(p, ec);

		dstream4 << "VS_FileTransferImpl::StartDownload: torrent added. infoHash = " <<
			p.info_hash << "; name = " <<
			p.name << "; download path = " <<
			p.save_path << "; torrent_handele.is_valid() = " <<
			h.is_valid() << " error_code = " << ec.message();

		if (ec) //if error
		{
			return;
		}

		timer_t *timer = new timer_t(m_ios);
		timer->expires_from_now(std::chrono::seconds(1));
		timer->async_wait([this, timer, p = std::move(p), f = std::move(from), timeout, xl](const boost::system::error_code &ec) mutable
		{
			if (!ec) //no error
			{
				this->WaitForMetadata(timer, std::move(p), std::move(f), timeout, xl);
			}
		});
	}
	});
}

void VS_FileTransferImpl::ConfirmDownload(string_view hash, bool result, const std::string &path)
{
	dstream4 << "VS_FileTransferImpl::ConfirmDownload: infoHash = " <<
		hash << "; result = " <<
		result << "; path = " <<
		path;
	//sha1_hash h = string_to_hash( infoHash );
	/*for (unsigned i = 0; i < m_not_added_yet.size(); i++)
	{
		if (m_not_added_yet[ i ].infoHash == h)
		{
			if (result)
			{
				// start downloading
				error_code ec;
				m_not_added_yet[ i ].save_path = boost::filesystem::absolute(path).string();
				torrent_handle h = m_session.find_torrent(m_not_added_yet[i].infoHash);//m_session.add_torrent( m_not_added_yet[ i ] );
				if (ec || !h.is_valid()) m_failed_to_start.push_back( m_not_added_yet[ i ] );
				else {
					for (unsigned j = 0; j < m_not_added_yet[ i ].endpoints.size(); j++)
					{
						Endpoint _e = m_not_added_yet[ i ].endpoints[j];
						tcp::endpoint e;
						e.address( boost::asio::ip::address().from_string(_e.ip) );
						e.port( _e.port );
						h.connect_peer( e );
					}
					h.move_storage(boost::filesystem::absolute(path).string());
					h.set_upload_mode(false);
					h.save_resume_data();
				}
			}

			m_not_added_yet.erase( m_not_added_yet.begin() + i, m_not_added_yet.begin() + i + 1);
		}
	}*/
	lt::error_code ec;
	auto infoHash = lt::string_to_hash(hash, ec);
	if (ec) // error
	{
		return;
	}

	lt::torrent_handle h = m_session.find_torrent(infoHash);
	if (h.is_valid()) {
		if (result) {
			h.move_storage(path);

			lt::unset_flags(h, lt::torrent_flags::upload_mode | lt::torrent_flags::paused);

			h.save_resume_data(lt::torrent_handle::save_info_dict);
		}
		else {
			RemoveTorrent(hash, true);
		}
	}
}

void VS_FileTransferImpl::RemoveTorrent(string_view hash, bool deleteFiles) {
	dstream4 << "VS_FileTransferImpl::RemoveTorrent: infoHash = " <<
		hash << "; delete_files = " <<
		deleteFiles;

	lt::error_code ec;
	lt::sha1_hash infoHash = lt::string_to_hash(hash, ec);
	if (ec) // error
	{
		return;
	}

	for (size_t i = 0; i < m_failedToStart.size(); i++) {
		if (m_failedToStart[i].info_hash == infoHash) {
			m_failedToStart.erase(m_failedToStart.begin() + i, m_failedToStart.begin() + i + 1);
		}
	}

	lt::torrent_handle h = m_session.find_torrent(infoHash);

	std::string resume_data;
	resume_data.reserve(m_workingDir.length() + hash.length() + ::strlen(RESUME_DATA_EXT));
	resume_data.append(m_workingDir);
	resume_data += hash;
	resume_data.append(RESUME_DATA_EXT);

	lt::remove(resume_data, ec);

	if (h.is_valid())
	{
		m_session.remove_torrent(h, deleteFiles ? lt::session::delete_files : lt::remove_flags_t{});
	}

	const auto it_last_pr = m_lastFilesProgress.find(hash);
	if (it_last_pr != m_lastFilesProgress.cend())
	{
		m_lastFilesProgress.erase(it_last_pr);
	}

	std::lock_guard<decltype(m_statLock)> _(m_statLock);
	m_serverProgress.erase(infoHash);
}

VS_FileTransfer::Control VS_FileTransferImpl::GetControl(std::string hash)
{
	return Control(std::move(hash), this);
}

void VS_FileTransferImpl::Shutdown() noexcept
{
	const auto res = m_isShuttingDown.exchange(true, std::memory_order_acq_rel);

	assert(!res);
	(void)res;

	m_worker.reset();
	m_ios.stop();
	assert(m_thread.joinable());
	m_thread.join();
}

void VS_FileTransferImpl::Connect(string_view hash, const endpoint_t &endpoint)
{
	dstream4 << "VS_FileTransferImpl::Connect: infoHash = " <<
		hash << "; endpoint = " <<
		endpoint.ip << ":" << endpoint.port;

	lt::error_code ec;
	const auto infoHash = lt::string_to_hash(hash, ec);

	if (!ec) // no error
	{
		for (auto &i : m_notAddedYet)
		{
			if (i.info_hash == infoHash)
			{
				i.endpoints.push_back(endpoint);
				return;
			}
		}

		lt::torrent_handle h = m_session.find_torrent(infoHash);
		if (!h.is_valid())
		{
			return;
		}

		auto addr = lt::address::from_string(endpoint.ip, ec);

		if (!ec) // no error
		{
			h.connect_peer(lt::tcp::endpoint{ addr, static_cast<decltype(std::declval<lt::tcp::endpoint>().port())>(endpoint.port) });
		}
	}

	if (ec) // error
	{
		dstream3 << "VS_FileTransferImpl::Connect: error msg - " << ec.message();
	}

}

void VS_FileTransferImpl::SetLimits(unsigned int dwnLim, unsigned int upLim)
{
	lt::settings_pack p;
	p.set_int(lt::settings_pack::download_rate_limit, dwnLim);
	p.set_int(lt::settings_pack::upload_rate_limit, upLim);
	m_session.apply_settings(LT_MOVE(p));
}

void VS_FileTransferImpl::SetFilePoolSize(unsigned int maxSize)
{
	lt::settings_pack p;
	p.set_int(lt::settings_pack::file_pool_size, (maxSize > VS_FileTransfer::MAX_FILE_POOL_SIZE ? VS_FileTransfer::MAX_FILE_POOL_SIZE : maxSize));
	m_session.apply_settings(LT_MOVE(p));
}

void VS_FileTransferImpl::Pause(string_view hash)
{
	lt::error_code ec;
	auto infoHash = lt::string_to_hash(hash, ec);
	if (!ec) // no error
	{
		lt::torrent_handle h = m_session.find_torrent(infoHash);
		if (h.is_valid()) h.pause();
	}
}

void VS_FileTransferImpl::Resume(string_view hash)
{
	lt::error_code ec;
	auto infoHash = lt::string_to_hash(hash, ec);
	if (!ec) // no error
	{
		lt::torrent_handle h = m_session.find_torrent(infoHash);
		if (h.is_valid()) h.resume();
	}
}

void VS_FileTransferImpl::WaitForMetadata(timer_t *timer, lt::add_torrent_params p, std::string from, clock_t::time_point timeout, uint64_t size)
{
	if (m_isShuttingDown) {
		delete timer;
		return;
	}

	const std::string hash = lt::hash_to_string(p.info_hash);

	lt::torrent_handle h = m_session.find_torrent(p.info_hash);

	auto stat = h.status(lt::torrent_handle::query_torrent_file);
	auto tr_info = stat.torrent_file.lock();
	const bool valid_metadata = tr_info && stat.has_metadata;
	const bool time_out = clock().now() > timeout;

	if (valid_metadata || time_out)
	{
		std::unique_ptr<timer_t> _{ timer };

		assert(m_workingDir.empty() || m_workingDir.back() == '/' || m_workingDir.back() == '\\');

		bool res = !time_out && valid_metadata;

		if (res)
		{
			assert(valid_metadata);

			if (size == 0)
			{
				if (auto listener = m_eventsListener.lock())
					res = listener->onAskUser(hash, p.name, from, tr_info->total_size());
			}

			if (res)
			{
				if (auto listener = m_eventsListener.lock())
					listener->onMetadataSignal(hash, from);
			}
		}

		ConfirmDownload(hash, res, m_workingDir + hash + lt::FILE_SEPARATOR);

	}
	else
	{
		timer->expires_from_now(std::chrono::seconds(1));
		timer->async_wait([this, timer, p = std::move(p), f = std::move(from), timeout, xl = size](const boost::system::error_code &ec) mutable
		{
			if (!ec) // no error
			{
				this->WaitForMetadata(timer, std::move(p), std::move(f), timeout, xl);
			}
		});
	}
}

void VS_FileTransferImpl::DispachEvent()
{
	if (m_isShuttingDown.load(std::memory_order_acquire)) return;

	std::vector<lt::alert *> alerts;
	m_session.pop_alerts(&alerts);

	for (const auto alert : alerts)
	{
		ProcessEvent(alert);
	}
}

void VS_FileTransferImpl::ProcessEvent(lt::alert *alert)
{
	assert(!!alert);

#ifdef TORRENT_DISABLE_LOGGING
	if(alert->category() & lt::alert::error_notification)
#endif //TORRENT_DISABLE_LOGGING
	{
		dstream4 << "VS_LibTorrent_impl::ProcessEvent: alert: type = " <<
			alert->type() << "; what = " <<
			alert->what() << "; message = " <<
			alert->message();
	}

	const lt::torrent_checked_alert *tc = lt::alert_cast<lt::torrent_checked_alert>(alert);
	const lt::torrent_finished_alert *tf = lt::alert_cast<lt::torrent_finished_alert>(alert);
	const lt::metadata_received_alert *mr = lt::alert_cast<lt::metadata_received_alert>(alert);
	const lt::save_resume_data_alert *sr = lt::alert_cast<lt::save_resume_data_alert>(alert);
	const lt::torrent_error_alert *te = lt::alert_cast<lt::torrent_error_alert>(alert);
	const VS_PeerProgressAlert *pp = lt::alert_cast<VS_PeerProgressAlert>(alert);

	const lt::save_resume_data_failed_alert *srf = lt::alert_cast<lt::save_resume_data_failed_alert>(alert);
	const lt::file_error_alert *fe = lt::alert_cast<lt::file_error_alert>(alert);
	const lt::metadata_failed_alert *mf = lt::alert_cast<lt::metadata_failed_alert>(alert);

	if (tf && !m_workingDir.empty()) {
		m_ios.post([h = tf->handle]()
		{
			h.save_resume_data(lt::torrent_handle::save_info_dict);
		});
	}
	else if (mr) {
		// TODO: should use this instead of WaitForMetadata
	}
	else if (sr && !m_workingDir.empty())
	{
		auto resume_data = lt::get_resume_data(*sr);

		std::string infoHash = lt::hash_to_string(lt::sha1_hash(resume_data["info-hash"].string()));

		std::vector<char> buf;
		lt::bencode(std::back_inserter(buf), resume_data);

		const std::string file_name = m_workingDir + infoHash + RESUME_DATA_EXT;
		const std::string backup_file_name = m_workingDir + infoHash + ".bak";

		lt::error_code ec;
		lt::remove(backup_file_name, ec);
		lt::rename(file_name, backup_file_name, ec);

		std::ofstream f(file_name, std::ios::binary);
		f.write(buf.data(), buf.size());
		if (f) {
			lt::remove(backup_file_name, ec);
		}
	}
	else if (pp && m_workingDir.empty()) {
		uint64_t progress = pp->serverProgress;
		if (pp->isServerPeer) {
			auto p_i = pp->handle.torrent_file();
			if (p_i) {
				m_ios.post([this, infoHash = p_i->info_hash(), progress]()
				{
					this->SetServerProgress(infoHash, progress);
				});
			}
		}
	}
	else if (fe)
	{
		auto p_i = fe->handle.torrent_file();
		if (p_i) {
			m_ios.post([callback = m_eventsListener, hash = hash_to_string(p_i->info_hash())]()
			{
				if (auto listener = callback.lock())
					listener->onError(hash, VS_FileTransfer::FileError);
			});
		}
	}
}

unsigned short VS_FileTransferImpl::ListenPort() const
{
	return m_session.listen_port();
}

std::string VS_FileTransferImpl::PeerId() const
{
	return lt::get_id(m_session);
}

void VS_FileTransferImpl::AddTrackers(string_view hash, const std::vector<std::string> &uris)
{
	dstream4 << "VS_FileTransferImpl::AddTrackers: infoHash = " <<
		hash << "; uris = " <<
		boost::algorithm::join(uris, "\n");

	lt::error_code ec;
	auto infoHash = lt::string_to_hash(hash, ec);
	if (ec) return;

	lt::torrent_handle h = m_session.find_torrent(infoHash);
	if (!h.is_valid()) return;

	for (auto &uri : uris) {
		h.add_tracker(lt::announce_entry(uri));
	}
}

void VS_FileTransferImpl::AddTrackersFromURI(string_view hash, string_view uri)
{
	lt::add_torrent_params p;
	lt::error_code ec;
	lt::vs_parse_magnet_uri(uri, p, vs::ignore<uint64_t>{}, ec);
	if (ec || p.trackers.empty()) return;

	AddTrackers(hash, p.trackers);
}

std::unique_ptr<VS_FileTransfer> VS_FileTransfer::MakeFileTransfer(const char* workdir, unsigned int filePoolSize,
	int flags, std::weak_ptr<Events> eventsListener)
{
	return std::make_unique<VS_FileTransferImpl>(workdir, filePoolSize, flags, std::move(eventsListener));
}

std::string VS_FileTransfer::hash_str_from_uri(string_view uri)
{
	lt::add_torrent_params p;
	lt::error_code ec;
	lt::vs_parse_magnet_uri(uri, p, vs::ignore<uint64_t>{}, ec);
	if (ec) return ""; // invalid magnet uri

	return lt::hash_to_string(p.info_hash);
}

std::string VS_FileTransfer::name_from_uri(string_view uri)
{
	lt::add_torrent_params p;
	lt::error_code ec;
	lt::vs_parse_magnet_uri(uri, p, vs::ignore<uint64_t>{}, ec);
	if (ec) return ""; // invalid magnet

	return p.name;
}


VS_FileTransfer::Info::Info()
	: totalDownload(0)
	, totalUpload(0)
	, totalPayloadDownload(0)
	, totalPayloadUpload(0)
	, totalWanted(0)
	, totalWantedDone(0)
	, completedTime(0)
	, downloadRate(0)
	, uploadRate(0)
	, piecesTotal(0)
	, piecesIndexed(0)
	, numPeers(0)
	, unchanged(true)
	, paused(false)
	, autoManaged(false)
	, sequentialDownload(false)
	, isSeeding(false)
	, isFinished(false)
	, isIndexing(false)
{
}

void VS_FileTransfer::Control::ConfirmDownload(bool result, std::string path)
{
	get_libtorrent_impl(m_impl)->get_io_service().post(
		[impl = get_libtorrent_impl(this->m_impl), result, hash = this->m_hash, p = std::move(path)]()
	{
		impl->ConfirmDownload(hash, result, p);
	});
}

void VS_FileTransfer::Control::Remove(bool deleteFiles)
{
	get_libtorrent_impl(m_impl)->get_io_service().post([impl = get_libtorrent_impl(m_impl), h = m_hash, deleteFiles]()
	{
		impl->RemoveTorrent(h, deleteFiles);
	});
}


void VS_FileTransfer::Control::Connect(Endpoint address)
{
	get_libtorrent_impl(m_impl)->get_io_service().post([impl = get_libtorrent_impl(m_impl), hash = m_hash, addrs = std::move(address)]()
	{
		impl->Connect(hash, addrs);
	});
}

void VS_FileTransfer::Control::Pause()
{
	get_libtorrent_impl(m_impl)->Pause(m_hash);
}

void VS_FileTransfer::Control::Resume()
{
	get_libtorrent_impl(m_impl)->Resume(m_hash);
}

void VS_FileTransfer::Control::GetStatistics(VS_FileTransfer::Info &info, bool includeFilesStats)
{
	get_libtorrent_impl(m_impl)->GetStatistics(m_hash, info, includeFilesStats);
}

void VS_FileTransfer::Control::AddTrackers(const std::vector<std::string> &uris) {
	get_libtorrent_impl(m_impl)->AddTrackers(m_hash, uris);
}

void VS_FileTransfer::Control::AddTrackersFromURI(string_view uri)
{
	get_libtorrent_impl(m_impl)->AddTrackersFromURI(m_hash, uri);
}

#undef DEBUG_CURRENT_MODULE

#undef LT_MOVE
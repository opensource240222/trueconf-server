#include "VS_TransceiverPartsMgr.h"
#include "VS_TransceiverParticipant.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/MakeShared.h"

VS_TransceiverPartsMgr::VS_TransceiverPartsMgr(
	std::string serverEP, std::string serverAddr,
	boost::asio::io_service &ios)
	: m_serverAddr(std::move(serverAddr))
	, m_serverEP(std::move(serverEP))
	, m_isStopped(false)
	, m_stop_event(true)
	, m_strand(ios)
{
}

std::shared_ptr<VS_TransceiverParticipant> VS_TransceiverPartsMgr::GetPart(const char *conf_name, const char *part_name) {
	vs::event done(true);
	std::shared_ptr<VS_TransceiverParticipant> res;
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = GetPartImpl(conf_name, part_name);
	});
	done.wait();
	return res;
}

std::shared_ptr<VS_TransceiverParticipant> VS_TransceiverPartsMgr::GetPartImpl(const char *conf_name, const char *part_name)
{
	/**
		search in map
		if
			found => return
		else
			create
	**/
	if (!conf_name || !part_name) return nullptr;
	std::shared_ptr<VS_TransceiverParticipant> res;
	if(m_isStopped)
		return res;
	std::pair<std::string,std::string> key(conf_name,part_name);

	auto part_i = m_parts.find(key);
	if(m_parts.end() == part_i)
	{
		res = vs::MakeShared<VS_TransceiverParticipant>(
			m_serverAddr,m_serverEP,
			conf_name,part_name,
			m_strand.get_io_service());
		m_parts.emplace(key, res);
		std::weak_ptr<VS_TransceiverParticipant> arg(res);
		res->ConnectToConnectionDie([this, arg]() { OnPartDie(arg); });
	}
	else
		res = part_i->second;
	return res;
}

void VS_TransceiverPartsMgr::FreePart(const char *conf_name, const char *part_name) {
	if (!conf_name || !part_name) return;

	m_strand.dispatch([this, conf_name = std::string(conf_name), part_name = std::string(part_name)]() {
		FreePartImpl(conf_name, part_name);
	});
}

void VS_TransceiverPartsMgr::FreePartImpl(const std::string &conf_name, const std::string &part_name)
{
	std::shared_ptr<VS_TransceiverParticipant> part;
	std::pair<std::string,std::string> key(conf_name, part_name);
	auto part_i = m_parts.find(key);
	if(m_parts.end() != part_i)
	{
		part = part_i->second;
		m_parts.erase(part_i);
	}
	else
		return;
	m_pending_close_lst.insert(part);
	part->Stop();
}
void VS_TransceiverPartsMgr::OnPartDie(const std::weak_ptr<VS_TransceiverParticipant> part)
{
	std::shared_ptr<VS_TransceiverParticipant> p = part.lock();
	if(!p)
		return;

	m_strand.post([p, this]() {
		std::pair<std::string, std::string> key(p->GetConfName(), p->GetPartName());
		p->Stop();
		p->WaitForStop();
		m_pending_close_lst.erase(p);
		m_parts.erase(key);
		bool isNotify = m_pending_close_lst.size() == 0 && m_parts.size() == 0;
		if (isNotify)
			m_stop_event.set();
	});
}

void VS_TransceiverPartsMgr::Stop() {
	m_strand.dispatch([this]() {
		StopImpl();
	});
}

void VS_TransceiverPartsMgr::StopImpl()
{
	/**
		Stop all parts
	**/
	bool isNotify(false);
	m_isStopped = true;

	while(m_parts.size() > 0)
		FreePart(m_parts.begin()->first.first.c_str(),m_parts.begin()->first.second.c_str());
	isNotify = m_pending_close_lst.size() == 0 && m_parts.size() == 0;
	if (isNotify)
		m_stop_event.set();
}

void VS_TransceiverPartsMgr::WaitForStop()
{
	m_stop_event.wait();
}
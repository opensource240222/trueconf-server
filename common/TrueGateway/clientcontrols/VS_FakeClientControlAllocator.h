#pragma once
#include "VS_ClientControlAllocatorInterface.h"

#include <set>
#include <mutex>
#include <boost/shared_ptr.hpp>

namespace ts { struct IPool; }
class VS_TranscoderLogin;

class VS_FakeClientControlAllocator :
	public VS_ClientControlAllocatorInterface
{
public:
	VS_FakeClientControlAllocator(const std::weak_ptr<ts::IPool> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin);
	~VS_FakeClientControlAllocator(void);

	virtual boost::shared_ptr<VS_ClientControlInterface> GetTranscoder() override;
	virtual void ReleaseTranscoder(boost::shared_ptr<VS_ClientControlInterface> t);
	virtual void SetMaxTranscoders(const int max_transcoders);

private:
	std::mutex m_data_lock;
	unsigned m_maxClients;
	std::set< boost::shared_ptr<VS_ClientControlInterface> > m_clients;
	const std::weak_ptr<ts::IPool> m_TransPool;
	const std::weak_ptr<VS_TranscoderLogin> m_transLogin;
};

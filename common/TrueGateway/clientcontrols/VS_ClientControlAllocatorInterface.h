#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>

class VS_ClientControlInterface;
namespace ts { struct IPool; }
class VS_TranscoderLogin;

class VS_ClientControlAllocatorInterface
{
public:
	virtual ~VS_ClientControlAllocatorInterface() {}
	virtual boost::shared_ptr<VS_ClientControlInterface> GetTranscoder() = 0;
	virtual void ReleaseTranscoder(boost::shared_ptr<VS_ClientControlInterface> t) = 0;
	virtual void SetMaxTranscoders(const int max_transcoders) = 0;

	static void Init(const std::weak_ptr<ts::IPool>& pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin);
	static VS_ClientControlAllocatorInterface* Instance()
	{
		return s_instance.get();
	}

private:
	static std::unique_ptr<VS_ClientControlAllocatorInterface> s_instance;
};

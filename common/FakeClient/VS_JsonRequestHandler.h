#pragma once

#include <mutex>

#include <functional>
#include <boost/shared_ptr.hpp>

#include "std/cpplib/json/reader.h"
#include "std-generic/cpplib/string_view.h"

class VS_JsonRequestHandler {
public:
	static std::shared_ptr<VS_JsonRequestHandler> GetInstance(void);
	static void SetInstance(std::shared_ptr<VS_JsonRequestHandler> &&handler);

	using JsonCallback = std::function<void(json::Object&&)>;

	virtual void ProcessJsonRequest(json::Object& obj) = 0;
	virtual void RegisterWebPeerCallback(string_view peer_id, const JsonCallback& cb) = 0;
	virtual void DeleteWebPeerCallback(string_view peer_id) = 0;

private:
	static std::mutex m_lock;
	static std::shared_ptr<VS_JsonRequestHandler> m_instance;
};

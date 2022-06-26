#include "VS_JsonRequestHandler.h"

std::mutex VS_JsonRequestHandler::m_lock;
std::shared_ptr<VS_JsonRequestHandler> VS_JsonRequestHandler::m_instance;

std::shared_ptr<VS_JsonRequestHandler> VS_JsonRequestHandler::GetInstance(void)
{
	std::lock_guard<std::mutex> l(m_lock);
	return m_instance;
}

void VS_JsonRequestHandler::SetInstance(std::shared_ptr<VS_JsonRequestHandler> &&handler)
{
	std::lock_guard<std::mutex> l(m_lock);

	m_instance = std::move(handler);
}
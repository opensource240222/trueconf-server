#include "VS_H224Handler.h"
#include "VS_H224H281Handler.h"

std::shared_ptr<VS_H224Handler> VS_H224Handler::CreateHandler(unsigned char clientId)
{
	if (clientId == VS_H281_CLIENT_ID) return std::make_shared<VS_H224H281Handler>();
	// can add here T140, H284...

	return nullptr;
}

VS_H224Handler::VS_H224Handler(const std::string & name)
	: m_h224Display(name)
{

}

VS_H224Handler::~VS_H224Handler()
{

}

void VS_H224Handler::AttachH224Handler(const std::shared_ptr<OpalH224Handler> &h224Handler)
{
	if (m_h224Handler.expired()) {
		m_h224Handler = h224Handler;
	}
}

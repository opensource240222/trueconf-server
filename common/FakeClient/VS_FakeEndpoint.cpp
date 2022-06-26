#include "FakeClient/VS_FakeEndpoint.h"

std::unique_ptr<VS_FakeEndpointFactory> VS_FakeEndpointFactory::s_instance;

void VS_FakeEndpointFactory::DeInit()
{
	if (!s_instance)
		return;
	s_instance->Stop();
	s_instance = nullptr;
}

#pragma once

#include "IEndpoint.h"

#include <memory>

namespace transport {

class Router;

// Contains common logic for real (remote) and fake (local) endpoints.
class EndpointBase : public IEndpoint
{
public:
	string_view GetId() override;
	string_view GetUserId() override;
	void SetUserId(string_view user_id) override;
	void Authorize(string_view id = {}) override;
	void Unauthorize() override;
	bool IsAuthorized() override;
	uint8_t GetHops() override;

protected:
	EndpointBase(std::weak_ptr<Router> router, string_view endpoint_id);
	~EndpointBase();

	void PreprocessMessage(Message& message);

	std::weak_ptr<Router> m_router;
	std::string m_id;
	std::string m_user_id;
	uint8_t m_hops;
	bool m_authorized;
};

}

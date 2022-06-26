#include <boost/asio.hpp>

class MonitorClient
{
public:
	MonitorClient(boost::asio::io_service& io_serv);
	void Start();
	boost::asio::io_service& m_io_serv;
};

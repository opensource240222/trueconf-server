#if defined(_WIN32) // Not ported yet

#include "common.h"

namespace tls_test_common {

	static const size_t EBUFSZ = 1000;

	bool Common::SendAll(const void *buf, const unsigned long size)
	{
		int sent = 0;

		VS_ConnectionTCP *conn = this->conn.get();

		while (sent < (int)size)
		{
			int res = conn->Send(((uint8_t *)buf) + sent, (size - sent));

			if (res <= 0)
				return false;

			sent += res;
		}

		return true;
	}

	bool Common::ReceiveAll(void *buf, const unsigned long size)
	{
		int received = 0;
		VS_ConnectionTCP *conn = this->conn.get();

		while ((received < (int)size))
		{
			int res = conn->Receive(((uint8_t *)buf) + received, (size - received));

			if (res < 0)
				return false;

			received += res;
		}

		return true;
	}

	bool Common::GetStatus() const
	{
		return status;
	}

	void Common::SetStatus(bool status)
	{
		this->status = status;
	}

}

#endif

#include "VS_Connection.h"

#include <boost/shared_ptr.hpp>
#include "../../std/cpplib/VS_WorkThread.h"

bool VS_Connection::SetIOThread(const boost::shared_ptr<VS_WorkThread> &io_thread)
{
	return io_thread->SetHandledConnection( this );
}

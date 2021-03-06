// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "UserLoginV3.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

class UserLoginV3Handler : virtual public UserLoginV3If {
 public:
  UserLoginV3Handler() {
    // Your initialization goes here
  }

  void userLoginV3(UserLoginV3Response& _return, const UserLoginV3Request& request) {
    // Your implementation goes here
    printf("userLoginV3\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<UserLoginV3Handler> handler(new UserLoginV3Handler());
  shared_ptr<TProcessor> processor(new UserLoginV3Processor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}


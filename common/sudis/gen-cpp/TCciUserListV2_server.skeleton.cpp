// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "TCciUserListV2.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::sudis;

class TCciUserListV2Handler : virtual public TCciUserListV2If {
 public:
  TCciUserListV2Handler() {
    // Your initialization goes here
  }

  void cciUserListV2(TCciUserListV2Response& _return, const TCciUserListV2Request& request) {
    // Your implementation goes here
    printf("cciUserListV2\n");
  }

  void cciUserListV2Echo1(TCciUserListV2Request& _return, const TCciUserListV2Request& request) {
    // Your implementation goes here
    printf("cciUserListV2Echo1\n");
  }

  void cciUserListV2Echo2(TCciUserListV2Response& _return, const TCciUserListV2Response& response) {
    // Your implementation goes here
    printf("cciUserListV2Echo2\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<TCciUserListV2Handler> handler(new TCciUserListV2Handler());
  shared_ptr<TProcessor> processor(new TCciUserListV2Processor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

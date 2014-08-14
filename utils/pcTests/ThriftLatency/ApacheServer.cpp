#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "LatencyTest.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

class LatencyTestHandler : virtual public LatencyTestIf {
 public:
  LatencyTestHandler() {
    // Your initialization goes here
  }

  void latency(std::string& _return, const std::string& param) {
    // Your implementation goes here
//	printf("latency\n");
	_return = param;
  }

};

#include "ApacheServer.h"

void ApacheServer::serve()
{

	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<LatencyTestHandler> handler(new LatencyTestHandler());
	boost::shared_ptr<TProcessor> processor(new LatencyTestProcessor(handler));
	boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(9092));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

	TSimpleServer server(processor,
			serverTransport,
			transportFactory,
			protocolFactory);
	printf("Starting the server...\n");
	server.serve();
	printf("done.\n");
}



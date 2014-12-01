#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <chrono>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "LatencyTest.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

#include "ApacheClient.h"


double ApacheClientTest::run(string ip, int samples, int bytes)
{
	typedef std::chrono::high_resolution_clock Clock;
	typedef std::chrono::microseconds microseconds;

	Clock::time_point m_t1;
	Clock::time_point m_t2;

	m_t1 = Clock::now();
	for(int i=0;i<1000;i++)
		m_t2 = Clock::now();
	double m_overhead = std::chrono::duration_cast<microseconds>(m_t2 - m_t1).count()/1001;

	//START APACHE CLIENT:
	boost::shared_ptr<TTransport> socket(new TSocket(ip, 9092));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	LatencyTestClient client(protocol);
	try {
		transport->open();
	}
	catch (TException &tx) {
		printf("ERROR: %s\n", tx.what());
		return -1;
	}

	string payload = "";
	for(int i = 0; i < bytes; ++i) {
		payload += "A";
	}

	string _return;
	m_t1 = Clock::now();
	int isam = 0;
	for(isam = 0;isam<samples;++isam)
	{
		try {
			client.latency(_return, payload);
		} catch (exception &io) {
			printf("Exception: %s\n", io.what());
			break;
		}
	}
	m_t2 = Clock::now();
	transport->close();

	if(isam == samples)
	{
		return (std::chrono::duration_cast<microseconds>(m_t2 - m_t1).count() - m_overhead) / samples;
	}


	return -1;
}



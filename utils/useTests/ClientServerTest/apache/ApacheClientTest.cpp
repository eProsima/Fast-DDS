#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/Calculator.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace tutorial;
using namespace shared;

#include "ApacheClientTest.h"


double ApacheClientTest::run(int samples)
{
	m_clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	//START APACHE CLIENT:
	boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	CalculatorClient client(protocol);
	try {
		transport->open();
		client.ping();
	}
	catch (TException &tx) {
		printf("ERROR: %s\n", tx.what());
		return -1;
	}
	Work work;
	work.op = Operation::ADD;
	work.num1 = 10;
	work.num2 = 20;
	m_clock.setTimeNow(&m_t1);
	int isam = 0;
	for(isam = 0;isam<samples;++isam)
	{
		try {
			//cout << isam << "*";
			client.calculate(1, work);
		} catch (InvalidOperation &io) {
			printf("InvalidOperation: %s\n", io.why.c_str());
			break;
		}
	}
	m_clock.setTimeNow(&m_t2);
	transport->close();
	if(isam == samples)
	{
		return (TimeConv::Time_t2MicroSecondsDouble(m_t2)-
				TimeConv::Time_t2MicroSecondsDouble(m_t1)-
				m_overhead)/samples;
	}

	return -1;
}



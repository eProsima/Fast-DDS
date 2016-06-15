// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
	boost::shared_ptr<TTransport> socket(new TSocket("192.168.1.24", 9090));
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



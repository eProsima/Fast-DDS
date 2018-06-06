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

/**
 * @file ZeroMQPublisher.cpp
 *
 */

#include "fastrtps/utils/TimeConversion.h"
#include "ZeroMQPublisher.h"
#include <sstream>
#include <numeric>

#define TIME_LIMIT_US 10000


uint32_t dataspub[] = {12,28,60,124,252,508,1020,2044,4092,8188,16380};
std::vector<uint32_t> data_size_pub (dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );

ZeroMQPublisher::ZeroMQPublisher():
								m_overhead(0),
								mp_context(NULL),
								mp_datapub(NULL),
								mp_commandpub(NULL),
								mp_datasub(NULL),
								mp_commandsub(NULL),
								n_samples(1000),
								n_sub(0)

{
	// TODO Auto-generated constructor stub
	m_clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;
}

ZeroMQPublisher::~ZeroMQPublisher() {
	// TODO Auto-generated destructor stub
}

bool ZeroMQPublisher::init(std::vector<std::string>subIP,int samples)
{
	n_samples = samples;
	n_sub = (int)subIP.size();
	mp_context = new zmq::context_t(1);
	mp_datasub = new zmq::socket_t(*mp_context,ZMQ_SUB);
	mp_commandsub = new zmq::socket_t(*mp_context,ZMQ_SUB);
	for(auto it=n_sub-1;it>=0;--it)
	{
		stringstream ss;
		int port = 17553 + it;
		ss << "tcp://"<<subIP.at(it)<<":"<<port;
		mp_datasub->connect(ss.str().c_str());
		mp_datasub->setsockopt(ZMQ_SUBSCRIBE,0,0);

		stringstream ss2;
		port = 17573 + it;
		ss2 << "tcp://"<<subIP.at(it)<<":"<<port;
		mp_commandsub->connect(ss2.str().c_str());
		mp_commandsub->setsockopt(ZMQ_SUBSCRIBE,0,0);
	}
	eClock::my_sleep(300);

	mp_datapub = new zmq::socket_t(*mp_context,ZMQ_PUB);
	mp_datapub->bind("tcp://*:17551");
	//mp_datapub->bind("ipc://latency.ipc");
	mp_commandpub = new zmq::socket_t(*mp_context,ZMQ_PUB);
	mp_commandpub->bind("tcp://*:17552");
	//mp_commandpub->bind("ipc://command2sub.ipc");
	eClock::my_sleep(500);


	return true;
}

void ZeroMQPublisher::run()
{
	//WAIT FOR THE DISCOVERY PROCESS FO FINISH:


	printf("Printing round-trip times in us, statistics for %d samples\n",n_samples);
	printf("   Bytes,   stdev,    mean,     min,     50%%,     90%%,     99%%,  99.99%%,     max\n");
	printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");
	//int aux;
	for(std::vector<uint32_t>::iterator ndata = data_size_pub.begin();ndata!=data_size_pub.end();++ndata)
	{
		eClock::my_sleep(100);
		if(!this->test(*ndata))
			break;
		eClock::my_sleep(100);
		//		cout << "Enter number to start next text: ";
		//		std::cin >> aux;
	}
}
#if defined(_WIN32)
bool ZeroMQPublisher::test(uint32_t datasize)
{
	m_times.clear();
	zmq::message_t command(1);
	*(char*)(command.data())='S'; //READY
	mp_commandpub->send(command);
	mp_commandsub->recv(&command);
	if(*(char*)command.data()!=2)
		return false;
	//cout << endl;
	//BEGIN THE TEST:
	uint32_t result;
	m_clock.setTimeNow(&m_t1);
	for(uint32_t i = 0;i<(uint32_t)n_samples;++i)
	{
		zmq::message_t latency_out(datasize+4);
		zmq::message_t latency_in;
		memset(latency_out.data(),65,datasize+4);
#if defined(_WIN32)
		sprintf_s((char*)(latency_out.data()),datasize+4, "%d", i);
#else
		sprintf((char*)(latency_out.data()),"%d",i);
#endif
		mp_datapub->send(latency_out);
		mp_datasub->recv(&latency_in);
		//		std::istringstream iss(static_cast<char*>(latency_in.data()));
		//		cout << "RECEIVED DATA: "<< iss.str()<< endl;

		sscanf_s((char*)latency_in.data(),"%d",&result);
		//	cout << "recevied result: "<< result << " and i is: "<<i << endl;
		//  cout << "SENT/REC: "<< *(uint32_t*)latency_out.data() <<" / "<<*(uint32_t*)latency_in.data()<<endl;
		if(result != i)
		{
			cout << "RECEIVED BAD MESSAGE, STOPPING TEST"<<endl;
			*(char*)(command.data()) = 10;
			mp_commandpub->send(command);
			return false;
		}


	}
	m_clock.setTimeNow(&m_t2);
	m_times.push_back(TimeConv::Time_t2MicroSecondsDouble(m_t2)
	-TimeConv::Time_t2MicroSecondsDouble(m_t1)-m_overhead);
	analyzeTimes(datasize);
	printStat(m_stats.back());

	return true;
}


void ZeroMQPublisher::analyzeTimes(uint32_t datasize)
{
	TimeStats TS;
	TS.nbytes = datasize+4;
	TS.mean = (double)( *m_times.begin()/(n_samples+1));
	m_stats.push_back(TS);
}
void ZeroMQPublisher::printStat(TimeStats& TS)
{
	//cout << "MEAN PRINTING: " << TS.mean << endl;
	printf("%8llu,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
			TS.nbytes,TS.stdev,TS.mean,
			TS.tmin,
			TS.p50,TS.p90,TS.p99,TS.p9999,
			TS.tmax);
}
#else
bool ZeroMQPublisher::test(uint32_t datasize)
{
	m_times.clear();
	zmq::message_t command(1);
	*(char*)(command.data())='S'; //READY
	mp_commandpub->send(command);
	mp_commandsub->recv(&command);
	if(*(char*)command.data()!=2)
		return false;
	//cout << endl;
	//BEGIN THE TEST:
	uint32_t result;
	for(uint32_t i = 0;i<(uint32_t)n_samples;++i)
	{
		zmq::message_t latency_out(datasize+4);
		zmq::message_t latency_in;
		memset(latency_out.data(),65,datasize+4);
		sprintf((char*)(latency_out.data()),"%d",i);
		m_clock.setTimeNow(&m_t1);
		mp_datapub->send(latency_out);
		mp_datasub->recv(&latency_in);
		//		std::istringstream iss(static_cast<char*>(latency_in.data()));
		//		cout << "RECEIVED DATA: "<< iss.str()<< endl;
		m_clock.setTimeNow(&m_t2);
		sscanf((char*)latency_in.data(),"%d",&result);
		//	cout << "recevied result: "<< result << " and i is: "<<i << endl;
		//cout << "SENT/REC: "<< *(uint32_t*)latency_out.data() <<" / "<<*(uint32_t*)latency_in.data()<<endl;
		if(result != i)
		{
			cout << "RECEIVED BAD MESSAGE, STOPPING TEST"<<endl;
			*(char*)(command.data()) = 10;
			mp_commandpub->send(command);
			return false;
		}
		double time = TimeConv::Time_t2MicroSecondsDouble(m_t2)
					-TimeConv::Time_t2MicroSecondsDouble(m_t1)-m_overhead*2;
		if(time <= TIME_LIMIT_US)
			m_times.push_back(time);

	}
	analyzeTimes(datasize);
	printStat(m_stats.back());

	return true;
}
void ZeroMQPublisher::analyzeTimes(uint32_t datasize)
{
	TimeStats TS;
	TS.nbytes = datasize+4;
	TS.tmin = *std::min_element(m_times.begin(),m_times.end());
	TS.tmax = *std::max_element(m_times.begin(),m_times.end());
	TS.mean = std::accumulate(m_times.begin(),m_times.end(),0)/m_times.size();
	double auxstdev=0;
	for(std::vector<double>::iterator tit=m_times.begin();tit!=m_times.end();++tit)
	{
		//cout << *tit<< "/"<< TS.mean<< "///";
		auxstdev += pow(((*tit)-TS.mean),2);
	}
	auxstdev = sqrt(auxstdev/m_times.size());
	TS.stdev = (uint64_t)round(auxstdev);
	std::sort(m_times.begin(),m_times.end());
	double x= 0;
	double elem,dec;
	x = m_times.size()*0.5;
	dec = modf(x,&elem);
	if(dec == 0.0)
		TS.p50 = (uint64_t)((m_times.at(elem)+m_times.at(elem+1))/2);
	else
		TS.p50 = m_times.at(elem+1);
	x = m_times.size()*0.9;
	dec = modf(x,&elem);
	if(dec == 0.0)
		TS.p90 = (m_times.at(elem-1)+m_times.at(elem))/2;
	else
		TS.p90 = m_times.at(elem);
	x = m_times.size()*0.99;
	dec = modf(x,&elem);
	if(dec == 0.0)
		TS.p99 = (m_times.at(elem-1)+m_times.at(elem))/2;
	else
		TS.p99 = m_times.at(elem);
	x = m_times.size()*0.9999;
	dec = modf(x,&elem);
	if(dec == 0.0)
		TS.p9999 = (m_times.at(elem-1)+m_times.at(elem))/2;
	else
		TS.p9999 = m_times.at(elem);


	//printStat(TS);
	m_stats.push_back(TS);
}


void ZeroMQPublisher::printStat(TimeStats& TS)
{
	//cout << "MEAN PRINTING: " << TS.mean << endl;
	printf("%8lu,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
			TS.nbytes,TS.stdev,TS.mean,
			TS.tmin,
			TS.p50,TS.p90,TS.p99,TS.p9999,
			TS.tmax);
}
#endif

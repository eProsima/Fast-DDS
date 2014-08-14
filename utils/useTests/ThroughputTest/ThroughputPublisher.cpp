/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputPublisher.cxx
 *
 */



#include "ThroughputPublisher.h"
#include "eprosimartps/utils/TimeConversion.h"

//uint32_t dataspub[] = {8,24,56,120,248,504,1016,2040,4088,8184};
uint32_t dataspub[] = {504,1016,2040,4088,8184};
std::vector<uint32_t> data_size_pub (dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );

uint32_t demandspub[] = {1000,1250,1500,2000,3000,4000,5000};
vector<uint32_t> demand_pub (demandspub, demandspub + sizeof(demandspub) / sizeof(uint32_t) );


ThroughputPublisher::DataPubListener::DataPubListener(ThroughputPublisher& up):m_up(up){};
ThroughputPublisher::DataPubListener::~DataPubListener(){};

void ThroughputPublisher::DataPubListener::onPublicationMatched(MatchingInfo info)
{
	cout << RTPS_RED << "DATA    Pub Matched"<<RTPS_DEF<<endl;
	m_up.sema.post();
}

ThroughputPublisher::CommandSubListener::CommandSubListener(ThroughputPublisher& up):m_up(up){};
ThroughputPublisher::CommandSubListener::~CommandSubListener(){};
void ThroughputPublisher::CommandSubListener::onSubscriptionMatched(MatchingInfo info)
{
	cout << RTPS_RED << "COMMAND Sub Matched"<<RTPS_DEF<<endl;
	m_up.sema.post();
}

ThroughputPublisher::CommandPubListener::CommandPubListener(ThroughputPublisher& up):m_up(up){};
ThroughputPublisher::CommandPubListener::~CommandPubListener(){};
void ThroughputPublisher::CommandPubListener::onPublicationMatched(MatchingInfo info)
{
	cout << RTPS_RED << "COMMAND Pub Matched"<<RTPS_DEF<<endl;
	m_up.sema.post();
}


ThroughputPublisher::~ThroughputPublisher(){DomainParticipant::stopAll();}

ThroughputPublisher::ThroughputPublisher():
										sema(0),
										m_DataPubListener(*this),m_CommandSubListener(*this),m_CommandPubListener(*this),
										ready(true)
{
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.sendSocketBufferSize = 65536;
	PParam.listenSocketBufferSize = 2*65536;
	PParam.name = "participant1";
	mp_par = DomainParticipant::createParticipant(PParam);
	if(mp_par == NULL)
	{
		cout << "ERROR"<<endl;
		ready = false;
		return;
	}
	m_Clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_Clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;
	//PUBLISHER
	PublisherAttributes Wparam;
	//Wparam.historyMaxSize = 10000;
	Wparam.topic.topicDataType = "LatencyType";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "LatencyUp";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1;
	Wparam.topic.resourceLimitsQos.max_samples = 10000;
	Wparam.topic.resourceLimitsQos.allocated_samples = 10000;
	Wparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_datapub = DomainParticipant::createPublisher(mp_par,Wparam,(PublisherListener*)&this->m_DataPubListener);
	//COMMAND
	SubscriberAttributes Rparam;
	Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	Rparam.topic.resourceLimitsQos.max_samples = 20;
	Rparam.topic.resourceLimitsQos.allocated_samples = 20;
	Rparam.topic.topicDataType = "ThroughputCommand";
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicName = "ThroughputCommandS2P";
	mp_commandsub = DomainParticipant::createSubscriber(mp_par,Rparam,(SubscriberListener*)&this->m_CommandSubListener);

	Wparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	Wparam.topic.resourceLimitsQos.max_samples = 50;
	Wparam.topic.resourceLimitsQos.allocated_samples = 50;
	Wparam.topic.topicDataType = "ThroughputCommand";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "ThroughputCommandP2S";
	mp_commandpub = DomainParticipant::createPublisher(mp_par,Wparam,(PublisherListener*)&this->m_CommandPubListener);

	if(mp_datapub == NULL || mp_commandsub == NULL || mp_commandpub == NULL)
		ready = false;

	mp_par->stopParticipantAnnouncement();
	eClock::my_sleep(5000);
}

void ThroughputPublisher::run(uint32_t test_time)
{
	if(!ready)
		return;
	cout << "Waiting for discovery"<<endl;
	sema.wait();
	sema.wait();
	sema.wait();
	cout << "Discovery complete"<<endl;

	ThroughputCommandType command;
	SampleInfo_t info;
	printLabelsPublisher();
	for(std::vector<uint32_t>::iterator sit=data_size_pub.begin();sit!=data_size_pub.end();++sit)
	{
		for(std::vector<uint32_t>::iterator dit=demand_pub.begin();dit!=demand_pub.end();++dit)
		{
			eClock::my_sleep(100);
			//cout << "Starting test with demand: " << *dit << endl;
			command.m_command = READY_TO_START;
			command.m_size = *sit;
			command.m_demand = *dit;
			mp_commandpub->write((void*)&command);
			command.m_command = DEFAULT;
			mp_commandsub->waitForUnreadMessage();
			mp_commandsub->takeNextData((void*)&command,&info);
			//cout << "Received command of type: "<< command << endl;
			if(command.m_command == BEGIN)
			{
				test(test_time,*dit,*sit);
			}
		}
	}
	command.m_command = ALL_STOPS;
	mp_commandpub->write((void*)&command);
}

void ThroughputPublisher::test(uint32_t test_time,uint32_t demand,uint32_t size)
{
	LatencyType latency(size);
	m_Clock.setTimeNow(&m_t2);
	uint64_t timewait_us=0;
	uint32_t samples=0;
	size_t aux;
	ThroughputCommandType command;
	command.m_command = TEST_STARTS;
	mp_commandpub->write((void*)&command);
	m_Clock.setTimeNow(&m_t1);
	while(TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)<test_time*1000000)
	{
		for(uint32_t sample=0;sample<demand;sample++)
		{
			latency.seqnum++;
			mp_datapub->write((void*)&latency);
			//cout << sample << "*"<<std::flush;
		}
		m_Clock.setTimeNow(&m_t2);
		samples+=demand;
		//cout << "samples sent: "<<samples<< endl;
		eClock::my_sleep(10);
		timewait_us+=m_overhead+10;
		//cout << "Removing all..."<<endl;
		mp_datapub->removeAllChange(&aux);
		//cout << (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))<<endl;
	}
	command.m_command = TEST_ENDS;
	mp_commandpub->write((void*)&command);
	mp_commandpub->removeAllChange(&aux);
	TroughputTimeStats TS;
	TS.nsamples = samples;
	TS.totaltime_us = TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1)-timewait_us;
	TS.samplesize = size+4+4;
	TS.demand = demand;
	//cout << TS.demand << endl;
	TS.compute();
	//cout << TS.demand << endl;
	printTimeStatsPublisher(TS);
	//cout << TS.demand << endl;
	m_timeStats.push_back(TS);
}



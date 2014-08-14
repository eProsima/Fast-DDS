/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputSubscriber.cxx
 *
 */

#include "ThroughputSubscriber.h"
#include "eprosimartps/utils/TimeConversion.h"
#include <vector>

uint32_t datassub[] = {8,24,56,120,248,504,1016,2040,4088,8184};
std::vector<uint32_t> data_size_sub (datassub, datassub + sizeof(datassub) / sizeof(uint32_t) );

uint32_t demandssub[] = {2000,4000,6000,8000,10000};
std::vector<uint32_t> demand_sub (demandssub, demandssub + sizeof(demandssub) / sizeof(uint32_t) );


ThroughputSubscriber::DataSubListener::DataSubListener(ThroughputSubscriber& up):
	m_up(up),lastseqnum(0),saved_lastseqnum(0),lostsamples(0),saved_lostsamples(0),first(true),latencyin(NULL)
{

};
ThroughputSubscriber::DataSubListener::~DataSubListener()
{

};

void ThroughputSubscriber::DataSubListener::reset(){
	lastseqnum = 0;
	first = true;
	lostsamples=0;
}

void ThroughputSubscriber::DataSubListener::onSubscriptionMatched(MatchingInfo info)
{
	cout << RTPS_RED << "DATA    Sub Matched"<<RTPS_DEF<<endl;
	m_up.sema.post();
}
void ThroughputSubscriber::DataSubListener::onNewDataMessage()
{
	//cout << "NEW DATA MSG: "<< latencyin->seqnum << endl;
	m_up.mp_datasub->takeNextData((void*)latencyin,&info);
	//myfile << latencyin.seqnum << ",";
	if((lastseqnum+1)<latencyin->seqnum)
	{
		lostsamples+=latencyin->seqnum-lastseqnum-1;
	//	myfile << "***** lostsamples: "<< lastseqnum << "|"<< lostsamples<< "*****";
	}
	lastseqnum = latencyin->seqnum;
}

void ThroughputSubscriber::DataSubListener::saveNumbers()
{
	saved_lastseqnum = lastseqnum;
	saved_lostsamples = lostsamples;
}



ThroughputSubscriber::CommandSubListener::CommandSubListener(ThroughputSubscriber& up):m_up(up){};
ThroughputSubscriber::CommandSubListener::~CommandSubListener(){};
void ThroughputSubscriber::CommandSubListener::onSubscriptionMatched(MatchingInfo info)
{
	cout << RTPS_RED << "COMMAND Sub Matched"<<RTPS_DEF<<endl;
	m_up.sema.post();
}
void ThroughputSubscriber::CommandSubListener::onNewDataMessage()
{
	//cout << "Command Received: ";
	if(m_up.mp_commandsub->takeNextData((void*)&m_commandin,&info))
	{
	//	cout << m_commandin <<endl;
		switch(m_commandin.m_command)
		{
		default:
		case (DEFAULT): break;
		case (READY_TO_START): m_up.sema.post();break;
		case (BEGIN): break;
		case (TEST_STARTS): m_up.m_Clock.setTimeNow(&m_up.m_t1);break;
		case (TEST_ENDS): m_up.m_Clock.setTimeNow(&m_up.m_t2);m_up.m_DataSubListener.saveNumbers();m_up.sema.post();break;
		case (ALL_STOPS): m_up.sema.post();break;
		}
	}
	else
	{
		cout << "Error reading command"<<endl;
	}
}

ThroughputSubscriber::CommandPubListener::CommandPubListener(ThroughputSubscriber& up):m_up(up){};
ThroughputSubscriber::CommandPubListener::~CommandPubListener(){};
void ThroughputSubscriber::CommandPubListener::onPublicationMatched(MatchingInfo info)
{
	cout << RTPS_RED << "COMMAND Pub Matched"<<RTPS_DEF<<endl;
	m_up.sema.post();
}



ThroughputSubscriber::~ThroughputSubscriber(){DomainParticipant::stopAll();}

ThroughputSubscriber::ThroughputSubscriber():
		sema(0),
		m_DataSubListener(*this),m_CommandSubListener(*this),m_CommandPubListener(*this),
		ready(true),m_datasize(0),m_demand(0)
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
	PParam.name = "participant2";
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
	SubscriberAttributes Sparam;
	Sparam.topic.topicDataType = "LatencyType";
	Sparam.topic.topicKind = NO_KEY;
	Sparam.topic.topicName = "LatencyUp";
	Sparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Sparam.topic.historyQos.depth = 1;
	Sparam.topic.resourceLimitsQos.max_samples = 10000;
	Sparam.topic.resourceLimitsQos.allocated_samples = 10000;
	Sparam.unicastLocatorList.push_back(Locator_t(10110));
	mp_datasub = DomainParticipant::createSubscriber(mp_par,Sparam,(SubscriberListener*)&this->m_DataSubListener);
	//COMMAND
	SubscriberAttributes Rparam;
	Rparam.topic.topicDataType = "ThroughputCommand";
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicName = "ThroughputCommandP2S";
	Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	Rparam.topic.resourceLimitsQos.max_samples = 20;
	Rparam.topic.resourceLimitsQos.allocated_samples = 20;
	Rparam.unicastLocatorList.push_back(Locator_t(10111));
	mp_commandsub = DomainParticipant::createSubscriber(mp_par,Rparam,(SubscriberListener*)&this->m_CommandSubListener);
	PublisherAttributes Wparam;
	//Wparam.historyMaxSize = 20;
	Wparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
		Wparam.topic.resourceLimitsQos.max_samples = 50;
	Wparam.topic.topicDataType = "ThroughputCommand";
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicName = "ThroughputCommandS2P";
	mp_commandpub = DomainParticipant::createPublisher(mp_par,Wparam,(PublisherListener*)&this->m_CommandPubListener);

	if(mp_datasub == NULL || mp_commandsub == NULL || mp_commandpub == NULL)
		ready = false;

	mp_par->stopParticipantAnnouncement();
	eClock::my_sleep(5000);
}

void ThroughputSubscriber::run()
{
	if(!ready)
		return;
	cout << "Waiting for discovery"<<endl;
	sema.wait();
	sema.wait();
	sema.wait();
	cout << "Discovery complete"<<endl;
	bool stop = false;
	//printLabelsSubscriber();
	int demindex=0;
	while(1)
	{
		sema.wait();
	//	cout << "Received command of type: "<< m_CommandSubListener.m_commandin << endl;
		switch(m_CommandSubListener.m_commandin.m_command)
		{
		case (DEFAULT):
		case (BEGIN):
		{
			eClock::my_sleep(100);
			break;
		}
		case (READY_TO_START):
		{
			m_datasize = m_CommandSubListener.m_commandin.m_size;
			m_demand = m_CommandSubListener.m_commandin.m_demand;
			//cout << "Ready to start data size: " << m_datasize << " and demand; "<<m_demand << endl;
			m_DataSubListener.latencyin = new LatencyType(m_datasize);
			ThroughputCommandType command(BEGIN);
			eClock::my_sleep(50);
			m_DataSubListener.reset();
			mp_commandpub->write(&command);
			break;
		}
		case (TEST_STARTS):
		{
			break;
		}
		case (TEST_ENDS):
		{
			cout << "TEST ends, sending results"<<endl;
			ThroughputCommandType comm;
			comm.m_command = TEST_RESULTS;
			comm.m_demand = m_demand;
			comm.m_size = m_datasize+4+4;
			comm.m_lastrecsample = m_DataSubListener.saved_lastseqnum;
			comm.m_lostsamples = m_DataSubListener.saved_lostsamples;
			comm.m_totaltime = TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1);
			mp_commandpub->write(&comm);
			demindex++;
			size_t remvoed;
			mp_commandpub->removeAllChange(&remvoed);
			cout << "Removed changes: "<<remvoed<<endl;
			break;
		}
		case (ALL_STOPS):
				return;
		case(TEST_RESULTS):
				break;
		}
	if(stop)
			break;
	}

}






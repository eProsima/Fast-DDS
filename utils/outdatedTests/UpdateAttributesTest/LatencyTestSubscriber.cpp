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
 * @file LatencyTestSubscriber.cpp
 *
 */

#include "LatencyTestSubscriber.h"

uint32_t datassub[] = {12,28,60,124,252,508,1020,2044,4092,8188,12284};
std::vector<uint32_t> data_size_sub (datassub, datassub + sizeof(datassub) / sizeof(uint32_t) );

LatencyTestSubscriber::LatencyTestSubscriber():
				mp_RTPSParticipant(NULL),
				mp_datapub(NULL),
				mp_commandpub(NULL),
				mp_datasub(NULL),
				mp_commandsub(NULL),
				mp_latency(NULL),
				m_disc_sema(0),
				m_comm_sema(0),
				m_data_sema(0),
				m_status(0),
				n_received(0),
				n_samples(0),
				m_datapublistener(this),
				m_datasublistener(this),
				m_commandpublistener(this),
				m_commandsublistener(this),
				m_echo(true)
{


}

LatencyTestSubscriber::~LatencyTestSubscriber()
{

}

bool LatencyTestSubscriber::init(bool echo,int nsam)
{
	m_echo = echo;
	n_samples = nsam;
	RTPSParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.domainId = 80;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.sendSocketBufferSize = 65536;
	PParam.listenSocketBufferSize = 2*65536;
	PParam.name = "RTPSParticipant_sub";
	mp_RTPSParticipant = DomainRTPSParticipant::createRTPSParticipant(PParam);
	if(mp_RTPSParticipant == NULL)
		return false;

	// DATA PUBLISHER
	PublisherAttributes PubDataparam;
	PubDataparam.topic.topicDataType = "LatencyType";
	PubDataparam.topic.topicKind = NO_KEY;
	PubDataparam.topic.topicName = "LatencySUB2PUB";
	PubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	PubDataparam.topic.historyQos.depth = n_samples+100;
	PubDataparam.topic.resourceLimitsQos.max_samples = n_samples+100;
	PubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples+100;
	PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_datapub = DomainRTPSParticipant::createPublisher(mp_RTPSParticipant,PubDataparam,(PublisherListener*)&this->m_datapublistener);
	if(mp_datapub == NULL)
		return false;
	//DATA SUBSCRIBER
	SubscriberAttributes SubDataparam;
	Locator_t loc;
	loc.port = 7555;
	PubDataparam.unicastLocatorList.push_back(loc);
	SubDataparam.topic.topicDataType = "LatencyType";
	SubDataparam.topic.topicKind = NO_KEY;
	SubDataparam.topic.topicName = "LatencyPUB2SUB";
	SubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubDataparam.topic.historyQos.depth = 100;
	SubDataparam.topic.resourceLimitsQos.max_samples = n_samples+100;
	SubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples+100;
	SubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_datasub = DomainRTPSParticipant::createSubscriber(mp_RTPSParticipant,SubDataparam,&this->m_datasublistener);
	if(mp_datasub == NULL)
		return false;
	//COMMAND PUBLISHER
	PublisherAttributes PubCommandParam;
	PubCommandParam.topic.topicDataType = "TestCommandType";
	PubCommandParam.topic.topicKind = NO_KEY;
	PubCommandParam.topic.topicName = "CommandSUB2PUB";
	PubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	PubCommandParam.topic.historyQos.depth = 100;
	PubCommandParam.topic.resourceLimitsQos.max_samples = 50;
	PubCommandParam.topic.resourceLimitsQos.allocated_samples = 50;
	mp_commandpub = DomainRTPSParticipant::createPublisher(mp_RTPSParticipant,PubCommandParam,&this->m_commandpublistener);
	if(mp_commandpub == NULL)
		return false;
	SubscriberAttributes SubCommandParam;
	SubCommandParam.topic.topicDataType = "TestCommandType";
	SubCommandParam.topic.topicKind = NO_KEY;
	SubCommandParam.topic.topicName = "CommandPUB2SUB";
	SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubCommandParam.topic.historyQos.depth = 100;
	SubCommandParam.topic.resourceLimitsQos.max_samples = 50;
	SubCommandParam.topic.resourceLimitsQos.allocated_samples = 50;
	SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	mp_commandsub = DomainRTPSParticipant::createSubscriber(mp_RTPSParticipant,SubCommandParam,&this->m_commandsublistener);
	if(mp_commandsub == NULL)
		return false;
	return true;
}



void LatencyTestSubscriber::DataPubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << RTPS_MAGENTA << "Data Pub Matched "<<RTPS_DEF<<endl;
		mp_up->m_disc_sema.post();
	}
}

void LatencyTestSubscriber::DataSubListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << RTPS_MAGENTA << "Data Sub Matched "<<RTPS_DEF<<endl;
		mp_up->m_disc_sema.post();
	}
}



void LatencyTestSubscriber::CommandPubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << RTPS_MAGENTA << "Command Pub Matched "<<RTPS_DEF<<endl;
		mp_up->m_disc_sema.post();
	}
}

void LatencyTestSubscriber::CommandSubListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << RTPS_MAGENTA << "Command Sub Matched "<<RTPS_DEF<<endl;
		mp_up->m_disc_sema.post();
	}
}

void LatencyTestSubscriber::CommandSubListener::onNewDataMessage()
{
	TestCommandType command;
	mp_up->mp_commandsub->takeNextData(&command,&mp_up->m_sampleinfo);
	cout << "RCOMMAND: "<< command.m_command << endl;
	if(command.m_command == READY)
	{
		cout << "Publisher has new test ready..."<<endl;
		mp_up->m_comm_sema.post();
	}
	else if(command.m_command == STOP_ERROR)
	{
		mp_up->m_status = -1;
		mp_up->m_comm_sema.post();
	}
}

void LatencyTestSubscriber::DataSubListener::onNewDataMessage()
{
	mp_up->mp_datasub->takeNextData((void*)mp_up->mp_latency,&mp_up->m_sampleinfo);
	//	cout << "R: "<< mp_up->mp_latency->seqnum << "|"<<mp_up->m_echo<<std::flush;
	//	eClock::my_sleep(50);
	if(mp_up->m_echo)
		mp_up->mp_datapub->write((void*)mp_up->mp_latency);
	if(mp_up->mp_latency->seqnum == (uint32_t)mp_up->n_samples)
		mp_up->m_data_sema.post();
}


void LatencyTestSubscriber::run()
{
	//WAIT FOR THE DISCOVERY PROCESS FO FINISH:
	//EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
	for(uint8_t i = 0;i<4;++i)
	{
		m_disc_sema.wait();
	}
	cout << RTPS_B_MAGENTA << "DISCOVERY COMPLETE "<<RTPS_DEF<<endl;
	for(std::vector<uint32_t>::iterator ndata = data_size_sub.begin();ndata!=data_size_sub.end();++ndata)
	{
		if(!this->test(*ndata))
			break;
	}
}

bool LatencyTestSubscriber::test(uint32_t datasize)
{
	cout << "Preparing test with data size: " << datasize+4<<endl;
	mp_latency = new LatencyType(datasize);
	m_comm_sema.wait();
	m_status = 0;
	n_received = 0;
	TestCommandType command;
	command.m_command = BEGIN;
	mp_commandpub->write(&command);

	cout << "Testing with data size: " << datasize+4<<endl;
	m_data_sema.wait();
	cout << "TEST OF SiZE: "<< datasize +4 << " ENDS"<<endl;
	size_t removed;
	this->mp_datapub->removeAllChange(&removed);
	cout << "REMOVED: "<< removed<<endl;
	delete(mp_latency);
	if(m_status == -1)
		return false;
	return true;
}

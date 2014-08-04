/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencyTestSubscriber.cpp
 *
 */

#include "LatencyTestSubscriber.h"

uint32_t datassub[] = {12,28,60,124,252,508,1020,2044,4092,8188,12284};
std::vector<uint32_t> data_size_sub (datassub, datassub + sizeof(datassub) / sizeof(uint32_t) );

LatencyTestSubscriber::LatencyTestSubscriber():
mp_participant(NULL),
mp_datapub(NULL),
mp_commandpub(NULL),
mp_datasub(NULL),
mp_commandsub(NULL),
mp_latency(NULL),
sema(0),
m_status(0),
n_received(0),
m_datapublistener(this),
m_datasublistener(this),
m_commandpublistener(this),
m_commandsublistener(this)
{


}

LatencyTestSubscriber::~LatencyTestSubscriber()
{
	if(mp_participant!=NULL)
		DomainParticipant::removeParticipant(mp_participant);
}

bool LatencyTestSubscriber::init()
{

	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.domainId = 80;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.name = "participant_pub";
	mp_participant = DomainParticipant::createParticipant(PParam);
	if(mp_participant == NULL)
		return false;

	// DATA PUBLISHER
	PublisherAttributes PubDataparam;
	PubDataparam.topic.topicDataType = "LatencyType";
	PubDataparam.topic.topicKind = NO_KEY;
	PubDataparam.topic.topicName = "LatencySUB2PUB";
	PubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	PubDataparam.topic.historyQos.depth = NSAMPLES+100;
	PubDataparam.topic.resourceLimitsQos.max_samples = NSAMPLES+100;
	PubDataparam.topic.resourceLimitsQos.allocated_samples = NSAMPLES+100;
	PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_datapub = DomainParticipant::createPublisher(mp_participant,PubDataparam,(PublisherListener*)&this->m_datapublistener);
	if(mp_datapub == NULL)
		return false;
	//DATA SUBSCRIBER
	SubscriberAttributes SubDataparam;
	SubDataparam.topic.topicDataType = "LatencyType";
	SubDataparam.topic.topicKind = NO_KEY;
	SubDataparam.topic.topicName = "LatencyPUB2SUB";
	SubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubDataparam.topic.historyQos.depth = 100;
	SubDataparam.topic.resourceLimitsQos.max_samples = NSAMPLES+100;
	SubDataparam.topic.resourceLimitsQos.allocated_samples = NSAMPLES+100;
	SubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	mp_datasub = DomainParticipant::createSubscriber(mp_participant,SubDataparam,&this->m_datasublistener);
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
	mp_commandpub = DomainParticipant::createPublisher(mp_participant,PubCommandParam,&this->m_commandpublistener);
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
	mp_commandsub = DomainParticipant::createSubscriber(mp_participant,SubCommandParam,&this->m_commandsublistener);
	if(mp_commandsub == NULL)
		return false;
	return true;
}



void LatencyTestSubscriber::DataPubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
		mp_up->sema.post();
}

void LatencyTestSubscriber::DataSubListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
		mp_up->sema.post();
}



void LatencyTestSubscriber::CommandPubListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
		mp_up->sema.post();
}

void LatencyTestSubscriber::CommandSubListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
		mp_up->sema.post();
}

void LatencyTestSubscriber::CommandSubListener::onNewDataMessage()
{
	TestCommandType command;
	mp_up->mp_commandsub->takeNextData(&command,&mp_up->m_sampleinfo);
	if(command.m_command == READY)
	{
		command.m_command = BEGIN;
		mp_up->mp_commandpub->write(&command);
		mp_up->sema.post();
	}
	else if(command.m_command == STOP_ERROR)
	{
		mp_up->m_status = -1;
		mp_up->sema.post();
	}
}

void LatencyTestSubscriber::DataSubListener::onNewDataMessage()
{
	mp_up->mp_datasub->takeNextData((void*)mp_up->mp_latency,&mp_up->m_sampleinfo);
	//cout << "R: "<<m_latency->seqnum<< "|";
	mp_up->mp_datapub->write((void*)mp_up->mp_latency);
	mp_up->n_received++;
	if(mp_up->n_received == NSAMPLES)
		mp_up->sema.post();
}


void LatencyTestSubscriber::run()
{
	//WAIT FOR THE DISCOVERY PROCESS FO FINISH:
	//EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
	for(uint8_t i = 0;i<4;++i)
	{
		sema.wait();
	}
	printf("------,------,------,------,------,------,------,------,------,\n");
	for(std::vector<uint32_t>::iterator ndata = data_size_sub.begin();ndata!=data_size_sub.end();++ndata)
	{
		if(!this->test(*ndata))
			break;
	}
}

bool LatencyTestSubscriber::test(uint32_t datasize)
{
	mp_latency = new LatencyType(datasize);
	sema.wait();
	m_status = 0;
	cout << "Test with data size: " << datasize+4<<endl;
	sema.wait();
	delete(mp_latency);
	if(m_status == -1)
		return false;
	return true;
}

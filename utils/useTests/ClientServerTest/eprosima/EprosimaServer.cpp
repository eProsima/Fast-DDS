/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EprosimaServer.cpp
 *
 */

#include "EprosimaServer.h"



EprosimaServer::EprosimaServer():
mp_operation_sub(NULL),
mp_result_pub(NULL),
mp_participant(NULL),
mp_resultdatatype(NULL),
mp_operationdatatype(NULL),
m_operationsListener(this),
m_resultsListener(this)
{


}

EprosimaServer::~EprosimaServer()
{
	if(mp_participant!=NULL)
	{
		DomainParticipant::removeParticipant(mp_participant);
	}
	if(mp_resultdatatype!=NULL)
		delete(mp_resultdatatype);
	if(mp_operationdatatype!=NULL)
		delete(mp_operationdatatype);
}

void EprosimaServer::serve()
{
	cout << "Enter a number to stop the server: ";
	int aux;
	std::cin >> aux;
}

bool EprosimaServer::init()
{
	//REGISTER TYPES
	mp_resultdatatype = new ResultDataType();
	mp_operationdatatype = new OperationDataType();
	DomainParticipant::registerType(mp_resultdatatype);
	DomainParticipant::registerType(mp_operationdatatype);

	//CREATE PARTICIPANT
	ParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.domainId = 80;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_ParticipantDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.sendSocketBufferSize = 8712;
	PParam.listenSocketBufferSize = 17424;
	PParam.name = "server_participant";
	mp_participant = DomainParticipant::createParticipant(PParam);
	if(mp_participant == NULL)
		return false;

	// DATA PUBLISHER
	PublisherAttributes PubDataparam;
	PubDataparam.topic.topicDataType = "Result";
	PubDataparam.topic.topicKind = NO_KEY;
	PubDataparam.topic.topicName = "Results";
	PubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	PubDataparam.topic.historyQos.depth = 1000;
	PubDataparam.topic.resourceLimitsQos.max_samples = 1000;
	PubDataparam.topic.resourceLimitsQos.allocated_samples = 1000;
	PubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	mp_result_pub = DomainParticipant::createPublisher(mp_participant,PubDataparam,(PublisherListener*)&this->m_operationsListener);
	if(mp_result_pub == NULL)
		return false;
	//DATA SUBSCRIBER
	SubscriberAttributes SubDataparam;
	Locator_t loc;
	loc.port = 7555;
	PubDataparam.unicastLocatorList.push_back(loc);
	SubDataparam.topic.topicDataType = "Operation";
	SubDataparam.topic.topicKind = NO_KEY;
	SubDataparam.topic.topicName = "Operations";
	SubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	SubDataparam.topic.historyQos.depth = 1000;
	SubDataparam.topic.resourceLimitsQos.max_samples = 1000;
	SubDataparam.topic.resourceLimitsQos.allocated_samples = 1000;
	mp_operation_sub = DomainParticipant::createSubscriber(mp_participant,SubDataparam,(SubscriberListener*)&this->m_resultsListener);
	if(mp_operation_sub == NULL)
		return false;

	return true;


}

Result::RESULTTYPE EprosimaServer::calculate(Operation::OPERATIONTYPE type,
		int32_t num1, int32_t num2, int32_t* result)
{
	switch(type)
	{
	case Operation::SUBTRACTION:
	{
		*result = num1-num2;
		break;
	}
	case Operation::ADDITION:
	{
		*result = num1+num2;
		break;
	}

	case Operation::MULTIPLICATION:
	{
		*result = num1*num2;
		break;
	}
	case Operation::DIVISION:
	{
		if(num2 == 0)
			return Result::ERROR_RESULT;
		break;
	}
	}
	return Result::GOOD_RESULT;
}

void EprosimaServer::OperationListener::onNewDataMessage()
{
	mp_up->mp_operation_sub->takeNextData((void*)&m_operation,&m_sampleInfo);
	if(m_sampleInfo.sampleKind == ALIVE)
	{
		m_result.m_guid = m_operation.m_guid;
		m_result.m_operationId = m_operation.m_operationId;
		m_result.m_result = 0;
		m_result.m_resultType = mp_up->calculate(m_operation.m_operationType,
				m_operation.m_num1,m_operation.m_num2,&m_result.m_result);
		mp_up->mp_result_pub->write((void*)&m_result);
	}
}

void EprosimaServer::OperationListener::onSubscriptionMatched(MatchingInfo info)
{
}



void EprosimaServer::ResultListener::onPublicationMatched(MatchingInfo info)
{
}

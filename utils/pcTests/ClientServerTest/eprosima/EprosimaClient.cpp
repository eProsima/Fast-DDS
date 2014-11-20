/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EprosimaClient.cpp
 *
 */

#include "EprosimaClient.h"

EprosimaClient::EprosimaClient():
mp_operation_pub(NULL),
mp_result_sub(NULL),
mp_RTPSParticipant(NULL),
mp_resultdatatype(NULL),
mp_operationdatatype(NULL),
m_operationsListener(this),
m_resultsListener(this),
m_isReady(false),
m_operationMatched(0),
m_resultMatched(0)
{


}

EprosimaClient::~EprosimaClient()
{
	if(mp_resultdatatype!=NULL)
		delete(mp_resultdatatype);
	if(mp_operationdatatype!=NULL)
		delete(mp_operationdatatype);
}

bool EprosimaClient::init()
{
	//REGISTER TYPES
	mp_resultdatatype = new ResultDataType();
	mp_operationdatatype = new OperationDataType();
	DomainRTPSParticipant::registerType(mp_resultdatatype);
	DomainRTPSParticipant::registerType(mp_operationdatatype);

	//CREATE RTPSParticipant
	RTPSParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.builtin.domainId = 80;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.sendSocketBufferSize = 8712;
	PParam.listenSocketBufferSize = 17424;
	PParam.name = "client_RTPSParticipant";
	mp_RTPSParticipant = DomainRTPSParticipant::createRTPSParticipant(PParam);
	if(mp_RTPSParticipant == NULL)
		return false;

	// DATA PUBLISHER
	PublisherAttributes PubDataparam;
	PubDataparam.topic.topicDataType = "Operation";
	PubDataparam.topic.topicKind = NO_KEY;
	PubDataparam.topic.topicName = "Operations";
	PubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	PubDataparam.topic.historyQos.depth = 2;
	PubDataparam.topic.resourceLimitsQos.max_samples = 50;
	PubDataparam.topic.resourceLimitsQos.allocated_samples = 50;
	PubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	mp_operation_pub = DomainRTPSParticipant::createPublisher(mp_RTPSParticipant,PubDataparam,(PublisherListener*)&this->m_operationsListener);
	if(mp_operation_pub == NULL)
		return false;
	//DATA SUBSCRIBER
	SubscriberAttributes SubDataparam;
	Locator_t loc;
	loc.port = 7555;
	PubDataparam.unicastLocatorList.push_back(loc);
	SubDataparam.topic.topicDataType = "Result";
	SubDataparam.topic.topicKind = NO_KEY;
	SubDataparam.topic.topicName = "Results";
	SubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	SubDataparam.topic.historyQos.depth = 100;
	SubDataparam.topic.resourceLimitsQos.max_samples = 100;
	SubDataparam.topic.resourceLimitsQos.allocated_samples = 100;
	mp_result_sub = DomainRTPSParticipant::createSubscriber(mp_RTPSParticipant,SubDataparam,(SubscriberListener*)&this->m_resultsListener);
	if(mp_result_sub == NULL)
		return false;

	return true;
}

Result::RESULTTYPE EprosimaClient::calculate(Operation::OPERATIONTYPE type, int32_t num1,int32_t num2,int32_t* result)
{
	if(!m_isReady)
		return Result::SERVER_NOT_READY;
	m_operation.m_operationId++;
	m_operation.m_operationType = type;
	m_operation.m_num1 = num1;
	m_operation.m_num2 = num2;

	mp_operation_pub->write((void*)&m_operation);
	do{
		resetResult();
		mp_result_sub->waitForUnreadMessage();
		mp_result_sub->takeNextData((void*)&m_result,&m_sampleInfo);
	}while(m_sampleInfo.sampleKind !=ALIVE ||
			m_result.m_guid != m_operation.m_guid ||
			m_result.m_operationId != m_operation.m_operationId);
	if(m_result.m_resultType == Result::GOOD_RESULT)
	{
		*result = m_result.m_result;
	}
	return m_result.m_resultType;
}

void EprosimaClient::resetResult()
{
	m_result.m_guid = c_Guid_Unknown;
	m_result.m_operationId = 0;
	m_result.m_result = 0;
}

void EprosimaClient::OperationListener::onPublicationMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		mp_up->m_operationMatched++;
	}
	else
		mp_up->m_operationMatched--;
	mp_up->isReady();
}

void EprosimaClient::ResultListener::onSubscriptionMatched(MatchingInfo info)
{
	if(info.status == MATCHED_MATCHING)
	{
		mp_up->m_resultMatched++;
	}
	else
		mp_up->m_resultMatched--;
	mp_up->isReady();
}

void EprosimaClient::ResultListener::onNewDataMessage()
{
}

bool EprosimaClient::isReady()
{
	if(m_operationMatched == 1 && m_resultMatched == 1)
		m_isReady = true;
	else
		m_isReady = false;
	return m_isReady;
}

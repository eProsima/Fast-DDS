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
 * @file EprosimaClient.cpp
 *
 */

#include "EprosimaClient.h"

#include "fastrtps/fastrtps_all.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace clientserver;

EprosimaClient::EprosimaClient()
    : mp_operation_pub(nullptr)
    , mp_result_sub(nullptr)
    , mp_participant(nullptr)
    , mp_resultdatatype(nullptr)
    , mp_operationdatatype(nullptr)
    , m_operationsListener(nullptr)
    , m_resultsListener(nullptr)
    , m_isReady(false)
    , m_operationMatched(0)
    , m_resultMatched(0)
{
    m_operationsListener.mp_up = this;
    m_resultsListener.mp_up = this;

}

EprosimaClient::~EprosimaClient()
{
    Domain::removeParticipant(mp_participant);
    if (mp_resultdatatype != nullptr)
    {
        delete(mp_resultdatatype);
    }
    if (mp_operationdatatype != nullptr)
    {
        delete(mp_operationdatatype);
    }

}

bool EprosimaClient::init()
{
    //CREATE RTPSParticipant
    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName( "client_RTPSParticipant");
    mp_participant = Domain::createParticipant(PParam);
    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER TYPES
    mp_resultdatatype = new ResultDataType();
    mp_operationdatatype = new OperationDataType();
    Domain::registerType(mp_participant, mp_resultdatatype);
    Domain::registerType(mp_participant, mp_operationdatatype);

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
    mp_operation_pub = Domain::createPublisher(mp_participant, PubDataparam,
                    (PublisherListener*)&this->m_operationsListener);
    if (mp_operation_pub == nullptr)
    {
        return false;
    }
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
    mp_result_sub =
            Domain::createSubscriber(mp_participant, SubDataparam, (SubscriberListener*)&this->m_resultsListener);
    if (mp_result_sub == nullptr)
    {
        return false;
    }

    return true;
}

Result::RESULTTYPE EprosimaClient::calculate(
        Operation::OPERATIONTYPE type,
        int32_t num1,
        int32_t num2,
        int32_t* result)
{
    if (!m_isReady)
    {
        return Result::SERVER_NOT_READY;
    }
    m_operation.m_operationId++;
    m_operation.m_operationType = type;
    m_operation.m_num1 = num1;
    m_operation.m_num2 = num2;

    mp_operation_pub->write((void*)&m_operation);
    do {
        resetResult();
        mp_result_sub->wait_for_unread_samples({10, 0});
        mp_result_sub->takeNextData((void*)&m_result, &m_sampleInfo);
    } while (m_sampleInfo.sampleKind != ALIVE ||
            m_result.m_guid != m_operation.m_guid ||
            m_result.m_operationId != m_operation.m_operationId);
    if (m_result.m_resultType == Result::GOOD_RESULT)
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

void EprosimaClient::OperationListener::onPublicationMatched(
        Publisher*,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        mp_up->m_operationMatched++;
    }
    else
    {
        mp_up->m_operationMatched--;
    }
    mp_up->isReady();
}

void EprosimaClient::ResultListener::onSubscriptionMatched(
        Subscriber*,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        mp_up->m_resultMatched++;
    }
    else
    {
        mp_up->m_resultMatched--;
    }
    mp_up->isReady();
}

void EprosimaClient::ResultListener::onNewDataMessage(
        Subscriber*)
{
}

bool EprosimaClient::isReady()
{
    if (m_operationMatched == 1 && m_resultMatched == 1)
    {
        m_isReady = true;
    }
    else
    {
        m_isReady = false;
    }
    return m_isReady;
}

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
 * @file EprosimaServer.cpp
 *
 */

#include "EprosimaServer.h"

#include "fastrtps/fastrtps_all.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace clientserver;
using namespace std;

EprosimaServer::EprosimaServer()
    : mp_operation_sub(nullptr)
    , mp_result_pub(nullptr)
    , mp_participant(nullptr)
    , mp_resultdatatype(nullptr)
    , mp_operationdatatype(nullptr)
    , m_n_served(0)
    , m_operationsListener(nullptr)
    , m_resultsListener(nullptr)
{
    m_operationsListener.mp_up = this;
    m_resultsListener.mp_up = this;

}

EprosimaServer::~EprosimaServer()
{
    if (mp_participant != nullptr)
    {
        Domain::removeParticipant(mp_participant);
    }
    if (mp_resultdatatype != nullptr)
    {
        delete(mp_resultdatatype);
    }
    if (mp_operationdatatype != nullptr)
    {
        delete(mp_operationdatatype);
    }
}

void EprosimaServer::serve()
{
    cout << "Enter a number to stop the server: ";
    int aux;
    std::cin >> aux;
}

void EprosimaServer::serve(
        uint32_t samples)
{
    while (m_n_served < samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool EprosimaServer::init()
{
    //CREATE RTPSParticipant
    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("server_RTPSParticipant");
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
    PubDataparam.topic.topicDataType = "Result";
    PubDataparam.topic.topicKind = NO_KEY;
    PubDataparam.topic.topicName = "Results";
    PubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    PubDataparam.topic.historyQos.depth = 1000;
    PubDataparam.topic.resourceLimitsQos.max_samples = 1500;
    PubDataparam.topic.resourceLimitsQos.allocated_samples = 1000;
    PubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    mp_result_pub = Domain::createPublisher(mp_participant, PubDataparam, (PublisherListener*)&this->m_resultsListener);
    if (mp_result_pub == nullptr)
    {
        return false;
    }
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
    SubDataparam.topic.resourceLimitsQos.max_samples = 1500;
    SubDataparam.topic.resourceLimitsQos.allocated_samples = 1000;
    mp_operation_sub = Domain::createSubscriber(mp_participant, SubDataparam,
                    (SubscriberListener*)&this->m_operationsListener);
    if (mp_operation_sub == nullptr)
    {
        return false;
    }

    return true;
}

Result::RESULTTYPE EprosimaServer::calculate(
        Operation::OPERATIONTYPE type,
        int32_t num1,
        int32_t num2,
        int32_t* result)
{
    switch (type)
    {
        case Operation::SUBTRACTION:
        {
            *result = num1 - num2;
            break;
        }
        case Operation::ADDITION:
        {
            *result = num1 + num2;
            break;
        }

        case Operation::MULTIPLICATION:
        {
            *result = num1 * num2;
            break;
        }
        case Operation::DIVISION:
        {
            if (num2 == 0)
            {
                return Result::ERROR_RESULT;
            }
            break;
        }
    }
    return Result::GOOD_RESULT;
}

void EprosimaServer::OperationListener::onNewDataMessage(
        Subscriber*)
{
    mp_up->mp_operation_sub->takeNextData((void*)&m_operation, &m_sampleInfo);
    if (m_sampleInfo.sampleKind == ALIVE)
    {
        ++mp_up->m_n_served;
        m_result.m_guid = m_operation.m_guid;
        m_result.m_operationId = m_operation.m_operationId;
        m_result.m_result = 0;
        m_result.m_resultType = mp_up->calculate(
            m_operation.m_operationType,
            m_operation.m_num1,
            m_operation.m_num2,
            &m_result.m_result);
        mp_up->mp_result_pub->write((void*)&m_result);
    }
}

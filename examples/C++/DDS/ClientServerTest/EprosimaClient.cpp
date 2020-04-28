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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;
using namespace clientserver;

EprosimaClient::EprosimaClient()
    : mp_operation_pub(nullptr)
    , mp_result_sub(nullptr)
    , mp_participant(nullptr)
    , mp_resultdatatype(new ResultDataType())
    , mp_operationdatatype(new OperationDataType())
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
    if (mp_operation_writer != nullptr)
    {
        mp_operation_pub->delete_datawriter(mp_operation_writer);
    }
    if (mp_operation_pub != nullptr)
    {
        mp_participant->delete_publisher(mp_operation_pub);
    }
    if (mp_operation_topic != nullptr)
    {
        mp_participant->delete_topic(mp_operation_topic);
    }
    if (mp_result_reader != nullptr)
    {
        mp_result_sub->delete_datareader(mp_result_reader);
    }
    if (mp_result_sub != nullptr)
    {
        mp_participant->delete_subscriber(mp_result_sub);
    }
    if (mp_result_topic != nullptr)
    {
        mp_participant->delete_topic(mp_result_topic);
    }
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

bool EprosimaClient::init()
{
    //CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name( "client_RTPSParticipant");

    mp_participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER TYPES
    mp_resultdatatype.register_type(mp_participant);
    mp_operationdatatype.register_type(mp_participant);

    //CREATE THE PUBLISHER
    mp_operation_pub = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (mp_operation_pub == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    mp_operation_topic = mp_participant->create_topic("Operations", "Operation", TOPIC_QOS_DEFAULT);

    if (mp_operation_topic == nullptr)
    {
        return false;
    }

    //CREATE THE DATAWRITER
    DataWriterQos wqos;
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 2;
    wqos.resource_limits().max_samples = 50;
    wqos.resource_limits().allocated_samples = 50;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    mp_operation_writer = mp_operation_pub->create_datawriter(mp_operation_topic, wqos, &this->m_operationsListener);

    if (mp_operation_writer == nullptr)
    {
        return false;
    }

    //CREATE THE SUBSCRIBER
    mp_result_sub = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (mp_result_sub == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    mp_result_topic = mp_participant->create_topic("Results", "Result", TOPIC_QOS_DEFAULT);

    if (mp_result_topic == nullptr)
    {
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 100;
    rqos.resource_limits().max_samples = 100;
    rqos.resource_limits().allocated_samples = 100;
    mp_result_reader = mp_result_sub->create_datareader(mp_result_topic, rqos, &this->m_resultsListener);

    if (mp_result_reader == nullptr)
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
    SampleInfo m_sampleInfo;
    if (!m_isReady)
    {
        return Result::SERVER_NOT_READY;
    }
    m_operation.m_operationId++;
    m_operation.m_operationType = type;
    m_operation.m_num1 = num1;
    m_operation.m_num2 = num2;

    mp_operation_writer->write((void*)&m_operation);
    do {
        resetResult();
        mp_result_reader->wait_for_unread_message({10, 0});
        mp_result_reader->take_next_sample((void*)&m_result, &m_sampleInfo);
    } while (m_sampleInfo.instance_state != eprosima::fastdds::dds::ALIVE ||
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

void EprosimaClient::OperationListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        mp_up->m_operationMatched++;
    }
    else if (info.current_count_change == -1)
    {
        mp_up->m_operationMatched--;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
    mp_up->isReady();
}

void EprosimaClient::ResultListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        mp_up->m_resultMatched++;
    }
    else if (info.current_count_change == -1)
    {
        mp_up->m_resultMatched--;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
    mp_up->isReady();
}

void EprosimaClient::ResultListener::on_data_available(
        DataReader*)
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

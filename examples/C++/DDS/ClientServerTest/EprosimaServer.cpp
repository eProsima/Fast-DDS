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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;
using namespace clientserver;
using namespace std;

EprosimaServer::EprosimaServer()
    : mp_operation_sub(nullptr)
    , mp_result_pub(nullptr)
    , mp_participant(nullptr)
    , mp_resultdatatype(new ResultDataType())
    , mp_operationdatatype(new OperationDataType())
    , m_n_served(0)
    , m_operationsListener(nullptr)
    , m_resultsListener(nullptr)
{
    m_operationsListener.mp_up = this;
    m_resultsListener.mp_up = this;

}

EprosimaServer::~EprosimaServer()
{
    if (mp_operation_reader != nullptr)
    {
        mp_operation_sub->delete_datareader(mp_operation_reader);
    }
    if (mp_operation_sub != nullptr)
    {
        mp_participant->delete_subscriber(mp_operation_sub);
    }
    if (mp_operation_topic != nullptr)
    {
        mp_participant->delete_topic(mp_operation_topic);
    }
    if (mp_result_writer != nullptr)
    {
        mp_result_pub->delete_datawriter(mp_result_writer);
    }
    if (mp_result_pub != nullptr)
    {
        mp_participant->delete_publisher(mp_result_pub);
    }
    if (mp_result_topic != nullptr)
    {
        mp_participant->delete_topic(mp_result_topic);
    }
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
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
    //CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("server_RTPSParticipant");

    mp_participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER TYPES
    mp_resultdatatype.register_type(mp_participant);
    mp_operationdatatype.register_type(mp_participant);

    // CREATE THE PUBLISHER
    mp_result_pub = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (mp_result_pub == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    mp_result_topic = mp_participant->create_topic("Results", "Result", TOPIC_QOS_DEFAULT);

    if (mp_result_topic == nullptr)
    {
        return false;
    }

    //CREATE THE DATAWRITER
    DataWriterQos wqos;
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 1000;
    wqos.resource_limits().max_samples = 1500;
    wqos.resource_limits().allocated_samples = 1000;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    mp_result_writer = mp_result_pub->create_datawriter(mp_result_topic, wqos, &m_resultsListener);

    if (mp_result_writer == nullptr)
    {
        return false;
    }

    //CREATE THE SUBSCRIBER
    mp_operation_sub = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (mp_operation_sub == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    mp_operation_topic = mp_participant->create_topic("Operations", "Operation", TOPIC_QOS_DEFAULT);

    if (mp_operation_topic == nullptr)
    {
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 1000;
    rqos.resource_limits().max_samples = 1500;
    rqos.resource_limits().allocated_samples = 1000;

    mp_operation_reader = mp_operation_sub->create_datareader(mp_operation_topic, rqos, &m_operationsListener);

    if (mp_operation_reader == nullptr)
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

void EprosimaServer::OperationListener::on_data_available(
        DataReader* /*reader*/)
{
    SampleInfo m_sampleInfo;
    mp_up->mp_operation_reader->take_next_sample((void*)&m_operation, &m_sampleInfo);
    if (m_sampleInfo.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
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
        mp_up->mp_result_writer->write((void*)&m_result);
    }
}

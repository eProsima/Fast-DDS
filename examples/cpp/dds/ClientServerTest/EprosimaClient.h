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
 * @file EprosimaClient.h
 *
 */

#ifndef EPROSIMACLIENT_H_
#define EPROSIMACLIENT_H_

#include "ClientServerTypes.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

class EprosimaClient
{
public:

    EprosimaClient();

    virtual ~EprosimaClient();

    eprosima::fastdds::dds::Publisher* mp_operation_pub;

    eprosima::fastdds::dds::DataWriter* mp_operation_writer;

    eprosima::fastdds::dds::Subscriber* mp_result_sub;

    eprosima::fastdds::dds::DataReader* mp_result_reader;

    eprosima::fastdds::dds::Topic* mp_operation_topic;

    eprosima::fastdds::dds::Topic* mp_result_topic;

    eprosima::fastdds::dds::DomainParticipant* mp_participant;

    bool init();

    clientserver::Result::RESULTTYPE calculate(
            clientserver::Operation::OPERATIONTYPE type,
            int32_t num1,
            int32_t num2,
            int32_t* result);

    bool isReady();

private:

    clientserver::Operation m_operation;

    clientserver::Result m_result;

    void resetResult();

    eprosima::fastdds::dds::TypeSupport mp_resultdatatype;

    eprosima::fastdds::dds::TypeSupport mp_operationdatatype;

    class OperationListener : public eprosima::fastdds::dds::DataWriterListener
    {
public:

        OperationListener(
                EprosimaClient* up)
            : mp_up(up)
        {
        }

        ~OperationListener() override
        {
        }

        EprosimaClient* mp_up;

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;


    } m_operationsListener;

    class ResultListener : public eprosima::fastdds::dds::DataReaderListener
    {
public:

        ResultListener(
                EprosimaClient* up)
            : mp_up(up)
        {
        }

        ~ResultListener() override
        {
        }

        EprosimaClient* mp_up;

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    } m_resultsListener;

    bool m_isReady;

    int m_operationMatched;

    int m_resultMatched;

};

#endif /* EPROSIMACLIENT_H_ */

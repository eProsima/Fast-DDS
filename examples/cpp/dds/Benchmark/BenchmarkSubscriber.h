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
 * @file BenchmarkSubscriber.h
 *
 */

#ifndef BENCHMARK_SUBSCRIBER_H_
#define BENCHMARK_SUBSCRIBER_H_

#include "BenchmarkPubSubTypes.h"
#include "Benchmark_smallPubSubTypes.h"
#include "Benchmark_mediumPubSubTypes.h"
#include "Benchmark_bigPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include "Benchmark.h"
#include "Benchmark_small.h"
#include "Benchmark_medium.h"
#include "Benchmark_big.h"

class BenchMarkSubscriber
{
public:

    BenchMarkSubscriber();

    virtual ~BenchMarkSubscriber();

    //!Initialize the subscriber
    bool init(
            int transport,
            eprosima::fastrtps::ReliabilityQosPolicyKind kind,
            const std::string& topicName,
            int domain,
            int size /*, bool dynamicTypes*/);

    //!RUN the subscriber
    void run();

private:

    BenchMarkBig m_HelloBig;

    BenchMarkMedium m_HelloMedium;

    BenchMarkSmall m_HelloSmall;

    BenchMark m_Hello;

    int m_iSize;

    eprosima::fastdds::dds::DomainParticipant* mp_participant;

    eprosima::fastdds::dds::Publisher* mp_publisher;

    eprosima::fastdds::dds::Subscriber* mp_subscriber;

    eprosima::fastdds::dds::DataWriter* mp_writer;

    eprosima::fastdds::dds::DataReader* mp_reader;

    eprosima::fastdds::dds::Topic* mp_topic_pub;

    eprosima::fastdds::dds::Topic* mp_topic_sub;

public:

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
public:

        PubListener(
                BenchMarkSubscriber* parent);

        ~PubListener() override
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        BenchMarkSubscriber* mParent;

        int matched_;
        bool first_connected_;

    } m_pubListener;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
public:

        SubListener()
        {
        }

        SubListener(
                BenchMarkSubscriber* parent);

        ~SubListener() override
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        BenchMarkSubscriber* mParent;

        BenchMark m_Hello;

        BenchMarkSmall m_HelloSmall;

        BenchMarkMedium m_HelloMedium;

        BenchMarkBig m_HelloBig;

        int matched_;

        uint32_t samples_;

    } m_subListener;

private:

    eprosima::fastdds::dds::TypeSupport m_type;
    eprosima::fastdds::dds::TypeSupport m_typeSmall;
    eprosima::fastdds::dds::TypeSupport m_typeMedium;
    eprosima::fastdds::dds::TypeSupport m_typeBig;
};

#endif /* BENCHMARK_SUBSCRIBER_H_ */

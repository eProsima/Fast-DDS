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
 * @file BenchMarkPublisher.h
 *
 */

#ifndef BENCHMARKPUBLISHER_H_
#define BENCHMARKPUBLISHER_H_

#include "BenchmarkPubSubTypes.h"
#include "Benchmark_smallPubSubTypes.h"
#include "Benchmark_mediumPubSubTypes.h"
#include "Benchmark_bigPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include "Benchmark.h"
#include "Benchmark_small.h"
#include "Benchmark_medium.h"
#include "Benchmark_big.h"

#include <chrono>
#include <atomic>

class BenchMarkPublisher
{
public:

    BenchMarkPublisher();

    virtual ~BenchMarkPublisher();

    //!Initialize
    bool init(
            int transport,
            eprosima::fastrtps::ReliabilityQosPolicyKind kind,
            int time,
            int tick_time,
            int wait_time,
            const std::string& topicName,
            int domain,
            int size);

    //!Publish a sample
    bool publish();

    //!Run for number samples
    void run();

private:

    std::chrono::time_point<std::chrono::system_clock> m_testStartTime;

    bool m_bBenchmarkFinished;

    int m_iTestTimeMs;

    int m_iTickTime;

    int m_iWaitTime;

    std::atomic_uint m_iCount;

    int m_iSize;

    int* m_vSamples;

    int m_iSamplesSize;

    int m_iSamplesCount;

    BenchMark m_Hello;

    BenchMarkSmall m_HelloSmall;

    BenchMarkMedium m_HelloMedium;

    BenchMarkBig m_HelloBig;

    eprosima::fastdds::dds::DomainParticipant* mp_participant;

    eprosima::fastdds::dds::Publisher* mp_publisher;

    eprosima::fastdds::dds::DataWriter* mp_writer;

    eprosima::fastdds::dds::Subscriber* mp_subscriber;

    eprosima::fastdds::dds::DataReader* mp_reader;

    eprosima::fastdds::dds::Topic* mp_topic_pub;

    eprosima::fastdds::dds::Topic* mp_topic_sub;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
public:

        PubListener()
        {
        }

        PubListener(
                BenchMarkPublisher* parent);

        ~PubListener() override
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        BenchMarkPublisher* mParent;

        int matched_;

    } m_pubListener;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
public:

        SubListener()
        {
        }

        SubListener(
                BenchMarkPublisher* parent);

        ~SubListener() override
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        BenchMarkPublisher* mParent;

        BenchMark m_Hello;

        BenchMarkSmall m_HelloSmall;

        BenchMarkMedium m_HelloMedium;

        BenchMarkBig m_HelloBig;

    } m_subListener;

    eprosima::fastdds::dds::TypeSupport m_type;

    eprosima::fastdds::dds::TypeSupport m_typeSmall;

    eprosima::fastdds::dds::TypeSupport m_typeMedium;

    eprosima::fastdds::dds::TypeSupport m_typeBig;

    void runThread();
};



#endif /* BENCHMARKPUBLISHER_H_ */

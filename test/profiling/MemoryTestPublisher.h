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
 * @file MemoryPublisher.h
 *
 */

#ifndef MEMORYPUBLISHER_H_
#define MEMORYPUBLISHER_H_

#include <chrono>
#include <condition_variable>

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>

#include "MemoryTestTypes.h"

class MemoryTestPublisher
{
public:

    MemoryTestPublisher();
    virtual ~MemoryTestPublisher();

    eprosima::fastdds::dds::DomainParticipant* participant_ {nullptr};
    eprosima::fastdds::dds::Publisher* publisher_ {nullptr};
    eprosima::fastdds::dds::Subscriber* subscriber_ {nullptr};
    eprosima::fastdds::dds::TypeSupport command_type_ {new TestCommandDataType()};
    eprosima::fastdds::dds::TypeSupport data_type_;
    eprosima::fastdds::dds::Topic* command_pub_topic_ {nullptr};
    eprosima::fastdds::dds::Topic* command_sub_topic_ {nullptr};
    eprosima::fastdds::dds::Topic* data_topic_ {nullptr};
    eprosima::fastdds::dds::DataWriter* command_writer_ {nullptr};
    eprosima::fastdds::dds::DataWriter* data_writer_ {nullptr};
    eprosima::fastdds::dds::DataReader* command_reader_ {nullptr};
    int n_subscribers {0};
    unsigned int n_samples {0};
    std::mutex mutex_;
    int disc_count_ {0};
    std::condition_variable disc_cond_;
    int comm_count_ {0};
    std::condition_variable comm_cond_;
    int data_count_ {0};
    std::condition_variable data_cond_;
    int m_status {0};
    unsigned int n_received {0};
    bool n_export_csv {false};
    std::string m_exportPrefix;

    bool init(
            int n_sub,
            int n_sam,
            bool reliable,
            uint32_t pid,
            bool hostname,
            bool export_csv,
            const std::string& export_prefix,
            const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
            const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
            const std::string& sXMLConfigFile,
            uint32_t data_size,
            bool dynamic_types);
    void run(
            uint32_t test_time);
    bool test(
            uint32_t test_time,
            uint32_t datasize);

    class DataPubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        DataPubListener(
                MemoryTestPublisher* up)
            : up_(up)
        {
        }

        ~DataPubListener() = default;

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        MemoryTestPublisher* up_ {nullptr};

        int n_matched {0};
    }
    m_datapublistener {nullptr};

    class CommandPubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        CommandPubListener(
                MemoryTestPublisher* up)
            : up_(up)
        {
        }

        ~CommandPubListener()
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        MemoryTestPublisher* up_ {nullptr};
        int n_matched {0};
    }
    m_commandpublistener {nullptr};

    class CommandSubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        CommandSubListener(
                MemoryTestPublisher* up)
            : up_(up)
        {
        }

        ~CommandSubListener()
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        MemoryTestPublisher* up_ {nullptr};
        int n_matched {0};
    }
    m_commandsublistener {nullptr};

    TestCommandDataType command_t;
    std::string m_sXMLConfigFile;
    bool reliable_;
    uint32_t m_data_size {0};
    bool dynamic_data_ {false};
    // Static Data
    MemoryType* memory_ {nullptr};
    MemoryDataType memory_t;
    // Dynamic Data
    eprosima::fastdds::dds::DynamicData::_ref_type m_DynData;
    eprosima::fastdds::dds::DynamicType::_ref_type m_pDynType;
    eprosima::fastdds::dds::DynamicPubSubType* dynamic_type_support_ {nullptr};

};


#endif /* MEMORYPUBLISHER_H_ */

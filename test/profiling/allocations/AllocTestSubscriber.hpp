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
 * @file AllocTestSubscriber.h
 *
 */

#ifndef _FASTDDS_ALLOCTESTSUBSCRIBER_H_
#define _FASTDDS_ALLOCTESTSUBSCRIBER_H_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "AllocTestType.hpp"

class AllocTestSubscriber : public eprosima::fastdds::dds::DataReaderListener
{
public:

    AllocTestSubscriber();

    virtual ~AllocTestSubscriber();

    //! Initialize the subscriber
    bool init(
            const char* profile,
            uint32_t domain_id,
            const std::string& output_file);

    //! RUN the subscriber
    void run(
            bool wait_unmatch);

    void run(
            uint32_t number,
            bool wait_unmatching);

    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& status) override;

    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

private:

    void wait_match();

    void wait_unmatch();

    void wait_until_total_received_at_least(
            uint32_t n);

    eprosima::fastdds::dds::TypeSupport type_;

    AllocTestType data_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::DataReader* reader_;

    std::string profile_;

    std::string output_file_;

    std::atomic<uint16_t> matched_;

    std::atomic<uint16_t> samples_;

    mutable std::mutex mtx_;

    std::condition_variable cv_;

};

#endif /* _FASTDDS_ALLOCTESTSUBSCRIBER_H_ */

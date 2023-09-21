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
 * @file LargeDataPublisher.h
 *
 */

#ifndef IMAGEDATAPUBLISHER_H_
#define IMAGEDATAPUBLISHER_H_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>

#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "LargeData.h"

class LargeDataPublisher : public eprosima::fastdds::dds::DataWriterListener
{
public:

    LargeDataPublisher(const uint32_t data_size);

    virtual ~LargeDataPublisher();

    //!Initialize
    bool init(const std::string &tcp_type);

    //!Run publisher
    void run(
            uint16_t frequency);

private:

    void init_msg(const uint32_t data_size);

    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    //!https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/publisher/dataWriterListener/dataWriterListener.html#on-unacknowledged-sample-removed-callback
    void on_unacknowledged_sample_removed(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::InstanceHandle_t& instance) override;

    void publish();

    LargeDataMsg msg_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

    eprosima::fastdds::dds::TypeSupport type_;

    std::atomic<int> matched_{0};

    bool first_connected_;

    static std::atomic<bool> running_;

    static std::mutex running_mtx_;

    static std::condition_variable running_cv_;

    std::mutex matched_mtx_;

    uint16_t frequency_;

    uint32_t removed_unacked_samples_;
};

#endif /* IMAGEDATAPUBLISHER_H_ */

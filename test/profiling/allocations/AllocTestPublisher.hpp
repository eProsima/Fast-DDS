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
 * @file AllocTestPublisher.h
 *
 */

#ifndef _FASTDDS_ALLOCTESTPUBLISHER_H_
#define _FASTDDS_ALLOCTESTPUBLISHER_H_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>

#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "AllocTestType.hpp"

class AllocTestPublisher : public eprosima::fastdds::dds::DataWriterListener
{
public:

    AllocTestPublisher();

    virtual ~AllocTestPublisher();

    //! Initialize
    bool init(
            const char* profile,
            uint32_t domain_id,
            const std::string& output_file);

    //! Publish a sample
    bool publish();

    //! Run for number samples
    void run(
            uint32_t number,
            bool wait_unmatching);

    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& status) override;

private:

    bool is_matched();

    void wait_match();

    void wait_unmatch();

    eprosima::fastdds::dds::TypeSupport type_;

    AllocTestType data_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::DataWriter* writer_;

    std::string profile_;

    std::string output_file_;

    std::atomic<uint16_t> matched_;

    mutable std::mutex mtx_;

    std::condition_variable cv_;

};



#endif /* _FASTDDS_ALLOCTESTPUBLISHER_H_ */

// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file CustomPayloadPoolDataPublisher.h
 *
 */

#ifndef CUSTOM_PAYLOAD_POOL_DATA_PUBLISHER_H_
#define CUSTOM_PAYLOAD_POOL_DATA_PUBLISHER_H_

#include <condition_variable>
#include <mutex>

#include "CustomPayloadPool.hpp"
#include "CustomPayloadPoolDataPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

/**
 * Class used to group into a single working unit a Publisher with a DataWriter, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class CustomPayloadPoolDataPublisher : private eprosima::fastdds::dds::DataWriterListener
{
public:

    CustomPayloadPoolDataPublisher(
            std::shared_ptr<CustomPayloadPool> payload_pool);

    virtual ~CustomPayloadPoolDataPublisher();

    //! Initialize the publisher
    bool init();

    //! Run for number samples, publish every sleep seconds
    bool run(
            uint32_t number,
            uint32_t sleep);

private:

    //! Publish a sample
    bool publish();

    //! Run thread for number samples, publish every sleep seconds
    void run_thread(
            uint32_t number,
            uint32_t sleep);

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

    //! Callback executed when a DataReader is matched or unmatched
    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    //! Return true if there are at least 1 matched DataReaders
    bool enough_matched();

    //! Block the thread until enough DataReaders are matched
    void wait();

    //! Unblock the thread so publication of samples begins/resumes
    static void awake();

    CustomPayloadPoolData hello_;

    std::shared_ptr<CustomPayloadPool> payload_pool_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

    eprosima::fastdds::dds::TypeSupport type_;

    //! Number of DataReaders matched to the associated DataWriter
    std::atomic<std::uint32_t> matched_;

    //! Member used for control flow purposes
    std::atomic<bool> has_stopped_for_unexpected_error_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects wait_matched condition variable
    static std::mutex wait_matched_cv_mtx_;

    //! Waits until enough DataReaders are matched
    static std::condition_variable wait_matched_cv_;
};


#endif /* CUSTOM_PAYLOAD_POOL_DATA_PUBLISHER_H_ */

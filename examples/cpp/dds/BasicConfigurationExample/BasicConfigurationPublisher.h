// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BasicConfigurationPublisher.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_BASICCONFIGURATIONPUBLISHER_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_BASICCONFIGURATIONPUBLISHER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "HelloWorldPubSubTypes.h"
#include "types.hpp"

/**
 * Class handling discovery events and dataflow
 */
class PubListener : public eprosima::fastdds::dds::DataWriterListener
{
public:

    PubListener()
        : matched_(0)
        , num_wait_matched_(0)
    {
    }

    ~PubListener() override
    {
    }

    //! Callback executed when a DataReader is matched or unmatched
    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    //! Set the number of matched DataReaders required for publishing
    void set_num_wait_matched(
            uint32_t num_wait_matched);

    //! Return true if there are at least num_wait_matched_ matched DataReaders
    bool enough_matched();

    //! Block the thread until enough DataReaders are matched
    void wait();

    //! Unblock the thread so publication of samples begins/resumes
    static void awake();

    std::string topic_name_;

private:

    //! Number of DataReaders matched to the associated DataWriter
    std::atomic<std::uint32_t> matched_;

    //! Number of matched DataReaders required for publishing
    uint32_t num_wait_matched_;

    //! Protects wait_matched condition variable
    static std::mutex wait_matched_cv_mtx_;

    //! Waits until enough DataReaders are matched
    static std::condition_variable wait_matched_cv_;
};

/**
 * Class used to group into a single working unit a Publisher with a DataWriter, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class HelloWorldPublisher
{
public:

    HelloWorldPublisher();

    virtual ~HelloWorldPublisher();

    //! Initialize the publisher
    bool init(
            std::vector<std::string> topic_names,
            uint32_t domain,
            uint32_t num_wait_matched,
            bool async,
            TransportType transport,
            bool reliable,
            bool transient);

    //! Publish a sample
    void publish(
            uint32_t writer_idx);

    //! Run for number samples, publish every sleep seconds
    void run(
            uint32_t number,
            uint32_t sleep,
            bool single_thread = true);

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

private:

    std::vector<HelloWorld> hellos_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    std::vector<eprosima::fastdds::dds::Topic*> topics_;

    std::vector<eprosima::fastdds::dds::DataWriter*> writers_;

    static std::atomic<uint32_t> n_topics_;

    eprosima::fastdds::dds::TypeSupport type_;

    std::vector<std::shared_ptr<PubListener>> listeners_;

    //! Run thread for number samples, publish every sleep seconds
    void runThread(
            uint32_t number,
            uint32_t sleep,
            uint32_t idx);

    void runSingleThread(
            uint32_t number,
            uint32_t sleep);

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    static std::mutex terminate_cv_mtx_;

    static std::condition_variable terminate_cv_;
};



#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_BASICCONFIGURATIONPUBLISHER_H_ */

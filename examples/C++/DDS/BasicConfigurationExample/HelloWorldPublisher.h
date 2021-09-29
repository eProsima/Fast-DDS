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
 * @file HelloWorldPublisher.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_HELLOWORLDPUBLISHER_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_HELLOWORLDPUBLISHER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "HelloWorldPubSubTypes.h"

/**
 * Class used to group into a single working unit a Publisher with a DataWriter, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class HelloWorldPublisher
{
public:

    HelloWorldPublisher();

    virtual ~HelloWorldPublisher();

    //!Initialize the publisher
    bool init(
            const std::string& topic_name,
            uint32_t domain,
            uint32_t num_wait_matched,
            bool async,
            const std::string& transport,
            bool reliable,
            bool transient);

    //!Publish a sample
    void publish();

    //!Run for number samples, publish every sleep seconds
    void run(
            uint32_t number,
            uint32_t sleep);

    // returns the current state of execution
    static bool is_stopped();

    // triggers the end of execution
    static void stop();

private:

    HelloWorld hello_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

    eprosima::fastdds::dds::TypeSupport type_;

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

        // callback executed when a DataReader is matched or unmatched
        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        // sets the number of matched DataReaders required for publishing
        void set_num_wait_matched(
                uint32_t num_wait_matched);

        // returns true if there are at least num_wait_matched_ matched DataReaders
        bool enough_matched();

        // blocks the thread until enough DataReaders are matched
        void wait();

        // unblocks the thread so publication of samples begins/resumes
        static void awake();

    private:

        // number of DataReaders matched to the associated DataWriter
        std::atomic<std::uint32_t> matched_;

        // number of matched DataReaders required for publishing
        uint32_t num_wait_matched_;

        // protects wait_matched condition variable
        static std::mutex wait_matched_cv_mtx_;

        // waits until enough DataReaders are matched
        static std::condition_variable wait_matched_cv_;
    }
    listener_;

    //!Run thread for number samples, publish every sleep seconds
    void runThread(
            uint32_t number,
            uint32_t sleep);

    // member used for control flow purposes
    static std::atomic<bool> stop_;
};



#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_HELLOWORLDPUBLISHER_H_ */

// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PublisherApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__PUBLISHERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__PUBLISHERAPP_HPP


#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace delivery_mechanisms {

class PublisherApp : public Application, public dds::DataWriterListener
{
public:

    PublisherApp(
            const CLIParser::delivery_mechanisms_config& config,
            const std::string& topic_name);

    ~PublisherApp();

    //! Publisher matched method
    void on_publication_matched(
            dds::DataWriter* writer,
            const dds::PublicationMatchedStatus& info) override;

    //! Run publisher
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    //! Publish a sample
    bool publish();

    dds::DomainParticipant* participant_;

    dds::Publisher* publisher_;

    dds::Topic* topic_;

    dds::DataWriter* writer_;

    dds::TypeSupport type_;

    std::condition_variable cv_;

    int32_t matched_;

    std::mutex mutex_;

    const uint32_t period_ms_ = 100; // in ms

    uint32_t index_of_last_sample_sent_;

    uint16_t samples_;

    std::atomic<bool> stop_;
};

} // namespace delivery_mechanisms
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__PUBLISHERAPP_HPP

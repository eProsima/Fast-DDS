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
 * @file PublisherApp.h
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_STATIC_EDP_DISCOVERY__PUBLISHERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_STATIC_EDP_DISCOVERY__PUBLISHERAPP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "HelloWorld.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace static_edp_discovery {

class PublisherApp : public Application, public DataWriterListener
{
public:

    PublisherApp(
            const CLIParser::static_edp_discovery_config& config,
            const std::string& topic_name);

    ~PublisherApp();

    //! Publisher matched method
    void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info) override;

    //! Run publisher
    void run() override;

    //! Stop publisher
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    //! Publish a sample
    bool publish();

    HelloWorld hello_;

    DomainParticipant* participant_;

    Publisher* publisher_;

    Topic* topic_;

    DataWriter* writer_;

    TypeSupport type_;

    int32_t matched_;

    uint16_t samples_;

    std::mutex mutex_;

    std::condition_variable cv_;

    std::atomic<bool> stop_;

    const uint32_t period_ms_ = 100; // in ms

    std::string xml_file_name;
};

} // namespace static_edp_discovery
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_STATIC_EDP_DISCOVERY__PUBLISHERAPP_HPP

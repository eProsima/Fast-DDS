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

#ifndef FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__PUBLISHERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__PUBLISHERAPP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "ShapeTypePubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace topic_instances {

class PublisherApp : public Application, public DataWriterListener
{
public:

    PublisherApp(
            const CLIParser::publisher_config& config);

    ~PublisherApp();

    //! Publisher matched method
    void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info) override;

    //! Run publisher
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    //! Publish a sample
    void publish();

    //! Check if all instances have sent the expected number of samples
    bool instances_sent_all_samples();

    //! Move the shape around the space limits
    void move(
            int& x,
            int& y,
            CLIParser::ShapeDirection& direction);

    DomainParticipant* participant_;

    Publisher* publisher_;

    Topic* topic_;

    DataWriter* writer_;

    TypeSupport type_;

    std::condition_variable cv_;

    int32_t matched_;

    std::mutex mutex_;

    uint32_t period_ms_;

    uint32_t timeout_s_;

    uint16_t samples_;

    uint16_t instances_;

    //! Shape location for movement
    struct shape_location
    {
        int x = 0;
        int y = 0;
        CLIParser::ShapeDirection direction = CLIParser::ShapeDirection::DOWN;
    };

    CLIParser::shape_configuration shape_config_;

    std::vector<InstanceHandle_t> instance_handles_;

    std::map<InstanceHandle_t, std::tuple<ShapeType, shape_location, uint32_t>> shapes_;

    std::atomic<bool> stop_;

};

} // namespace topic_instances
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__PUBLISHERAPP_HPP

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

#ifndef FASTDDS_EXAMPLES_CPP_CONFIGURATION__PUBLISHERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_CONFIGURATION__PUBLISHERAPP_HPP


#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "Configuration.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace configuration {

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

    //! Sample deadline missed method
    void on_offered_deadline_missed(
            DataWriter* writer,
            const OfferedDeadlineMissedStatus& status) override;

    //! Incompatible QoS method
    void on_offered_incompatible_qos(
            DataWriter* writer,
            const OfferedIncompatibleQosStatus& status) override;

    //! Liveliness lost method
    void on_liveliness_lost(
            DataWriter* writer,
            const LivelinessLostStatus& status) override;

    //! Un-acked sample removed method
    void on_unacknowledged_sample_removed(
            DataWriter* writer,
            const InstanceHandle_t& instance) override;

    //! Run publisher
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    //! Publish a sample
    bool publish();

    Configuration configuration_;

    DomainParticipant* participant_;

    Publisher* publisher_;

    Topic* topic_;

    DataWriter* writer_;

    TypeSupport type_;

    std::condition_variable cv_;

    int32_t matched_;

    std::mutex mutex_;

    uint32_t period_ms_;

    uint16_t samples_;

    std::atomic<bool> stop_;

    int16_t wait_;
};

} // namespace configuration
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_CONFIGURATION__PUBLISHERAPP_HPP

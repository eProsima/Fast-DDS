/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file PublisherApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__PUBLISHERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__PUBLISHERAPP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace low_bandwidth {

class PublisherApp : public Application, public DataWriterListener
{
public:

    PublisherApp(
            const CLIParser::publisher_config& config,
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

    DomainParticipant* participant_ {nullptr};

    Publisher* publisher_ {nullptr};

    Topic* topic_ {nullptr};

    DataWriter* writer_ {nullptr};

    TypeSupport type_ {new HelloWorldPubSubType()};

    int16_t matched_ {0};

    uint16_t samples_ {0};

    uint16_t expected_matches_ {0};

    std::mutex mutex_;

    std::condition_variable cv_;

    std::atomic<bool> stop_ {false};

    const uint32_t period_ms_ {100}; // in ms
};

} // namespace low_bandwidth
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__PUBLISHERAPP_HPP

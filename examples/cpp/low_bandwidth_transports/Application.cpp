/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file Application.cpp
 *
 */

#include "Application.hpp"

#include "CLIParser.hpp"
#include "SubscriberApp.hpp"
#include "PublisherApp.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace low_bandwidth {

//! Factory method to create a publisher or subscriber
std::shared_ptr<Application> Application::make_app(
        const CLIParser::low_bandwidth_config& config,
        const std::string& topic_name)
{
    std::shared_ptr<Application> entity;
    switch (config.entity)
    {
        case CLIParser::EntityKind::PUBLISHER:
            entity = std::make_shared<PublisherApp>(config.pub_config, topic_name);
            break;
        case CLIParser::EntityKind::SUBSCRIBER:
            entity = std::make_shared<SubscriberApp>(config.sub_config, topic_name);
            break;
        case CLIParser::EntityKind::UNDEFINED:
        default:
            throw std::runtime_error("Entity initialization failed");
            break;
    }
    return entity;
}

} // namespace low_bandwidth
} // namespace examples
} // namespace fastdds
} // namespace eprosima

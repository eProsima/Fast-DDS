/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file Application.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__APPLICATION_HPP
#define FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__APPLICATION_HPP

#include <atomic>

#include "CLIParser.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace low_bandwidth {

class Application
{
public:

    //! Virtual destructor
    virtual ~Application() = default;

    //! Run application
    virtual void run() = 0;

    //! Trigger the end of execution
    virtual void stop() = 0;

    //! Factory method to create applications based on configuration
    static std::shared_ptr<Application> make_app(
            const CLIParser::low_bandwidth_config& config,
            const std::string& topic_name);
};

} // namespace low_bandwidth
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__APPLICATION_HPP

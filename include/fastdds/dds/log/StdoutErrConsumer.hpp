// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_STDOUTERR_CONSUMER_HPP_
#define _FASTDDS_STDOUTERR_CONSUMER_HPP_

#include <fastdds/dds/log/Log.hpp>
#include <iostream>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Log consumer that writes the log events with a Log::Kind equal to or more severe than a threshold (defaults to
 * Log::Kind::Warning) to the STDERR, and events with a Log::Kind less severe than the threshold to the STDOUT.
 *
 * @file StdoutErrConsumer.hpp
 */
class StdoutErrConsumer : public LogConsumer
{
public:

    virtual ~StdoutErrConsumer() {}

    /** \internal
     * Called by Log to ask us to consume the Entry.
     * @param Log::Entry to consume.
     */
    RTPS_DllAPI virtual void Consume(
            const Log::Entry& entry);

    /**
     * @brief Set the stderr_threshold to a Log::Kind.
     * This threshold decides which log messages are output on STDOUT, and which are output to STDERR.
     * Log messages with a Log::Kind equal to or more severe than the stderr_threshold are output to STDERR using
     * std::cerr.
     * Log messages with a Log::Kind less severe than the stderr_threshold are output to STDOUT using
     * std::cout.
     * @param kind The Log::Kind to which stderr_threshold is set.
     */
    RTPS_DllAPI virtual void stderr_threshold(
            const Log::Kind& kind);

    /**
     * @brief Retrieve the stderr_threshold.
     * @return The Log::Kind to which stderr_threshold is set.
     */
    RTPS_DllAPI virtual Log::Kind stderr_threshold() const;

    /**
     * @brief Default value of stderr_threshold.
     */
    RTPS_DllAPI static const Log::Kind STDERR_THRESHOLD_DEFAULT = Log::Kind::Warning;

private:

    void print_header(
            const Log::Entry& entry) const;

    void print_context(
            const Log::Entry& entry) const;

    std::ostream* stream_;
    Log::Kind stderr_threshold_ = STDERR_THRESHOLD_DEFAULT;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif

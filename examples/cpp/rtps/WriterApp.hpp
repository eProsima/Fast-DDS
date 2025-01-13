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
 * @file WriterApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_RTPS__WRITERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_RTPS__WRITERAPP_HPP

#include <condition_variable>

#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include "CLIParser.hpp"
#include "Application.hpp"
#include "HelloWorld.hpp"
#include <fastcdr/FastCdr.h>

using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rtps {

class WriterApp : public Application, public WriterListener
{
public:

    WriterApp(
            const CLIParser::rtps_config& config,
            const std::string& topic_name);

    virtual ~WriterApp();

    //! Run RTPS Writer
    void run() override;

    //! Writer matched method
    void on_writer_matched(
            RTPSWriter*,
            const MatchingInfo& info) override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Add a new change to Writer History
    bool add_change_to_history();

    //! Return the current state of execution
    bool is_stopped();

    //! Serialize payload
    bool serialize_payload(
            const HelloWorld* data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload);

    uint16_t samples_;

    uint16_t samples_sent_;

    std::vector<GUID_t> remote_endpoints_guid;

    RTPSParticipant* rtps_participant_;

    RTPSWriter* rtps_writer_;

    WriterHistory* writer_history_;

    int16_t matched_;

    uint16_t expected_matches_;

    std::atomic<bool> stop_;

    mutable std::mutex terminate_cv_mtx_;

    std::condition_variable terminate_cv_;

    const uint32_t period_ms_ = 100; // in ms

    HelloWorld* data_;
};

} // namespace rtps
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RTPS__WRITERAPP_HPP

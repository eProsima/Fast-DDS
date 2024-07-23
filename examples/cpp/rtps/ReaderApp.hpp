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
 * @file ReaderApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_RTPS__READERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_RTPS__READERAPP_HPP

#include <condition_variable>
#include <string>

#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include "CLIParser.hpp"
#include "Application.hpp"
#include "HelloWorld.hpp"

using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rtps {

class ReaderApp : public Application, public ReaderListener
{
public:

    ReaderApp(
            const CLIParser::rtps_config& config,
            const std::string& topic_name);

    virtual ~ReaderApp();

    //! Run RTPS Reader
    void run() override;

    //! New CacheChange_t added to the history callback
    void on_new_cache_change_added(
            RTPSReader* reader,
            const CacheChange_t* const change) override;

    //! Reader matched method
    void on_reader_matched(
            RTPSReader*,
            const MatchingInfo& info) override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Method to deserialize the payload
    bool deserialize_payload(
            const SerializedPayload_t& payload,
            HelloWorld* data);

    //! Return the current state of execution
    bool is_stopped();

    uint16_t samples_;

    uint16_t samples_received_;

    RTPSParticipant* rtps_participant_;

    RTPSReader* rtps_reader_;

    ReaderHistory* reader_history_;

    std::atomic<bool> stop_;

    mutable std::mutex terminate_cv_mtx_;

    std::condition_variable terminate_cv_;

    HelloWorld* data_;
};

} // namespace rtps
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RTPS__READERAPP_HPP

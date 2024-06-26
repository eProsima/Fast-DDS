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
 * @file BaseWriter.cpp
 */

#include <rtps/writer/BaseWriter.hpp>

#include <chrono>
#include <memory>
#include <mutex>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <rtps/flowcontrol/FlowController.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

BaseWriter::BaseWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& att,
        FlowController* flow_controller,
        WriterHistory* hist,
        WriterListener* listen)
    : RTPSWriter(impl, guid, att, flow_controller, hist, listen)
{
    history_->mp_writer = this;
    history_->mp_mutex = &mp_mutex;

    flow_controller_->register_writer(this);

    EPROSIMA_LOG_INFO(RTPS_WRITER, "RTPSWriter created");
}

BaseWriter::~BaseWriter()
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "RTPSWriter destructor");

    // Deletion of the events has to be made in child destructor.
    // Also at this point all CacheChange_t must have been released by the child destructor

    history_->mp_writer = nullptr;
    history_->mp_mutex = nullptr;
}

void BaseWriter::deinit()
{
    // First, unregister changes from FlowController. This action must be protected.
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        for (auto it = history_->changesBegin(); it != history_->changesEnd(); ++it)
        {
            flow_controller_->remove_change(*it, std::chrono::steady_clock::now() + std::chrono::hours(24));
        }

        for (auto it = history_->changesBegin(); it != history_->changesEnd(); ++it)
        {
            history_->release_change(*it);
        }

        history_->m_changes.clear();
    }
    flow_controller_->unregister_writer(this);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

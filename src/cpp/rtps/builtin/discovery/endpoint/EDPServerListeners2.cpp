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

/**
 * @file EDPServerListener2.cpp
 *
 */

#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/history/WriterHistory.h>

#include <fastdds/dds/log/Log.hpp>

#include "./EDPServerListeners2.hpp"
#include "./EDPServer2.hpp"
#include "../participant/PDPServer2.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

EDPServerPUBListener2::EDPServerPUBListener2(
        EDPServer2* sedp)
    : EDPBasePUBListener(sedp->mp_RTPSParticipant->getAttributes().allocation.locators,
            sedp->mp_RTPSParticipant->getAttributes().allocation.data_limits)
    , sedp_(sedp)
{
}

void EDPServerPUBListener2::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    (void)reader;
    (void)change_in;
    // TODO DISCOVERY SERVER VERSION 2
}

void EDPServerPUBListener2::onWriterChangeReceivedByAll(
        RTPSWriter* writer,
        CacheChange_t* change)
{
    (void)writer;

    if (ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED == change->kind)
    {
        WriterHistory* writer_history =
                sedp_->publications_writer_.second;

        writer_history->remove_change(change);
    }
}

EDPServerSUBListener2::EDPServerSUBListener2(
        EDPServer2* sedp)
    : EDPBaseSUBListener(sedp->mp_RTPSParticipant->getAttributes().allocation.locators,
            sedp->mp_RTPSParticipant->getAttributes().allocation.data_limits)
    , sedp_(sedp)
{
}

void EDPServerSUBListener2::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    (void)reader;
    (void)change_in;
    // TODO DISCOVERY SERVER VERSION 2
}

void EDPServerSUBListener2::onWriterChangeReceivedByAll(
        RTPSWriter* writer,
        CacheChange_t* change)
{
    (void)writer;

    if (ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED == change->kind)
    {
        WriterHistory* writer_history =
                sedp_->subscriptions_writer_.second;

        writer_history->remove_change(change);
    }

}

} /* namespace rtps */
} // namespace fastdds
} /* namespace eprosima */

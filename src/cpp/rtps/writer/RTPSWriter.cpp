// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * @file RTPSWriter.cpp
 *
 */

#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <memory>
#include <mutex>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>

#include <rtps/DataSharing/DataSharingNotifier.hpp>
#include <rtps/DataSharing/WriterPool.hpp>
#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/messages/RTPSMessageCreator.hpp>
#include <rtps/messages/RTPSMessageGroup.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <statistics/rtps/StatisticsBase.hpp>
#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>
#include "../flowcontrol/FlowController.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

RTPSWriter::RTPSWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& att,
        fastdds::rtps::FlowController* flow_controller,
        WriterHistory* hist,
        WriterListener* listen)
    : Endpoint(impl, guid, att.endpoint)
    , flow_controller_(flow_controller)
    , history_(hist)
    , listener_(listen)
    , is_async_(att.mode == SYNCHRONOUS_WRITER ? false : true)
    , liveliness_kind_(att.liveliness_kind)
    , liveliness_lease_duration_(att.liveliness_lease_duration)
    , liveliness_announcement_period_(att.liveliness_announcement_period)
{
}

RTPSWriter::~RTPSWriter()
{
}

SequenceNumber_t RTPSWriter::get_seq_num_min()
{
    CacheChange_t* change;
    if (history_->get_min_change(&change) && change != nullptr)
    {
        return change->sequenceNumber;
    }
    else
    {
        return c_SequenceNumber_Unknown;
    }
}

SequenceNumber_t RTPSWriter::get_seq_num_max()
{
    CacheChange_t* change;
    if (history_->get_max_change(&change) && change != nullptr)
    {
        return change->sequenceNumber;
    }
    else
    {
        return c_SequenceNumber_Unknown;
    }
}

uint32_t RTPSWriter::getTypeMaxSerialized()
{
    return history_->getTypeMaxSerialized();
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

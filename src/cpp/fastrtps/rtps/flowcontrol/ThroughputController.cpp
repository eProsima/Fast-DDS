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

#include "ThroughputController.h"
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <asio.hpp>
#include <asio/steady_timer.hpp>
#include <cassert>


namespace eprosima{
namespace fastrtps{
namespace rtps{

ThroughputController::ThroughputController(const ThroughputControllerDescriptor& descriptor, const RTPSWriter* associatedWriter):
    mBytesPerPeriod(descriptor.bytesPerPeriod),
    mAccumulatedPayloadSize(0),
    mPeriodMillisecs(descriptor.periodMillisecs),
    mAssociatedParticipant(nullptr),
    mAssociatedWriter(associatedWriter)
{
}

ThroughputController::ThroughputController(const ThroughputControllerDescriptor& descriptor, const RTPSParticipantImpl* associatedParticipant):
    mBytesPerPeriod(descriptor.bytesPerPeriod),
    mAccumulatedPayloadSize(0),
    mPeriodMillisecs(descriptor.periodMillisecs),
    mAssociatedParticipant(associatedParticipant),
    mAssociatedWriter(nullptr)
{
}

void ThroughputController::operator()(RTPSWriterCollector<ReaderLocator*>& changesToSend)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mThroughputControllerMutex);

    auto it = changesToSend.items().begin();

    while(it != changesToSend.items().end())
    {
        if(!process_change_nts_(it->cacheChange, it->sequenceNumber, it->fragmentNumber))
            break;

        ++it;
    }

    changesToSend.items().erase(it, changesToSend.items().end());
}

void ThroughputController::operator()(RTPSWriterCollector<ReaderProxy*>& changesToSend)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mThroughputControllerMutex);

    auto it = changesToSend.items().begin();

    while(it != changesToSend.items().end())
    {
        if(!process_change_nts_(it->cacheChange, it->sequenceNumber, it->fragmentNumber))
            break;

        ++it;
    }

    changesToSend.items().erase(it, changesToSend.items().end());
}

bool ThroughputController::process_change_nts_(CacheChange_t* change, const SequenceNumber_t& /*seqNum*/,
        const FragmentNumber_t fragNum)
{
    assert(change != nullptr);

    uint32_t dataLength = change->serializedPayload.length;

    if (fragNum != 0)
        dataLength = (fragNum + 1) != change->getFragmentCount() ?
            change->getFragmentSize() : change->serializedPayload.length - (fragNum * change->getFragmentSize());

    if((mAccumulatedPayloadSize + dataLength) <= mBytesPerPeriod)
    {
        mAccumulatedPayloadSize += dataLength;
        ScheduleRefresh(dataLength);
        return true;
    }

    return false;
}

void ThroughputController::ScheduleRefresh(uint32_t sizeToRestore)
{
    std::shared_ptr<asio::steady_timer> throwawayTimer(std::make_shared<asio::steady_timer>(*FlowController::ControllerService));
    auto refresh = [throwawayTimer, this, sizeToRestore]
        (const asio::error_code& error)
        {
            if ((error != asio::error::operation_aborted) &&
                    FlowController::IsListening(this))
            {
                std::unique_lock<std::recursive_mutex> scopedLock(mThroughputControllerMutex);
                throwawayTimer->cancel();
                mAccumulatedPayloadSize = sizeToRestore > mAccumulatedPayloadSize ? 0 : mAccumulatedPayloadSize - sizeToRestore;

                if (mAssociatedWriter)
                    AsyncWriterThread::wakeUp(mAssociatedWriter);
                else if (mAssociatedParticipant)
                    AsyncWriterThread::wakeUp(mAssociatedParticipant);
            }
        };

    throwawayTimer->expires_from_now(std::chrono::milliseconds(mPeriodMillisecs));
    throwawayTimer->async_wait(refresh);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

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

#include <rtps/flowcontrol/ThroughputController.h>

#include <fastdds/rtps/resources/AsyncWriterThread.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <cassert>


namespace eprosima {
namespace fastrtps {
namespace rtps {

ThroughputController::ThroughputController(
        const ThroughputControllerDescriptor& descriptor,
        RTPSWriter* associatedWriter)
    : mBytesPerPeriod(descriptor.bytesPerPeriod)
    , mAccumulatedPayloadSize(0)
    , mPeriodMillisecs(descriptor.periodMillisecs)
    , mAssociatedParticipant(nullptr)
    , mAssociatedWriter(associatedWriter)
{
}

ThroughputController::ThroughputController(
        const ThroughputControllerDescriptor& descriptor,
        RTPSParticipantImpl* associatedParticipant)
    : mBytesPerPeriod(descriptor.bytesPerPeriod)
    , mAccumulatedPayloadSize(0)
    , mPeriodMillisecs(descriptor.periodMillisecs)
    , mAssociatedParticipant(associatedParticipant)
    , mAssociatedWriter(nullptr)
{
}

void ThroughputController::operator ()(
        RTPSWriterCollector<ReaderLocator*>& changesToSend)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mThroughputControllerMutex);
    process_nts(changesToSend);
}

void ThroughputController::operator ()(
        RTPSWriterCollector<ReaderProxy*>& changesToSend)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mThroughputControllerMutex);
    process_nts(changesToSend);
}

void ThroughputController::disable()
{
    std::unique_lock<std::recursive_mutex> scopedLock(mThroughputControllerMutex);
    mAssociatedWriter = nullptr;
    mAssociatedParticipant = nullptr;
    events_.clear();
}

template<typename Collector>
void ThroughputController::process_nts(
        Collector& changesToSend)
{
    uint32_t size_to_restore = 0;
    auto it = changesToSend.items().begin();
    while (
        it != changesToSend.items().end() &&
        process_change_nts_(it->cacheChange, it->sequenceNumber, it->fragmentNumber, &size_to_restore))
    {
        ++it;
    }

    changesToSend.items().erase(it, changesToSend.items().end());

    if (size_to_restore > 0)
    {
        ScheduleRefresh(size_to_restore);
    }
}

bool ThroughputController::process_change_nts_(
        CacheChange_t* change,
        const SequenceNumber_t& /*seqNum*/,
        const FragmentNumber_t fragNum,
        uint32_t* accumulated_size)
{
    assert(change != nullptr);

    uint32_t dataLength = change->serializedPayload.length;

    if (fragNum != 0)
    {
        dataLength = (fragNum + 1) != change->getFragmentCount() ?
                change->getFragmentSize() : change->serializedPayload.length - (fragNum * change->getFragmentSize());
    }

    if ((mAccumulatedPayloadSize + dataLength) <= mBytesPerPeriod)
    {
        mAccumulatedPayloadSize += dataLength;
        *accumulated_size += dataLength;
        return true;
    }

    return false;
}

void ThroughputController::ScheduleRefresh(
        uint32_t sizeToRestore)
{
    auto participant = mAssociatedWriter ? mAssociatedWriter->getRTPSParticipant() : mAssociatedParticipant;
    if (participant)
    {
        auto refresh = [this, sizeToRestore]()
        {
            if (FlowController::IsListening(this))
            {
                std::unique_lock<std::recursive_mutex> scopedLock(mThroughputControllerMutex);
                mAccumulatedPayloadSize = sizeToRestore > mAccumulatedPayloadSize ?
                    0 : mAccumulatedPayloadSize - sizeToRestore;

                if (mAssociatedWriter)
                {
                    mAssociatedWriter->getRTPSParticipant()->async_thread().wake_up(mAssociatedWriter);
                }
                else if (mAssociatedParticipant)
                {
                    std::unique_lock<std::recursive_mutex> lock(*mAssociatedParticipant->getParticipantMutex());
                    for (auto it = mAssociatedParticipant->userWritersListBegin();
                        it != mAssociatedParticipant->userWritersListEnd(); ++it)
                    {
                        mAssociatedParticipant->async_thread().wake_up(*it);
                    }
                }
                events_.pop_back();
            }

            return false;
        };

        events_.push_front(std::make_shared<TimedEvent>(participant->getEventResource(), refresh, mPeriodMillisecs));
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

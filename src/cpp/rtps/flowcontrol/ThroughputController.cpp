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

void ThroughputController::operator()(std::vector<CacheChange_t*>& changes)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mThroughputControllerMutex);

    uint32_t accumulatedPayloadSizeBeforeControllering = mAccumulatedPayloadSize;
    unsigned int clearedChanges = 0;
    while (clearedChanges < changes.size())
    {
        CacheChange_t* change = changes[clearedChanges];

        if (change->getFragmentSize() != 0)
        {
            ptrdiff_t fragments_to_send = std::count(change->getDataFragments()->begin(),
                    change->getDataFragments()->end(), PRESENT);
            unsigned int fittingFragments = std::min((mBytesPerPeriod - mAccumulatedPayloadSize) / change->getFragmentSize(),
                    static_cast<unsigned int>(fragments_to_send));

            if (fittingFragments)
            {
                mAccumulatedPayloadSize += fittingFragments * change->getFragmentSize();

                for(auto& aux_c : *change->getDataFragments())
                {
                    if(aux_c == PRESENT)
                    {
                        if(fittingFragments)
                            --fittingFragments;
                        else
                            aux_c = NOT_PRESENT;
                    }
                }

                clearedChanges++;
            }
            else
                break;
        }
        else
        {
            bool fits = (mAccumulatedPayloadSize + change->serializedPayload.length) <= mBytesPerPeriod;

            if (fits)
            {
                mAccumulatedPayloadSize += change->serializedPayload.length;
                clearedChanges++;
            }
            else
                break;
        }
    }


    if (mAccumulatedPayloadSize != accumulatedPayloadSizeBeforeControllering)
        ScheduleRefresh(mAccumulatedPayloadSize - accumulatedPayloadSizeBeforeControllering);
    changes.erase(changes.begin() + clearedChanges, changes.end());
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

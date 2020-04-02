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

#ifndef THROUGHPUT_CONTROLLER_H
#define THROUGHPUT_CONTROLLER_H

#include <rtps/flowcontrol/FlowController.h>
#include <fastdds/rtps/flowcontrol/ThroughputControllerDescriptor.h>

#include <thread>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSWriter;
class RTPSParticipantImpl;

/**
 * Simple filter that only clears changes up to a certain accumulated payload size.
 * It refreshes after a given time in MS, in a staggered way (e.g. if it clears
 * 500kb at t=0 and 800 kb at t=10, it will refresh 500kb at t = 0 + period, and
 * then fully refresh at t = 10 + period).
 */
class ThroughputController : public FlowController
{
public:

    ThroughputController(
            const ThroughputControllerDescriptor&,
            RTPSWriter* associatedWriter);
    ThroughputController(
            const ThroughputControllerDescriptor&,
            RTPSParticipantImpl* associatedParticipant);

    virtual void operator ()(
            RTPSWriterCollector<ReaderLocator*>& changesToSend) override;
    virtual void operator ()(
            RTPSWriterCollector<ReaderProxy*>& changesToSend) override;

    virtual void disable() override;

private:

    template<typename Collector>
    void process_nts(Collector& changesToSend);

    bool process_change_nts_(
            CacheChange_t* change,
            const SequenceNumber_t& seqNum,
            const FragmentNumber_t fragNum,
            uint32_t* accumulated_size);

    uint32_t mBytesPerPeriod;
    uint32_t mAccumulatedPayloadSize;
    uint32_t mPeriodMillisecs;
    std::recursive_mutex mThroughputControllerMutex;

    RTPSParticipantImpl* mAssociatedParticipant;
    RTPSWriter* mAssociatedWriter;

    /*
     * Schedules the filter to be refreshed in period ms. When it does, its capacity
     * will be partially restored, by "sizeToRestore" bytes.
     */
    void ScheduleRefresh(
            uint32_t sizeToRestore);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

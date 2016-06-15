#ifndef THROUGHPUT_CONTROLLER_H
#define THROUGHPUT_CONTROLLER_H

#include <fastrtps/rtps/flowcontrol/FlowController.h>
#include <thread>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * Descriptor for a Throughput Controller, containing all constructor information
 * for it. 
 *  -> sizeToClear: Packet size in bytes that this controller will allow in a given
 *     period.
 *  -> refreshTimeMS: Refresh period.
 * @ingroup NETWORK_MODULE
 */
struct ThroughputControllerDescriptor {
   uint32_t sizeToClear;
   uint32_t refreshTimeMS;
   RTPS_DllAPI ThroughputControllerDescriptor();
   RTPS_DllAPI ThroughputControllerDescriptor(uint32_t size, uint32_t time);
};

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
   ThroughputController(const ThroughputControllerDescriptor&, const RTPSWriter* associatedWriter);
   ThroughputController(const ThroughputControllerDescriptor&, const RTPSParticipantImpl* associatedParticipant);
   virtual void operator()(std::vector<CacheChangeForGroup_t>& changes);

private:
   uint32_t mSizeToClear;
   uint32_t mAccumulatedPayloadSize;
   uint32_t mRefreshTimeMS;
   std::recursive_mutex mThroughputControllerMutex;

   const RTPSParticipantImpl* mAssociatedParticipant;
   const RTPSWriter* mAssociatedWriter;

   /*
    * Schedules the filter to be refreshed in period ms. When it does, its capacity
    * will be partially restored, by "sizeToRestore" bytes.
    */
   void ScheduleRefresh(uint32_t sizeToRestore);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

#ifndef SIZE_FILTER_H
#define SIZE_FILTER_H

#include <fastrtps/rtps/filters/FlowFilter.h>
#include <thread>

namespace eprosima{
namespace fastrtps{
namespace rtps{

struct SizeFilterDescriptor {
   uint32_t sizeToClear;
   uint32_t refreshTimeMS;
};

/*
 * Simple filter that only clears changes up to a certain accumulated payload size.
 * It refreshes after a given time in MS, in a staggered way (e.g. if it clears
 * 500kb at t=0 and 800 kb at t=10, it will refresh 500kb at t = 0 + period, and
 * then fully refresh at t = 10 + period).
 */
class SizeFilter : public FlowFilter
{
public:
   SizeFilter(const SizeFilterDescriptor&);
   virtual void operator()(std::vector<CacheChangeForGroup_t>& changes);

private:
   uint32_t mSizeToClear;
   uint32_t mAccumulatedPayloadSize;
   uint32_t mRefreshTimeMS;
   std::recursive_mutex mSizeFilterMutex;

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

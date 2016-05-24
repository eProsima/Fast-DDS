#ifndef SIZE_FILTER_H
#define SIZE_FILTER_H

#include <fastrtps/rtps/filters/FlowFilter.h>
#include <thread>

/*
 * Simple filter that only clears changes up to a certain accumulated payload size.
 * It refreshes after a given time in MS.
 */
namespace eprosima{
namespace fastrtps{
namespace rtps{

class SizeFilter : public FlowFilter
{
public:
   SizeFilter(uint32_t sizeToClear, uint32_t refreshTimeMS);
   virtual void operator()(std::vector<CacheChangeForGroup_t>& changes);

private:
   uint32_t mSizeToClear;
   uint32_t mAccumulatedPayloadSize;
   uint32_t mRefreshTimeMS;
   std::recursive_mutex mSizeFilterMutex;

   void ScheduleRefresh(uint32_t sizeToOpen);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

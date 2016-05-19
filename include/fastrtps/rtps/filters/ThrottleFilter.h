#ifndef THROTTLE_FILTER_H
#define THROTTLE_FILTER_H

#include <fastrtps/rtps/filters/FlowFilter.h>
#include <thread>
#include <chrono>

/*
 * Lets all changes through. If any of the changes it last
 * allowed is sent, it will filter everything out for
 * a given period.
 */
namespace eprosima{
namespace fastrtps{
namespace rtps{

class ThrottleFilter : public FlowFilter
{
   public:
   explicit ThrottleFilter(unsigned int throttlePeriodInMS);
   virtual std::vector<const CacheChange_t*> operator()(std::vector<const CacheChange_t*>);

   private:
   virtual void NotifyChangeSent(const CacheChange_t*);
   void ThrottlePeriodCheck();
   unsigned int mThrottlePeriodInMs;
   bool mThrottling;
   std::recursive_mutex mMutex;
   std::vector<const CacheChange_t*> mLastClearedChanges;
   std::chrono::time_point<std::chrono::high_resolution_clock> mLastThrottleStartTime;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

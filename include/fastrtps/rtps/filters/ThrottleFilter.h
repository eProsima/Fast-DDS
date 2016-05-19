#ifndef THROTTLE_FILTER_H
#define THROTTLE_FILTER_H

#include <fastrtps/rtps/filters/FlowFilter.h>

/*
 * Lets changes through. If any of the changes it last
 * allowed is sent, it will filter everything out for
 * a period in milliseconds.
 */
namespace eprosima{
namespace fastrtps{
namespace rtps{

class ThrottleFilter : public FlowFilter
{
   public:
   explicit ThrottleFilter(unsigned int throttlePeriodInMS);
   ~ThrottleFilter();

   virtual std::vector<CacheChange_t*>       operator()(const std::vector<CacheChange_t*>);
   virtual std::vector<const CacheChange_t*> operator()(const std::vector<const CacheChange_t*>);

   private:
   ThrottleFilter(const ThrottleFilter&) = delete;
   const ThrottleFilter& operator=(const ThrottleFilter&) = delete;

   unsigned int m_lastThrottleTimeInMs;
   unsigned int m_throttlePeriodInMs;
   virtual void NotifyChangeSent(const CacheChange_t*);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

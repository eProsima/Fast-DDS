#include <fastrtps/rtps/filters/ThrottleFilter.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

ThrottleFilter::ThrottleFilter(unsigned int throttlePeriodInMS):
   m_lastThrottleTimeInMs(0),
   m_throttlePeriodInMs(throttlePeriodInMS)
{
   RegisterAsListeningFilter();
}

ThrottleFilter::~ThrottleFilter()
{}

std::vector<CacheChange_t*> ThrottleFilter::operator()(const std::vector<CacheChange_t*> changes)
{
   return changes;
}

std::vector<const CacheChange_t*> ThrottleFilter::operator()(const std::vector<const CacheChange_t*> changes)
{
   return changes;
}

void ThrottleFilter::NotifyChangeSent(const CacheChange_t* change)
{
   (void) change;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

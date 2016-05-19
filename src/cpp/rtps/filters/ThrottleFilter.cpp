#include <fastrtps/rtps/filters/ThrottleFilter.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

ThrottleFilter::ThrottleFilter(unsigned int throttlePeriodInMS):
   m_lastThrottleTimeInMs(0),
   m_throttlePeriodInMs(throttlePeriodInMS),
   m_throttling(false)
{
   RegisterAsListeningFilter();
}

vector<const CacheChange_t*> ThrottleFilter::operator()(vector<const CacheChange_t*> changes)
{
   unique_lock<recursive_mutex> scopedLock(m_mutex);
   if (m_throttling)
      changes.clear();

   m_lastClearedChanges = changes;
   return changes;
}

void ThrottleFilter::ThrottlePeriodCheck()
{
   if (!m_throttling)
      return;
}

void ThrottleFilter::SleepUntilEndOfThrottling()
{
   if (!m_throttling)
      return;
}

void ThrottleFilter::NotifyChangeSent(const CacheChange_t* change)
{
   unique_lock<recursive_mutex> scopedLock(m_mutex);

   // If the change was in our last cleared changes, we start throttling.
   if (find(m_lastClearedChanges.begin(), m_lastClearedChanges.end(), change) != m_lastClearedChanges.end())
      m_throttling = true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

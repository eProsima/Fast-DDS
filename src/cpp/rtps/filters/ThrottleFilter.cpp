#include <fastrtps/rtps/filters/ThrottleFilter.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

ThrottleFilter::ThrottleFilter(unsigned int throttlePeriodInMS):
   m_throttlePeriodInMs(throttlePeriodInMS),
   m_throttling(false)
{
   RegisterAsListeningFilter();
}

vector<const CacheChange_t*> ThrottleFilter::operator()(vector<const CacheChange_t*> changes)
{
   unique_lock<recursive_mutex> scopedLock(m_mutex);
   ThrottlePeriodCheck();
   if (m_throttling)
      changes.clear();

   m_lastClearedChanges = changes;
   return changes;
}

void ThrottleFilter::ThrottlePeriodCheck()
{
   unique_lock<recursive_mutex> scopedLock(m_mutex);
   if (!m_throttling)
      return;

   auto now = chrono::high_resolution_clock::now();
   auto period = chrono::duration_cast<chrono::milliseconds>(now - m_lastThrottleStartTime);

   if (period.count() > m_throttlePeriodInMs)
      m_throttling = false;
}

void ThrottleFilter::NotifyChangeSent(const CacheChange_t* change)
{
   unique_lock<recursive_mutex> scopedLock(m_mutex);

   // If the change was in our last cleared changes, we start throttling.
   if (find(m_lastClearedChanges.begin(), m_lastClearedChanges.end(), change) != m_lastClearedChanges.end())
   {
      m_throttling = true;
      m_lastThrottleStartTime = std::chrono::high_resolution_clock::now();
   }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

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
   unique_lock<recursive_mutex> scopedLock(mMutex);
   ThrottlePeriodCheck();
   if (mThrottling)
      changes.clear();

   mLastClearedChanges = changes;
   return changes;
}

void ThrottleFilter::ThrottlePeriodCheck()
{
   unique_lock<recursive_mutex> scopedLock(mMutex);
   if (!mThrottling)
      return;

   auto now = chrono::high_resolution_clock::now();
   auto period = chrono::duration_cast<chrono::milliseconds>(now - mLastThrottleStartTime);

   if (period.count() > mThrottlePeriodInMs)
      mThrottling = false;
}

void ThrottleFilter::NotifyChangeSent(const CacheChange_t* change)
{
   unique_lock<recursive_mutex> scopedLock(mMutex);

   // If the change was in our last cleared changes, we start throttling.
   if (find(mLastClearedChanges.begin(), mLastClearedChanges.end(), change) != mLastClearedChanges.end())
   {
      mThrottling = true;
      mLastThrottleStartTime = std::chrono::high_resolution_clock::now();
   }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#include <fastrtps/rtps/filters/ThrottleFilter.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

ThrottleFilter::ThrottleFilter(unsigned int throttlePeriodInMS):
   mThrottlePeriodInMs(throttlePeriodInMS),
   mThrottling(false)
{
   RegisterAsListeningFilter();
}

void ThrottleFilter::operator()(vector<CacheChangeForGroup_t>& changes)
{
   unique_lock<recursive_mutex> scopedLock(mMutex);
   ThrottlePeriodCheck();
   if (mThrottling)
      changes.clear();

   mLastClearedChanges.clear();
   for (auto& change : changes)
      mLastClearedChanges.push_back(&change);
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

void ThrottleFilter::NotifyChangeSent(const CacheChangeForGroup_t* change)
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

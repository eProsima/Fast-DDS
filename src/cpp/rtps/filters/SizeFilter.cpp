#include <fastrtps/rtps/filters/SizeFilter.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

SizeFilter::SizeFilter(uint32_t sizeToClear, uint32_t refreshTimeMS):
   mSizeToClear(sizeToClear),
   mAccumulatedPayloadSize(0),
   mRefreshTimeMS(refreshTimeMS),
   mRefreshTimer(FlowFilter::FilterService, boost::posix_time::milliseconds(mRefreshTimeMS))
{
}

SizeFilter::~SizeFilter()
{
   std::unique_lock<std::recursive_mutex> scopedLock(mSizeFilterMutex);
   mRefreshTimer.cancel();
}

void SizeFilter::operator()(vector<CacheChangeForGroup_t>& changes)
{
   std::unique_lock<std::recursive_mutex> scopedLock(mSizeFilterMutex);

   uint32_t accumulatedPayloadSizeBeforeFiltering = mAccumulatedPayloadSize;
   unsigned int clearedChanges = 0;
   while (clearedChanges < changes.size())
   {
      auto& change = changes[clearedChanges];
      if (change.isFragmented())
      {
         unsigned int fitting_fragments = min((mSizeToClear - mAccumulatedPayloadSize) / change.getChange()->getFragmentSize(),
                                              change.getChange()->getFragmentCount());
         if (fitting_fragments)
         {
            mAccumulatedPayloadSize += fitting_fragments * change.getChange()->getFragmentSize();
            change.setFragmentsClearedForSending(fitting_fragments);
            clearedChanges++;
         }
         else
            break;
      }
      else
      {
         bool fits = (mAccumulatedPayloadSize + change.getChange()->serializedPayload.length) <= mSizeToClear;

         if (fits)
         {
            mAccumulatedPayloadSize += change.getChange()->serializedPayload.length;
            clearedChanges++;
         }
         else
            break;
      }
   }

   if (mAccumulatedPayloadSize != accumulatedPayloadSizeBeforeFiltering)
      ScheduleRefresh();
   changes.erase(changes.begin() + clearedChanges, changes.end());
}

void SizeFilter::ScheduleRefresh()
{
   auto refresh = [&](const boost::system::error_code& error)
   {
      if ((error == boost::asio::error::operation_aborted))
         return;
      else
      {
         std::unique_lock<std::recursive_mutex> scopedLockB(mSizeFilterMutex);
         mAccumulatedPayloadSize = 0;
      //// TODO: Poke the async thread.
      }
   };

   mRefreshTimer.expires_from_now(boost::posix_time::milliseconds(mRefreshTimeMS));
   mRefreshTimer.async_wait(refresh);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#include <fastrtps/rtps/filters/SizeFilter.h>

using namespace std;
using namespace boost::asio;

namespace eprosima{
namespace fastrtps{
namespace rtps{

SizeFilter::SizeFilter(uint32_t sizeToClear, uint32_t refreshTimeMS):
   mSizeToClear(sizeToClear),
   mAccumulatedPayloadSize(0),
   mRefreshTimeMS(refreshTimeMS)
{
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
      ScheduleRefresh(mAccumulatedPayloadSize - accumulatedPayloadSizeBeforeFiltering);
   changes.erase(changes.begin() + clearedChanges, changes.end());
}

void SizeFilter::ScheduleRefresh(uint32_t sizeToOpen)
{
   shared_ptr<deadline_timer> throwawayTimer(make_shared<deadline_timer>(FlowFilter::FilterService));
   auto refresh = [throwawayTimer, this, sizeToOpen]
                   (const boost::system::error_code& error)
   {

      std::unique_lock<std::recursive_mutex> scopedLock(mSizeFilterMutex);
      if ((error != boost::asio::error::operation_aborted) &&
          FlowFilter::IsListening(this))
      {
         throwawayTimer->cancel();
         mAccumulatedPayloadSize = sizeToOpen > mAccumulatedPayloadSize ? 0 : mAccumulatedPayloadSize - sizeToOpen;
         // TODO: Poke the async thread.
      }
   };

   throwawayTimer->expires_from_now(boost::posix_time::milliseconds(mRefreshTimeMS));
   throwawayTimer->async_wait(refresh);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

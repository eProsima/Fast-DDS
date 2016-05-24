#include <fastrtps/rtps/filters/SizeFilter.h>

using namespace std;

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
         mAccumulatedPayloadSize += change.getChange()->serializedPayload.length;
         if (mAccumulatedPayloadSize <= mSizeToClear)
            clearedChanges++;
         else
            break;
      }
   }

   changes.erase(changes.begin() + clearedChanges, changes.end());
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

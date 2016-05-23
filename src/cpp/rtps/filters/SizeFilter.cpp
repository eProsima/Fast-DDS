#include <fastrtps/rtps/filters/SizeFilter.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

SizeFilter::SizeFilter(unsigned int sizeToClear):
   mSize(sizeToClear)
{
}

void SizeFilter::operator()(vector<CacheChangeForGroup_t>& changes)
{
   unsigned int accumulatedPayloadSize = 0;
   unsigned int clearedChanges = 0;
   while (clearedChanges < changes.size())
   {
      auto& change = changes[clearedChanges];
      if (change.isFragmented())
      {
         unsigned int fitting_fragments = min((mSize - accumulatedPayloadSize) / change.getChange()->getFragmentSize(),
                                              change.getChange()->getFragmentCount());
         if (fitting_fragments)
         {
            accumulatedPayloadSize += fitting_fragments * change.getChange()->getFragmentSize();
            change.setFragmentsClearedForSending(fitting_fragments);
            clearedChanges++;
         }
         else
            break;
      }
      else
      {
         accumulatedPayloadSize += change.getChange()->serializedPayload.length;
         if (accumulatedPayloadSize <= mSize)
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

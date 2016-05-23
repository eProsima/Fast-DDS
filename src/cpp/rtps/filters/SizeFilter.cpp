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
         // Todo partial clearing
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

   if (changes.size() <= mSize)
      return;
   changes.erase(changes.begin() + mSize, changes.end());
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

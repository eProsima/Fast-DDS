#include <fastrtps/rtps/filters/QuantityFilter.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

QuantityFilter::QuantityFilter(unsigned int quantityToClear):
   mQuantity(quantityToClear)
{
}

void QuantityFilter::operator()(vector<CacheChangeForGroup_t>& changes)
{
   if (changes.size() <= mQuantity)
      return;
   changes.erase(changes.begin() + mQuantity, changes.end());
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

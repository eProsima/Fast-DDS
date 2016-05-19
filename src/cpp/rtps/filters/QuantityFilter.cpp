#include <fastrtps/rtps/filters/QuantityFilter.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

QuantityFilter::QuantityFilter(unsigned int quantityToClear):
   mQuantity(quantityToClear);
{
}

vector<const CacheChange_t*> QuantityFilter::operator()(vector<const CacheChange_t*> changes)
{
   if (changes.size() > mQuantity)
      changes.resize(mQuantity);

   return changes;
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

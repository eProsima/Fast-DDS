#ifndef SIZE_FILTER_H
#define SIZE_FILTER_H

#include <fastrtps/rtps/filters/FlowFilter.h>
#include <thread>

/*
 * Simple filter that only clears changes up to a certain accumulated payload size.
 */
namespace eprosima{
namespace fastrtps{
namespace rtps{

class SizeFilter : public FlowFilter
{
   public:
   explicit SizeFilter(unsigned int sizeToClear);
   virtual void operator()(std::vector<CacheChangeForGroup_t>& changes);

   private:
   unsigned int mSize;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

#ifndef FLOW_FILTER_H
#define FLOW_FILTER_H

#include <vector>
#include <mutex>
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/rtps/messages/RTPSMessageGroup.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/*
* Flow Filters take a vector of cache changes (by reference) and return a filtered
* vector, with a collection of changes this filter considers valid for sending, 
* ordered by its subjective priority.
* */
class FlowFilter 
{
   public:
   // Called when a change is finally dispatched.
   static void NotifyFiltersChangeSent(const CacheChangeForGroup_t*);

   virtual ~FlowFilter();
   virtual void operator()(std::vector<CacheChangeForGroup_t>& changes) = 0;

   protected:
   // To be used by filters with interest in being called back when a change is sent.
   virtual void RegisterAsListeningFilter();

   private:
   static std::vector<FlowFilter*> ListeningFilters;
   static std::recursive_mutex ListeningFiltersMutex;

   virtual void NotifyChangeSent(const CacheChangeForGroup_t*){};
};



} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

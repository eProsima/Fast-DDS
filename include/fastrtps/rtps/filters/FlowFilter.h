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
* vector, with a collection of changes this filter considers valid for sending. Note
* that sending ANY ONE of these changes may invalidate the criteria and require a 
* new call to the filter.
*/
class FlowFilter 
{
   friend class RTPSMessageGroup;

   public:
   virtual ~FlowFilter(){};
   virtual std::vector<CacheChange_t*>       operator()(const std::vector<CacheChange_t*>) = 0;
   virtual std::vector<const CacheChange_t*> operator()(const std::vector<const CacheChange_t*>) = 0;


   protected:
   virtual void NotifyChangeSent(const CacheChange_t*){};

   virtual void RegisterAsListeningFilter();
   virtual void DeregisterAsListeningFilter();

   private:
   static std::vector<FlowFilter*> ListeningFilters;
   static std::recursive_mutex ListeningFiltersMutex;
   // To be called by the message group when a change is sent.
   static void NotifyFiltersChangeSent(const CacheChange_t*);
};



} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

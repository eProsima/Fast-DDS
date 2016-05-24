#ifndef FLOW_FILTER_H
#define FLOW_FILTER_H

#include <vector>
#include <mutex>
#include <functional>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
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

   // Filter operator
   virtual void operator()(std::vector<CacheChangeForGroup_t>& changes) = 0;

   virtual ~FlowFilter();
   FlowFilter();

private:
   virtual void NotifyChangeSent(const CacheChangeForGroup_t*){};
   void RegisterAsListeningFilter();
   void DeRegisterAsListeningFilter();

   static std::vector<FlowFilter*> ListeningFilters;
   static std::unique_ptr<boost::thread> FilterThread;
	static boost::asio::io_service FilterService;
   static std::recursive_mutex FlowFilterMutex;
   static void StartFilterService();

   // No copy, assignment or move! Filters are accessed by reference
   // from several places.
   // Ownership to be transferred via unique_ptr move semantics only.
   const FlowFilter& operator=(const FlowFilter&) = delete;
   FlowFilter(const FlowFilter&) = delete;
   FlowFilter(FlowFilter&&) = delete;

protected:
   // Schedules the filter to be updated in the future. Also
   // wakes up the asynchronous thread when it happens.
   void ScheduleCall(std::function<void()>, uint32_t ms);
};



} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

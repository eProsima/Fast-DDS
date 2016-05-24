#include <fastrtps/rtps/filters/FlowFilter.h>

std::vector<FlowFilter*> FlowFilter::ListeningFilters;
std::recursive_mutex FlowFilter::FlowFilterMutex;
std::unique_ptr<boost::thread> FlowFilter::FilterThread;
boost::asio::io_service FlowFilter::FilterService;

FlowFilter::FlowFilter()
{
   RegisterAsListeningFilter();
}

FlowFilter::~FlowFilter()
{
   DeRegisterAsListeningFilter();
}

void FlowFilter::NotifyFiltersChangeSent(const CacheChangeForGroup_t* change)
{
   std::unique_lock<std::recursive_mutex> scopedLock(FlowFilterMutex);
   for (auto filter : ListeningFilters)
      filter->NotifyChangeSent(change);
}

void FlowFilter::RegisterAsListeningFilter()
{
   std::unique_lock<std::recursive_mutex> scopedLock(FlowFilterMutex);
   ListeningFilters.push_back(this);

   if (!FilterThread)
      StartFilterService();
}

void FlowFilter::DeRegisterAsListeningFilter()
{
   std::unique_lock<std::recursive_mutex> scopedLock(FlowFilterMutex);
   ListeningFilters.erase(std::remove(ListeningFilters.begin(), ListeningFilters.end(), this), ListeningFilters.end());

   if (ListeningFilters.empty() && FilterThread)
   {
      // No listening filters, so there is no need for the filter thread.
      FilterService.stop();
      FilterThread->join();
      FilterThread.reset();
   }
}

void FlowFilter::StartFilterService()
{
   auto ioServiceFunction = [&]()
   {
      boost::asio::io_service::work work(FilterService);
      FilterService.run();
   };
   FilterThread.reset(new boost::thread(ioServiceFunction));
}

#include <fastrtps/rtps/filters/FlowFilter.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

std::vector<FlowFilter*> FlowFilter::ListeningFilters;
std::recursive_mutex FlowFilter::FlowFilterMutex;
std::unique_ptr<boost::thread> FlowFilter::FilterThread;
std::unique_ptr<boost::asio::io_service> FlowFilter::FilterService;

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
      StopFilterService();
}

bool FlowFilter::IsListening(FlowFilter* filter) 
{
   std::unique_lock<std::recursive_mutex> scopedLock(FlowFilterMutex);
   auto it = find(ListeningFilters.begin(), ListeningFilters.end(), filter);
   return it != ListeningFilters.end();
}

void FlowFilter::StartFilterService()
{
   auto ioServiceFunction = [&]()
   {
      FilterService.reset(new boost::asio::io_service);
      FilterService->reset();
      boost::asio::io_service::work work(*FilterService);
      FilterService->run();
   };
   FilterThread.reset(new boost::thread(ioServiceFunction));
}

void FlowFilter::StopFilterService()
{
   FilterService->stop();
   FilterThread->join();
   FilterThread.reset();
   FilterService.reset();
}

#include <fastrtps/rtps/filters/FlowFilter.h>

std::vector<FlowFilter*> FlowFilter::ListeningFilters;
std::recursive_mutex FlowFilter::ListeningFiltersMutex;

FlowFilter::~FlowFilter()
{
   std::unique_lock<std::recursive_mutex> scopedLock(ListeningFiltersMutex);
   ListeningFilters.erase(std::remove(ListeningFilters.begin(), ListeningFilters.end(), this), ListeningFilters.end());
}

void FlowFilter::NotifyFiltersChangeSent(const CacheChangeForGroup_t* change)
{
   std::unique_lock<std::recursive_mutex> scopedLock(ListeningFiltersMutex);
   for (auto filter : ListeningFilters)
      filter->NotifyChangeSent(change);
}

void FlowFilter::RegisterAsListeningFilter()
{
   std::unique_lock<std::recursive_mutex> scopedLock(ListeningFiltersMutex);
   ListeningFilters.push_back(this);
}

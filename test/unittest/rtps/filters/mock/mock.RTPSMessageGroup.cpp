#include <fastrtps/rtps/messages/RTPSMessageGroup.h>
#include <fastrtps/rtps/filters/FlowFilter.h>

uint32_t RTPSMessageGroup::send_Changes_AsData(RTPSMessageGroup_t* msg_group,
        RTPSWriter* W, std::vector<CacheChangeForGroup_t>& changes,
        const GuidPrefix_t& remoteGuidPrefix, const EntityId_t& ReaderId,
        LocatorList_t& unicast, LocatorList_t& multicast,
        bool expectsInlineQos)
{
   (void) msg_group;
   (void) W;
   (void) remoteGuidPrefix;
   (void) ReaderId;
   (void) unicast;
   (void) multicast;
   (void) expectsInlineQos;

   for (auto& change : changes)
      FlowFilter::NotifyFiltersChangeSent(change.getChange());

   return changes.size();
}

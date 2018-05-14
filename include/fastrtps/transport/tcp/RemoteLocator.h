#ifndef RTPS_TCP_REMOTE_LOCATOR_
#define RTPS_TCP_REMOTE_LOCATOR_

#include <fastrtps/rtps/common/Locator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RemoteLocator_t : public Locator_t
{
public:
    RemoteLocator_t(const Locator_t& loc) : Locator_t(loc)
    {
    }
};

inline bool operator<(const RemoteLocator_t &loc1, const RemoteLocator_t &loc2)
{
    if(loc1.kind < loc2.kind)
        return true;
    
    for(uint8_t i = 0; i < 16; ++i){
        if(loc1.address[i] < loc2.address[i])
            return true;
    }

    if(loc1.get_Logical_port() < loc2.get_Logical_port())
        return true;

    return false;
}

inline bool operator==(const RemoteLocator_t&loc1, const RemoteLocator_t& loc2)
{
    if(loc1.kind!=loc2.kind)
        return false;
    if(loc1.get_Logical_port() != loc2.get_Logical_port())
        return false;
    if(!std::equal(loc1.address,loc1.address+16,loc2.address))
        return false;
    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_REMOTE_LOCATOR_
#ifndef SENDER_RESOURCE_H
#define SENDER_RESOURCE_H

#include <functional>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class SenderResource 
{
public:
   bool Send();

private:
   std::function<void()> Cleanup();
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

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

   // Resources can only be transfered through move semantics. Copy and assignment are prohibited
   SenderResource(){};
   SenderResource(SenderResource&&){};
   SenderResource(const SenderResource&) = delete;
   SenderResource& operator=(const SenderResource&) = delete;

private:
   std::function<void()> Cleanup();
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

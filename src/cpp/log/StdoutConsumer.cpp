#include <fastrtps/log/StdoutConsumer.h>
#include <iostream>
#include <iomanip>

namespace eprosima {
namespace fastrtps {

void StdoutConsumer::Consume(const Log::Entry& entry)
{
   PrintHeader(entry);
   PrintMessage(std::cout, entry, true);
   PrintContext(entry);
   PrintNewLine(std::cout, true);
}

void StdoutConsumer::PrintHeader(const Log::Entry& entry) const
{
    PrintTimestamp(std::cout, entry, true);
    LogConsumer::PrintHeader(std::cout, entry, true);
}

void StdoutConsumer::PrintContext(const Log::Entry& entry) const
{
    LogConsumer::PrintContext(std::cout, entry, true);
}

} // Namespace fastrtps
} // Namespace eprosima

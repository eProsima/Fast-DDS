#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <iostream>
#include <iomanip>

namespace eprosima {
namespace fastdds {
namespace dds {

void StdoutConsumer::Consume(
        const Log::Entry& entry)
{
    print_header(entry);
    print_message(std::cout, entry, true);
    print_context(entry);
    print_new_line(std::cout, true);
}

void StdoutConsumer::print_header(
        const Log::Entry& entry) const
{
    print_timestamp(std::cout, entry, true);
    LogConsumer::print_header(std::cout, entry, true);
}

void StdoutConsumer::print_context(
        const Log::Entry& entry) const
{
    LogConsumer::print_context(std::cout, entry, true);
}

} // Namespace dds
} // Namespace fastdds
} // Namespace eprosima

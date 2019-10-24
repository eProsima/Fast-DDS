#include <fastdds/dds/log/FileConsumer.hpp>
#include <iomanip>

namespace eprosima {
namespace fastdds {
namespace dds {

FileConsumer::FileConsumer()
    : FileConsumer("output.log")
{
}

FileConsumer::FileConsumer(
        const std::string& filename,
        bool append)
    : output_file_(filename)
    , append_(append)
{
    if (append_)
    {
        file_.open(output_file_, std::ios::out | std::ios::app);
    }
    else
    {
        file_.open(output_file_, std::ios::out);
    }
}

FileConsumer::~FileConsumer()
{
    file_.close();
}

void FileConsumer::Consume(
        const Log::Entry& entry)
{
    print_header(entry);
    print_message(file_, entry, false);
    print_context(entry);
    print_new_line(file_, false);
    file_.flush();
}

void FileConsumer::print_header(
        const Log::Entry& entry)
{
    print_timestamp(file_, entry, false);
    LogConsumer::print_header(file_, entry, false);
}

void FileConsumer::print_context(
        const Log::Entry& entry)
{
    LogConsumer::print_context(file_, entry, false);
}

} // Namespace dds
} // Namespace fastdds
} // Namespace eprosima

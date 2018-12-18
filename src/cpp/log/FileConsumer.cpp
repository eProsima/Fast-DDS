#include <fastrtps/log/FileConsumer.h>
#include <iomanip>

namespace eprosima {
namespace fastrtps {

FileConsumer::FileConsumer()
    : FileConsumer("output.log")
{
}

FileConsumer::FileConsumer(const std::string &filename, bool append)
    : mOutputFile(filename)
    , mAppend(append)
{
    if (mAppend)
    {
        mFile.open(mOutputFile, std::ios::out | std::ios::app);
    }
    else
    {
        mFile.open(mOutputFile, std::ios::out);
    }
}

FileConsumer::~FileConsumer()
{
    mFile.close();
}

void FileConsumer::Consume(const Log::Entry& entry)
{
    PrintHeader(entry);
    PrintMessage(mFile, entry, false);
    PrintContext(entry);
    PrintNewLine(mFile, false);
    mFile.flush();
}

void FileConsumer::PrintHeader(const Log::Entry& entry)
{
    PrintTimestamp(mFile, entry, false);
    LogConsumer::PrintHeader(mFile, entry, false);
}

void FileConsumer::PrintContext(const Log::Entry& entry)
{
    LogConsumer::PrintContext(mFile, entry, false);
}

} // Namespace fastrtps
} // Namespace eprosima

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
    if (append)
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
   mFile << entry.message;
   PrintContext(entry);
   mFile << std::endl;
   mFile.flush();
}

void FileConsumer::PrintHeader(const Log::Entry& entry)
{
    PrintTimestamp(mFile);

    switch (entry.kind)
    {
    case Log::Kind::Error:
        mFile << "[" << entry.context.category << " Error] ";
        break;
    case Log::Kind::Warning:
        mFile << "[" << entry.context.category << " Warning] ";
        break;
    case Log::Kind::Info:
        mFile << "[" << entry.context.category << " Info] ";
        break;
    }
}

void FileConsumer::PrintContext(const Log::Entry& entry)
{
    if (entry.context.filename)
    {
      mFile << " (" << entry.context.filename;
      mFile << ":" << entry.context.line << ")";
    }
    if (entry.context.function)
    {
      mFile << " -> Function " << entry.context.function;
    }
}

} // Namespace fastrtps
} // Namespace eprosima

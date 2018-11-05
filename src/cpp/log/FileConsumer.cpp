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
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);
    auto ms = static_cast<unsigned>(tp / std::chrono::milliseconds(1));
    mFile << std::put_time(localtime(&now_c), "%F %T")
       << "." << std::setw(3) << std::setfill('0') <<  ms << " ";
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

#include <chrono>
#include <iomanip>
#include <mutex>

#include <fastrtps/log/Log.h>
#include <fastrtps/log/StdoutConsumer.h>
#include <fastrtps/log/Colors.h>
#include <iostream>

using namespace std;
namespace eprosima {
namespace fastrtps {

struct Log::Resources Log::mResources;

Log::Resources::Resources() : mLogging(false),
        mWork(false),
        mFilenames(false),
        mFunctions(true),
        mVerbosity(Log::Error)
{
    mResources.mConsumers.emplace_back(new StdoutConsumer);
}

Log::Resources::~Resources()
{
    Log::KillThread();
}

void Log::RegisterConsumer(std::unique_ptr<LogConsumer> &&consumer)
{
    std::unique_lock<std::mutex> guard(mResources.mConfigMutex);
    mResources.mConsumers.emplace_back(std::move(consumer));
}

void Log::ClearConsumers()
{
    std::unique_lock<std::mutex> working(mResources.mCvMutex);
    mResources.mCv.wait(working, [&]()
    {
        return mResources.mLogs.BothEmpty();
    });
    std::unique_lock<std::mutex> guard(mResources.mConfigMutex);
    mResources.mConsumers.clear();
}

void Log::Reset()
{
    std::unique_lock<std::mutex> configGuard(mResources.mConfigMutex);
    mResources.mCategoryFilter.reset();
    mResources.mFilenameFilter.reset();
    mResources.mErrorStringFilter.reset();
    mResources.mFilenames = false;
    mResources.mFunctions = true;
    mResources.mVerbosity = Log::Error;
    mResources.mConsumers.clear();
    mResources.mConsumers.emplace_back(new StdoutConsumer);
}

void Log::Run()
{
    std::unique_lock<std::mutex> guard(mResources.mCvMutex);
    while (mResources.mLogging)
    {
        while (mResources.mWork)
        {
            mResources.mWork = false;
            guard.unlock();
            {
                mResources.mLogs.Swap();
                while (!mResources.mLogs.Empty())
                {
                    std::unique_lock<std::mutex> configGuard(mResources.mConfigMutex);
                    if (Preprocess(mResources.mLogs.Front()))
                    {
                        for (auto &consumer : mResources.mConsumers)
                        {
                            consumer->Consume(mResources.mLogs.Front());
                        }
                    }

                    mResources.mLogs.Pop();
                }
            }
            guard.lock();
        }
        mResources.mCv.notify_one();
        if (mResources.mLogging)
            mResources.mCv.wait(guard);
    }
}

void Log::ReportFilenames(bool report)
{
    std::unique_lock<std::mutex> configGuard(mResources.mConfigMutex);
    mResources.mFilenames = report;
}

void Log::ReportFunctions(bool report)
{
    std::unique_lock<std::mutex> configGuard(mResources.mConfigMutex);
    mResources.mFunctions = report;
}

bool Log::Preprocess(Log::Entry &entry)
{
    if (mResources.mCategoryFilter && !regex_search(entry.context.category, *mResources.mCategoryFilter))
        return false;
    if (mResources.mFilenameFilter && !regex_search(entry.context.filename, *mResources.mFilenameFilter))
        return false;
    if (mResources.mErrorStringFilter && !regex_search(entry.message, *mResources.mErrorStringFilter))
        return false;
    if (!mResources.mFilenames)
        entry.context.filename = nullptr;
    if (!mResources.mFunctions)
        entry.context.function = nullptr;

    return true;
}

void Log::KillThread()
{
    {
        std::unique_lock<std::mutex> guard(mResources.mCvMutex);
        mResources.mLogging = false;
        mResources.mWork = false;
    }

    if (mResources.mLoggingThread)
    {
        mResources.mCv.notify_all();
        // The #ifdef workaround here is due to an unsolved MSVC bug, which Microsoft has announced
        // they have no intention of solving: https://connect.microsoft.com/VisualStudio/feedback/details/747145
        // Each VS version deals with post-main deallocation of threads in a very different way.
#if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
        mResources.mLoggingThread->join();
#endif
        mResources.mLoggingThread.reset();
    }
}

void Log::QueueLog(const std::string &message, const Log::Context &context, Log::Kind kind)
{
    {
        std::unique_lock<std::mutex> guard(mResources.mCvMutex);
        if (!mResources.mLogging && !mResources.mLoggingThread)
        {
            mResources.mLogging = true;
            mResources.mLoggingThread.reset(new thread(Log::Run));
        }
    }

    std::string timestamp;
    GetTimestamp(timestamp);
    mResources.mLogs.Push(Log::Entry{message, context, kind, timestamp});
    {
        std::unique_lock<std::mutex> guard(mResources.mCvMutex);
        mResources.mWork = true;
    }
    mResources.mCv.notify_all();
}

Log::Kind Log::GetVerbosity()
{
    return mResources.mVerbosity;
}

void Log::SetVerbosity(Log::Kind kind)
{
    std::unique_lock<std::mutex> configGuard(mResources.mConfigMutex);
    mResources.mVerbosity = kind;
}

void Log::SetCategoryFilter(const std::regex &filter)
{
    std::unique_lock<std::mutex> configGuard(mResources.mConfigMutex);
    mResources.mCategoryFilter.reset(new std::regex(filter));
}

void Log::SetFilenameFilter(const std::regex &filter)
{
    std::unique_lock<std::mutex> configGuard(mResources.mConfigMutex);
    mResources.mFilenameFilter.reset(new std::regex(filter));
}

void Log::SetErrorStringFilter(const std::regex &filter)
{
    std::unique_lock<std::mutex> configGuard(mResources.mConfigMutex);
    mResources.mErrorStringFilter.reset(new std::regex(filter));
}

void Log::GetTimestamp(std::string &timestamp)
{
    std::stringstream stream;
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);
    auto ms = static_cast<unsigned>(tp / std::chrono::milliseconds(1));

#if defined(_WIN32)
    struct tm timeinfo;
    localtime_s(&timeinfo, &now_c);
    stream << std::put_time(&timeinfo, "%F %T") << "." << std::setw(3) << std::setfill('0') << ms << " ";
//#elif defined(__clang__) && !defined(std::put_time) // TODO arm64 doesn't seem to support std::put_time
//    (void)now_c;
//    (void)ms;
#else
    stream << std::put_time(localtime(&now_c), "%F %T") << "." << std::setw(3) << std::setfill('0') << ms << " ";
#endif
    timestamp = stream.str();
}

void LogConsumer::PrintTimestamp(std::ostream &stream, const Log::Entry &entry, bool color) const
{
    std::string white = (color) ? C_B_WHITE : "";
    stream << white << entry.timestamp;
}

void LogConsumer::PrintHeader(std::ostream &stream, const Log::Entry &entry, bool color) const
{
    std::string c_b_color = (!color) ? "" :
        (entry.kind == Log::Kind::Error) ? C_B_RED :
        (entry.kind == Log::Kind::Warning) ? C_B_YELLOW :
        (entry.kind == Log::Kind::Info) ? C_B_GREEN : "";

    std::string white = (color) ? C_B_WHITE : "";

    std::string kind = (entry.kind == Log::Kind::Error) ? "Error" :
        (entry.kind == Log::Kind::Warning) ? "Warning" :
        (entry.kind == Log::Kind::Info) ? "Info" : "";

    stream << c_b_color << "[" << white << entry.context.category << c_b_color << " " << kind << "] ";
}

void LogConsumer::PrintContext(std::ostream &stream, const Log::Entry &entry, bool color) const
{
    if (color)
    {
        stream << C_B_BLUE;
    }
    if (entry.context.filename)
    {
        stream << " (" << entry.context.filename;
        stream << ":" << entry.context.line << ")";
    }
    if (entry.context.function)
    {
        stream << " -> Function ";
        if (color)
        {
            stream << C_CYAN;
        }
        stream << entry.context.function;
    }
}

void LogConsumer::PrintMessage(std::ostream &stream, const Log::Entry &entry, bool color) const
{
    std::string white = (color) ? C_WHITE : "";
    stream << white << entry.message;
}

void LogConsumer::PrintNewLine(std::ostream &stream, bool color) const
{
    std::string def = (color) ? C_DEF : "";
    stream << def << std::endl;
}

} //namespace fastrtps
} //namespace eprosima

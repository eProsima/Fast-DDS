#include "Log.h"
#include <iostream>


namespace eprosima {
namespace fastrtps {

struct Log::Resources Log::mResources;

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
    mResources.mConsumers.emplace_back(new LogConsumer);
}

void Log::RegisterConsumer(std::unique_ptr<LogConsumer> consumer)
{
    std::unique_lock<std::mutex> guard(mResources.mConfigMutex);
    mResources.mConsumers.emplace_back(std::move(consumer));
}

void Log::ClearConsumers()
{
    std::unique_lock<std::mutex> guard(mResources.mConfigMutex);
    mResources.mConsumers.clear();
}

} // namespace fastrtps
} // namespace eprosima
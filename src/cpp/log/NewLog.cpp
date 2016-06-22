#include <fastrtps/log/NewLog.h>

using namespace std;
namespace eprosima {
namespace fastrtps {

DBQueue<Log::Entry> Log::mLogs;
vector<unique_ptr<LogConsumer> > Log::mConsumers;
unique_ptr<thread> Log::mLoggingThread;
condition_variable Log::mCv;
mutex Log::mMutex;


void Log::RegisterConsumer(std::unique_ptr<LogConsumer> consumer) 
{
   std::unique_lock<std::mutex> guard(mMutex);
   mConsumers.emplace_back(std::move(consumer));
}

} //namespace fastrtps 
} //namespace eprosima 

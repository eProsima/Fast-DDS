#include <fastrtps/log/NewLog.h>

using namespace std;
namespace eprosima {
namespace fastrtps {

DBQueue<Log::Entry> Log::mLogs;
vector<unique_ptr<LogConsumer> > Log::mConsumers;
unique_ptr<thread> Log::mLoggingThread;
condition_variable Log::mCv;
mutex Log::mMutex;
bool Log::mLogging = false;
bool Log::mWork = false;

void Log::RegisterConsumer(std::unique_ptr<LogConsumer> consumer) 
{
   std::unique_lock<std::mutex> guard(mMutex);
   mConsumers.emplace_back(std::move(consumer));
}

void Log::Run() 
{
   std::unique_lock<std::mutex> guard(mMutex);
   while (mLogging) 
   {
      while (mWork) 
      {
         mWork = false;
         guard.unlock();

         mLogs.Swap();

         for (auto& consumer: mConsumers)
            consumer->Consume(mLogs.Front());
         mLogs.Pop();

         guard.lock();
      }

      mCv.wait(guard);
   }
}

void Log::StartLogging() 
{
   {
      std::unique_lock<std::mutex> guard(mMutex);
      mLogging = true;
   }
   if (!mLoggingThread) 
      mLoggingThread.reset(new thread(Log::Run));
}

void Log::StopLogging() 
{
   {
      std::unique_lock<std::mutex> guard(mMutex);
      mLogging = false;
   }
   mCv.notify_all();
   if (mLoggingThread)
      mLoggingThread->join();
}

void Log::QueueLog(const std::string& message, const Log::Context& context, Log::Kind kind) 
{
   mLogs.Push(Log::Entry{message, context, kind});
   {
      std::unique_lock<std::mutex> guard(mMutex);
      mWork = true;
   }
   mCv.notify_all();
}

} //namespace fastrtps 
} //namespace eprosima 

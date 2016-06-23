#include <fastrtps/log/Log.h>
#include <fastrtps/log/StdoutConsumer.h>
#include <iostream>

using namespace std;
namespace eprosima {
namespace fastrtps {

DBQueue<Log::Entry> Log::mLogs;
vector<unique_ptr<LogConsumer> > Log::mConsumers;
std::unique_ptr<LogConsumer> Log::mDefaultConsumer(new StdoutConsumer());
unique_ptr<thread> Log::mLoggingThread;
condition_variable Log::mCv;
mutex Log::mCvMutex;
mutex Log::mConfigMutex;

bool Log::mLogging = false;
bool Log::mWork = false;

bool Log::mFilenames(false);
bool Log::mFunctions(true);
regex Log::mRegexFilter("(.*?)");

void Log::RegisterConsumer(std::unique_ptr<LogConsumer> consumer) 
{
   std::unique_lock<std::mutex> guard(mConfigMutex);
   mConsumers.emplace_back(std::move(consumer));
}

void Log::Reset()
{
   StopLogging();
   ClearRegexFilter();
   ReportFilenames(false);
   ReportFunctions(true);

   std::unique_lock<std::mutex> configGuard(mConfigMutex);
   mConsumers.clear();

   std::unique_lock<std::mutex> guard(mCvMutex);
   mWork = false;
   mLogging = false;
   mLogs.Clear();
}

void Log::Run() 
{
   std::unique_lock<std::mutex> guard(mCvMutex);
   while (mLogging) 
   {
      while (mWork) 
      {
         mWork = false;
         guard.unlock();

         mLogs.Swap();
         while (!mLogs.Empty())
         {
            std::unique_lock<std::mutex> configGuard(mConfigMutex);
            if (Preprocess(mLogs.Front()))
            {
               for (auto& consumer: mConsumers)
                  consumer->Consume(mLogs.Front());

               mDefaultConsumer->Consume(mLogs.Front());
            }

            mLogs.Pop();
         }

         guard.lock();
      }

      mCv.wait(guard);
   }
}

void Log::ReportFilenames(bool report)
{
   std::unique_lock<std::mutex> configGuard(mConfigMutex);
   mFilenames = report;
}

void Log::ReportFunctions(bool report)
{
   std::unique_lock<std::mutex> configGuard(mConfigMutex);
   mFunctions = report;
}

bool Log::Preprocess(Log::Entry& entry)
{
   if (!mFilenames)
      entry.context.filename = nullptr;
   if (!mFunctions)
      entry.context.function = nullptr;
   if (!regex_search(entry.context.category, mRegexFilter))
      return false;

   return true;
}

void Log::StartLogging() 
{
   {
      std::unique_lock<std::mutex> guard(mCvMutex);
      mLogging = true;
   }
   if (!mLoggingThread) 
      mLoggingThread.reset(new thread(Log::Run));
}

void Log::StopLogging() 
{
   {
      std::unique_lock<std::mutex> guard(mCvMutex);
      mLogging = false;
   }
   if (mLoggingThread) 
   {
      mCv.notify_all();
      mLoggingThread->join();
      mLoggingThread.reset();
   }
}

void Log::QueueLog(const std::string& message, const Log::Context& context, Log::Kind kind) 
{
   {
      std::unique_lock<std::mutex> guard(mCvMutex);
      if (!mLogging) return;
   }

   mLogs.Push(Log::Entry{message, context, kind});
   {
      std::unique_lock<std::mutex> guard(mCvMutex);
      mWork = true;
   }
   mCv.notify_all();
}

void Log::SetRegexFilter(const std::regex& filter)
{
   std::unique_lock<std::mutex> configGuard(mConfigMutex);
   mRegexFilter = filter; 
}

void Log::ClearRegexFilter()
{
   std::unique_lock<std::mutex> configGuard(mConfigMutex);
   mRegexFilter = "(.*?)";
}

} //namespace fastrtps 
} //namespace eprosima 

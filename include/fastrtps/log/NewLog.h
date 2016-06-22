
// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef _FASTRTPS_LOG_LOG_H_
#define _FASTRTPS_LOG_LOG_H_

#include <fastrtps/utils/DBQueue.h>
#include <thread>

namespace eprosima {
namespace fastrtps {

class LogConsumer;

class Log 
{
public:
   enum class Kind {
      Info,
      Warning,
      Error,
   };

   struct Context {
      const char* filename;
      const char* line;
      const char* function;
   };

   static void QueueLog(std::string message, Log::Context, Log::Kind);
   static void RegisterConsumer(std::unique_ptr<LogConsumer>);

   struct Entry 
   {
      std::string message;
      Log::Context context;
   };

   static void Run();
private:
   static DBQueue<Entry> mLogs;
   static std::vector<std::unique_ptr<LogConsumer> > mConsumers;

   static std::unique_ptr<std::thread> mLoggingThread;
   static std::condition_variable mCv;
   static std::mutex mMutex;
};

/**
 * Consumes a log entry to output it somewhere.
 */
class LogConsumer {
public:
   virtual ~LogConsumer(){};
   virtual void Consume(const Log::Entry&) = 0;
};

} // namespace fastrtps
} // namespace eprosima

#endif

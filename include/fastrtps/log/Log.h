
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
#include <sstream>
#include <atomic>
#include <regex>

#define logError(cat, msg) {std::stringstream ss; ss << msg; Log::QueueLog(ss.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Error); }
#define logWarning(cat, msg) {std::stringstream ss; ss << msg; Log::QueueLog(ss.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Warning); }

#if (defined(__INTERNALDEBUG) || defined(_INTERNALDEBUG)) && (defined(_DEBUG) || defined(__DEBUG) )
   #define logInfo(cat, msg) {std::stringstream ss; ss << msg; Log::QueueLog(ss.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Info); }
#else
   #define logInfo(cat, msg)
#endif

namespace eprosima {
namespace fastrtps {

class LogConsumer;
class Log 
{
public:
   enum Kind {
      Error,
      Warning,
      Info,
   };

   static void RegisterConsumer(std::unique_ptr<LogConsumer>);
   static void ReportFilenames(bool);
   static void ReportFunctions(bool);
   static void SetVerbosity(Log::Kind);

   //! Sets a filter that will match against logging categories.
   static void SetRegexFilter(const std::regex& filter);
   static void ClearRegexFilter();

   //! Returns the logging engine to configuration defaults.
   static void Reset();

   struct Entry; struct Context; 

private:
   struct Resources 
   {
      DBQueue<Entry> mLogs;
      std::vector<std::unique_ptr<LogConsumer> > mConsumers;
      std::unique_ptr<LogConsumer> mDefaultConsumer;

      std::unique_ptr<std::thread> mLoggingThread;

      // Condition variable segment.
      std::condition_variable mCv;
      std::mutex mCvMutex;
      bool mLogging;
      bool mWork;

      // Context configuration.
      std::mutex mConfigMutex;
      bool mFilenames;
      bool mFunctions;
      std::regex mRegexFilter;
      Log::Kind mVerbosity;

      Resources();
      ~Resources();
   };

   static struct Resources mResources;

   // Applies transformations to the entries compliant with the options selected (such as
   // erasure of certain context information, or filtering by category. Returns false 
   // if the log entry is blacklisted.
   static bool Preprocess(Entry&);
   static void LaunchThread();
   static void KillThread();
   static void Run();

   // Public definitions for macro access, not direct user consumption
public:
   struct Context {
      const char* filename;
      int line;
      const char* function;
      const char* category;
   };

   struct Entry 
   {
      std::string message;
      Log::Context context;
      Log::Kind kind;
   };

   static void QueueLog(const std::string& message, const Log::Context&, Log::Kind);
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

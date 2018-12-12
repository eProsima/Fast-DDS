
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
#include <memory>
#include <regex>
#include <thread>
#include <atomic>

/**
 * eProsima log mock.
 */

// Logging API:

//! Logs an info message. Disable it through Log::SetVerbosity, #define LOG_NO_INFO, or being in a release branch
#define logInfo(cat,msg) logInfo_(cat,msg)
//! Logs a warning. Disable reporting through Log::SetVerbosity or #define LOG_NO_WARNING
#define logWarning(cat,msg) logWarning_(cat,msg)
//! Logs an error. Disable reporting through #define LOG_NO_ERROR
#define logError(cat,msg) logError_(cat,msg)

namespace eprosima {
namespace fastrtps {


class LogConsumer;

/**
 * Logging utilities.
 * Logging is accessed through the three macros above, and configuration on the log output
 * can be achieved through static methods on the class. Logging at various levels can be
 * disabled dynamically (through the Verbosity level) or statically (through the LOG_NO_[VERB]
 * macros) for maximum performance.
 * @ingroup COMMON_MODULE
 */
class Log
{
public:
   /**
    * Types of log entry.
    * * Error: Maximum priority. Can only be disabled statically through #define LOG_NO_ERROR.
    * * Warning: Medium priority.  Can be disabled statically and dynamically.
    * * Info: Low priority. Useful for debugging. Disabled by default on release branches.
    */
   enum Kind {
      Error,
      Warning,
      Info,
   };

   /**
    * Registers an user defined consumer to route log output. There is a default
    * stdout consumer active as default.
    */
   static void RegisterConsumer(std::unique_ptr<LogConsumer>);
   //! Removes all registered consumers, including the default stdout.
   static void ClearConsumers();
   //! Enables the reporting of filenames in log entries. Disabled by default.
   static void ReportFilenames(bool);
   //! Enables the reporting of function names in log entries. Enabled by default when supported.
   static void ReportFunctions(bool);
   //! Sets the verbosity level, allowing for messages equal or under that priority to be logged.
   static void SetVerbosity(Log::Kind kind);
   //! Returns the current verbosity level.
   static Log::Kind GetVerbosity();
   //! Sets a filter that will pattern-match against log categories, dropping any unmatched categories.
   static void SetCategoryFilter    (const std::regex&);
   //! Sets a filter that will pattern-match against filenames, dropping any unmatched categories.
   static void SetFilenameFilter    (const std::regex&);
   //! Sets a filter that will pattern-match against the provided error string, dropping any unmatched categories.
   static void SetErrorStringFilter (const std::regex&);
   //! Returns the logging engine to configuration defaults.
   static void Reset();
   //! Stops the logging thread. It will re-launch on the next call to a successful log macro.
   static void KillThread();
   // Note: In VS2013, if you're linking this class statically, you will have to call KillThread before leaving
   // main, due to an unsolved MSVC bug.

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
      std::string timestamp;
   };

   /**
    * Not recommended to call this method directly! Use the following macros:
    *  * logInfo(cat, msg);
    *  * logWarning(cat, msg);
    *  * logError(cat, msg);
    */
   static void QueueLog(const std::string&, const Log::Context&, Log::Kind);

   struct Resources
   {
      DBQueue<Entry> mLogs;
      std::vector<std::unique_ptr<LogConsumer> > mConsumers;

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
      std::unique_ptr<std::regex> mCategoryFilter;
      std::unique_ptr<std::regex> mFilenameFilter;
      std::unique_ptr<std::regex> mErrorStringFilter;

      std::atomic<Log::Kind> mVerbosity;

      Resources() {}
      ~Resources() {}
   };

   static struct Resources mResources;

   // Applies transformations to the entries compliant with the options selected (such as
   // erasure of certain context information, or filtering by category. Returns false
   // if the log entry is blacklisted.
   static bool Preprocess(Entry&) { return true; }
   static void LaunchThread() {}
   static void Run() {}
   static void GetTimestamp(std::string&) {}
};

/**
 * Consumes a log entry to output it somewhere.
 */
class LogConsumer {
public:
    virtual ~LogConsumer(){};
    virtual void Consume(const Log::Entry&) {}
protected:
    void PrintTimestamp(std::ostream&, const Log::Entry&, bool) const {}
    void PrintHeader(std::ostream&, const Log::Entry&, bool) const {};
    void PrintContext(std::ostream&, const Log::Entry&, bool) const {};
    void PrintMessage(std::ostream&, const Log::Entry&, bool) const {};
    void PrintNewLine(std::ostream&, bool) const {};
};

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

void Log::QueueLog(const std::string&, const Log::Context&, Log::Kind) {}

void Log::ClearConsumers()
{
    std::unique_lock<std::mutex> guard(mResources.mConfigMutex);
    mResources.mConsumers.clear();
}

void Log::ReportFilenames(bool) {}

void Log::ReportFunctions(bool) {}

void Log::SetVerbosity(Log::Kind kind) { mResources.mVerbosity = kind; }

Log::Kind Log::GetVerbosity() { return mResources.mVerbosity; }

void Log::SetCategoryFilter    (const std::regex&) {}

void Log::SetFilenameFilter    (const std::regex&) {}

void Log::SetErrorStringFilter (const std::regex&) {}

void Log::KillThread() {}

#if defined ( WIN32 )
   #define __func__ __FUNCTION__
#endif

#ifndef LOG_NO_ERROR
   #define logError_(cat, msg) {std::stringstream ss; ss << msg; Log::QueueLog(ss.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Error); }
#else
   #define logError_(cat, msg)
#endif

#ifndef LOG_NO_WARNING
   #define logWarning_(cat, msg) { if (Log::GetVerbosity() >= Log::Kind::Warning) { std::stringstream ss; ss << msg; Log::QueueLog(ss.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Warning); } }
#else
   #define logWarning_(cat, msg)
#endif

#if (defined(__INTERNALDEBUG) || defined(_INTERNALDEBUG)) && (defined(_DEBUG) || defined(__DEBUG)) && (!defined(LOG_NO_INFO))
   #define logInfo_(cat, msg) { if (Log::GetVerbosity() >= Log::Kind::Info) { std::stringstream ss; ss << msg; Log::QueueLog(ss.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Info); } }
#else
   #define logInfo_(cat, msg)
#endif

} // namespace fastrtps
} // namespace eprosima

#endif

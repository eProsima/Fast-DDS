// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#ifndef _FASTDDS_DDS_LOG_LOG_HPP_
#define _FASTDDS_DDS_LOG_LOG_HPP_

#include <regex>
#include <sstream>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastrtps/fastrtps_dll.h>

/**
 * eProsima log layer. Logging categories and verbosity can be specified dynamically at runtime.
 * However, even on a category not covered by the current verbosity level,
 * there is some overhead on calling a log macro. For maximum performance, you can
 * opt out of logging any particular level by defining the following symbols:
 *
 * * define LOG_NO_ERROR
 * * define LOG_NO_WARNING
 * * define LOG_NO_INFO
 *
 * Additionally. the lowest level (Info) is disabled by default on release branches.
 */

// Logging API:

// EPROSIMA LOG MACROS
//! Logs an info message. Disable it through Log::SetVerbosity, define LOG_NO_INFO, or being in a release branch
#define EPROSIMA_LOG_INFO(cat, msg) EPROSIMA_LOG_INFO_IMPL_(cat, msg)
//! Logs a warning. Disable reporting through Log::SetVerbosity or define LOG_NO_WARNING
#define EPROSIMA_LOG_WARNING(cat, msg) EPROSIMA_LOG_WARNING_IMPL_(cat, msg)
//! Logs an error. Disable reporting through define LOG_NO_ERROR
#define EPROSIMA_LOG_ERROR(cat, msg) EPROSIMA_LOG_ERROR_IMPL_(cat, msg)

#if ENABLE_OLD_LOG_MACROS_
// Compile old eProsima macros for compatibility shake.
// However, these macros will be deprecated in future releases, so please do not use them.

//! Logs an info message. Disable it through Log::SetVerbosity, define LOG_NO_INFO, or being in a release branch
#define logInfo(cat, msg) logInfo_(cat, msg)
//! Logs a warning. Disable reporting through Log::SetVerbosity or define LOG_NO_WARNING
#define logWarning(cat, msg) logWarning_(cat, msg)
//! Logs an error. Disable reporting through define LOG_NO_ERROR
#define logError(cat, msg) logError_(cat, msg)

//! Old internal macros. Just kept them in case some crazy bastard thoughtlessly used them
#define logInfo_(cat, msg) EPROSIMA_LOG_INFO_IMPL_(cat, msg);
#define logWarning_(cat, msg) EPROSIMA_LOG_WARNING_IMPL_(cat, msg);
#define logError_(cat, msg) EPROSIMA_LOG_ERROR_IMPL_(cat, msg);

#endif  // ENABLE_OLD_LOG_MACROS_

namespace eprosima {
namespace fastdds {
namespace dds {

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
     * * Error: Maximum priority. Can only be disabled statically through LOG_NO_ERROR.
     * * Warning: Medium priority.  Can be disabled statically and dynamically.
     * * Info: Low priority. Useful for debugging. Disabled by default on release branches.
     */
    enum Kind
    {
        Error,
        Warning,
        Info,
    };

    /**
     * Registers an user defined consumer to route log output.
     * There is a default stdout consumer active as default.
     * @param consumer r-value to a consumer unique_ptr. It will be invalidated after the call.
     */
    RTPS_DllAPI static void RegisterConsumer(
            std::unique_ptr<LogConsumer>&& consumer);

    //! Removes all registered consumers, including the default stdout.
    RTPS_DllAPI static void ClearConsumers();

    //! Enables the reporting of filenames in log entries. Disabled by default.
    RTPS_DllAPI static void ReportFilenames(
            bool);

    //! Enables the reporting of function names in log entries. Enabled by default when supported.
    RTPS_DllAPI static void ReportFunctions(
            bool);

    //! Sets the verbosity level, allowing for messages equal or under that priority to be logged.
    RTPS_DllAPI static void SetVerbosity(
            Log::Kind);

    //! Returns the current verbosity level.
    RTPS_DllAPI static Log::Kind GetVerbosity();

    //! Sets a filter that will pattern-match against log categories, dropping any unmatched categories.
    RTPS_DllAPI static void SetCategoryFilter(
            const std::regex&);

    //! Sets a filter that will pattern-match against filenames, dropping any unmatched categories.
    RTPS_DllAPI static void SetFilenameFilter(
            const std::regex&);

    //! Sets a filter that will pattern-match against the provided error string, dropping any unmatched categories.
    RTPS_DllAPI static void SetErrorStringFilter(
            const std::regex&);

    //! Sets thread configuration for the logging thread.
    RTPS_DllAPI static void SetThreadConfig(
            const rtps::ThreadSettings&);

    //! Returns the logging engine to configuration defaults.
    RTPS_DllAPI static void Reset();

    //! Waits until all info logged up to the call time is consumed
    RTPS_DllAPI static void Flush();

    //! Stops the logging thread. It will re-launch on the next call to a successful log macro.
    RTPS_DllAPI static void KillThread();

    // Note: In VS2013, if you're linking this class statically, you will have to call KillThread before leaving
    // main, due to an unsolved MSVC bug.

    struct Context
    {
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
     *  * EPROSIMA_LOG_INFO(cat, msg);
     *  * EPROSIMA_LOG_WARNING(cat, msg);
     *  * EPROSIMA_LOG_ERROR(cat, msg);
     *
     * @todo this method takes 2 mutexes (same mutex) internally.
     * This is a very high sensible point of the code and it should be refactored to be as efficient as possible.
     */
    RTPS_DllAPI static void QueueLog(
            const std::string& message,
            const Log::Context&,
            Log::Kind);
};

/**
 * Consumes a log entry to output it somewhere.
 */
class LogConsumer
{
public:

    virtual ~LogConsumer() = default;

    virtual void Consume(
            const Log::Entry&) = 0;

protected:

    RTPS_DllAPI void print_timestamp(
            std::ostream& stream,
            const Log::Entry&,
            bool color) const;

    RTPS_DllAPI void print_header(
            std::ostream& stream,
            const Log::Entry&,
            bool color) const;

    RTPS_DllAPI void print_context(
            std::ostream& stream,
            const Log::Entry&,
            bool color) const;

    RTPS_DllAPI void print_message(
            std::ostream& stream,
            const Log::Entry&,
            bool color) const;

    RTPS_DllAPI void print_new_line(
            std::ostream& stream,
            bool color) const;
};

#if defined(WIN32)
#define __func__ __FUNCTION__
#endif // if defined(WIN32)

/********************
* Implementation of the log macros depending on the defined macros:
* HAVE_LOG_NO_<level> disable completly a verbosity level
* _INTERNALDEBUG || __INTERNALDEBUG  force to compile the log macro call even when it would not be added to queue
* EPROSIMA_LOG_INFO_IMPL_ would only be compiled if HAVE_LOG_NO_INFO is OFF and
* - FASTDDS_ENFORCE_LOG_INFO or (DEBUG and INTERNALDEBUG) are defined
*
* There are 3 implementations for each level:
* 1. Compile and add log to queue
* 2. Compile but do not add it to queue (with INTERNALDEBUG)
* 3. Do not compile
*
* Every macro (with implementation) occurs inside a code block so after call every internal variable is destroyed.
* Every macro declared has a do while(0).
* This will not generate an assembler instruction and forces the user of the macro to use ";" after calling it.
* https://gcc.gnu.org/onlinedocs/cpp/Swallowing-the-Semicolon.html
* NOTE: some compilation cases do not use do while loop and so they do not force ";".
* It is a risk that a user takes in exchange of a perfect way of non generating code in such cases.
********************/

/*********
* ERROR *
*********/
// Name of variables inside macros must be unique, or it could produce an error with external variables
#if !HAVE_LOG_NO_ERROR

#define EPROSIMA_LOG_ERROR_IMPL_(cat, msg)                                                                                 \
    do {                                                                                                               \
        using namespace eprosima::fastdds::dds;                                                                        \
        std::stringstream fastdds_log_ss_tmp__;                                                                        \
        fastdds_log_ss_tmp__ << msg;                                                                                   \
        Log::QueueLog(fastdds_log_ss_tmp__.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Error); \
    } while (0)

#elif (__INTERNALDEBUG || _INTERNALDEBUG)

#define EPROSIMA_LOG_ERROR_IMPL_(cat, msg)                          \
    do {                                                        \
        auto fastdds_log_lambda_tmp__ = [&]()                   \
                {                                               \
                    std::stringstream fastdds_log_ss_tmp__;     \
                    fastdds_log_ss_tmp__ << msg;                \
                };                                              \
        (void)fastdds_log_lambda_tmp__;                         \
    } while (0)
#else

#define EPROSIMA_LOG_ERROR_IMPL_(cat, msg)

#endif // ifndef LOG_NO_ERROR

/***********
* WARNING *
***********/
#if !HAVE_LOG_NO_WARNING

#define EPROSIMA_LOG_WARNING_IMPL_(cat, msg)                                                                            \
    do {                                                                                                            \
        using namespace eprosima::fastdds::dds;                                                                     \
        if (Log::GetVerbosity() >= Log::Kind::Warning)                                                              \
        {                                                                                                           \
            std::stringstream fastdds_log_ss_tmp__;                                                                 \
            fastdds_log_ss_tmp__ << msg;                                                                            \
            Log::QueueLog(                                                                                          \
                fastdds_log_ss_tmp__.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Warning);  \
        }                                                                                                           \
    } while (0)

#elif (__INTERNALDEBUG || _INTERNALDEBUG)

#define EPROSIMA_LOG_WARNING_IMPL_(cat, msg)                        \
    do {                                                        \
        auto fastdds_log_lambda_tmp__ = [&]()                   \
                {                                               \
                    std::stringstream fastdds_log_ss_tmp__;     \
                    fastdds_log_ss_tmp__ << msg;                \
                };                                              \
        (void)fastdds_log_lambda_tmp__;                         \
    } while (0)

#else

#define EPROSIMA_LOG_WARNING_IMPL_(cat, msg)

#endif // ifndef LOG_NO_WARNING

/********
* INFO *
********/
// Allow multiconfig platforms like windows to disable info queueing on Release and other non-debug configs
#if !HAVE_LOG_NO_INFO &&  \
    (defined(FASTDDS_ENFORCE_LOG_INFO) || \
    ((defined(__INTERNALDEBUG) || defined(_INTERNALDEBUG)) && (defined(_DEBUG) || defined(__DEBUG) || \
    !defined(NDEBUG))))

#define EPROSIMA_LOG_INFO_IMPL_(cat, msg)                                                                   \
    do {                                                                                                \
        using namespace eprosima::fastdds::dds;                                                         \
        if (Log::GetVerbosity() >= Log::Kind::Info)                                                     \
        {                                                                                               \
            std::stringstream fastdds_log_ss_tmp__;                                                     \
            fastdds_log_ss_tmp__ << msg;                                                                \
            Log::QueueLog(fastdds_log_ss_tmp__.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, \
                    Log::Kind::Info);                                                                   \
        }                                                                                               \
    } while (0)

#elif (__INTERNALDEBUG || _INTERNALDEBUG)

#define EPROSIMA_LOG_INFO_IMPL_(cat, msg)                       \
    do {                                                    \
        auto fastdds_log_lambda_tmp__ = [&]()               \
                {                                           \
                    std::stringstream fastdds_log_ss_tmp__; \
                    fastdds_log_ss_tmp__ << msg;            \
                };                                          \
        (void)fastdds_log_lambda_tmp__;                     \
    } while (0)

#else

#define EPROSIMA_LOG_INFO_IMPL_(cat, msg)

#endif // ifndef LOG_NO_INFO


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // ifndef _FASTDDS_DDS_LOG_LOG_HPP_

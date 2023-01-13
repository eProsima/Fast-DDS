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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>

#include <fastrtps/utils/DBQueue.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <utils/SystemInfo.hpp>

using namespace std;
namespace eprosima {
namespace fastdds {
namespace dds {

struct LogResources
{
    fastrtps::DBQueue<Log::Entry> logs;
    std::vector<std::unique_ptr<LogConsumer>> consumers;
    std::unique_ptr<std::thread> logging_thread;

    // Condition variable segment.
    std::condition_variable cv;
    std::mutex cv_mutex;
    bool logging;
    bool work;
    int current_loop;

    // Context configuration.
    std::mutex config_mutex;
    bool filenames;
    bool functions;
    std::unique_ptr<std::regex> category_filter;
    std::unique_ptr<std::regex> filename_filter;
    std::unique_ptr<std::regex> error_string_filter;

    std::atomic<Log::Kind> verbosity;

    LogResources()
        : logging(false)
        , work(false)
        , current_loop(0)
        , filenames(false)
        , functions(true)
        , verbosity(Log::Error)
    {
#if STDOUTERR_LOG_CONSUMER
        consumers.emplace_back(new StdoutErrConsumer);
#else
        consumers.emplace_back(new StdoutConsumer);
#endif // STDOUTERR_LOG_CONSUMER
    }

    ~LogResources()
    {
        Flush();
        KillThread();
    }

    /**
     * Registers an user defined consumer to route log output.
     * There is a default stdout consumer active as default.
     * @param consumer r-value to a consumer unique_ptr. It will be invalidated after the call.
     */
    void RegisterConsumer(
            std::unique_ptr<LogConsumer>&& consumer)
    {
        std::unique_lock<std::mutex> guard(config_mutex);
        consumers.emplace_back(std::move(consumer));
    }

    //! Removes all registered consumers, including the default stdout.
    void ClearConsumers()
    {
        Flush();

        std::lock_guard<std::mutex> guard(config_mutex);
        consumers.clear();
    }

    //! Enables the reporting of filenames in log entries. Disabled by default.
    void ReportFilenames(
            bool report)
    {
        std::lock_guard<std::mutex> configGuard(config_mutex);
        filenames = report;
    }

    //! Enables the reporting of function names in log entries. Enabled by default when supported.
    void ReportFunctions(
            bool report)
    {
        std::lock_guard<std::mutex> configGuard(config_mutex);
        functions = report;
    }

    //! Sets the verbosity level, allowing for messages equal or under that priority to be logged.
    void SetVerbosity(
            Log::Kind kind)
    {
        verbosity = kind;
    }

    //! Returns the current verbosity level.
    Log::Kind GetVerbosity()
    {
        return verbosity;
    }

    //! Sets a filter that will pattern-match against log categories, dropping any unmatched categories.
    void SetCategoryFilter(
            const std::regex& filter)
    {
        std::unique_lock<std::mutex> configGuard(config_mutex);
        category_filter.reset(new std::regex(filter));
    }

    //! Sets a filter that will pattern-match against filenames, dropping any unmatched categories.
    void SetFilenameFilter(
            const std::regex& filter)
    {
        std::unique_lock<std::mutex> configGuard(config_mutex);
        filename_filter.reset(new std::regex(filter));
    }

    //! Sets a filter that will pattern-match against the provided error string, dropping any unmatched categories.
    void SetErrorStringFilter(
            const std::regex& filter)
    {
        std::unique_lock<std::mutex> configGuard(config_mutex);
        error_string_filter.reset(new std::regex(filter));
    }

    //! Returns the logging engine to configuration defaults.
    void Reset()
    {
        std::unique_lock<std::mutex> configGuard(config_mutex);
        category_filter.reset();
        filename_filter.reset();
        error_string_filter.reset();
        filenames = false;
        functions = true;
        verbosity = Log::Error;
        consumers.clear();

#if STDOUTERR_LOG_CONSUMER
        consumers.emplace_back(new StdoutErrConsumer);
#else
        consumers.emplace_back(new StdoutConsumer);
#endif // if STDOUTERR_LOG_CONSUMER
    }

    //! Waits until all info logged up to the call time is consumed
    void Flush()
    {
        std::unique_lock<std::mutex> guard(cv_mutex);

        if (!logging && !logging_thread)
        {
            // already killed
            return;
        }

        /*   Flush() two steps strategy:

             I must assure Log::Run swaps the queues because only swapping the queues the background content
             will be consumed (first Run() loop).

             Then, I must assure the new front queue content is consumed (second Run() loop).
         */

        int last_loop = -1;

        for (int i = 0; i < 2; ++i)
        {
            cv.wait(guard,
                    [&]()
                    {
                        /* I must avoid:
                         + the two calls be processed without an intermediate Run() loop (by using last_loop sequence number)
                         + deadlock by absence of Run() loop activity (by using BothEmpty() call)
                         */
                        return !logging ||
                        (logs.Empty() &&
                        (last_loop != current_loop || logs.BothEmpty()));
                    });

            last_loop = current_loop;

        }
    }

    /**
     * Not recommended to call this method directly! Use the following macros:
     *  * EPROSIMA_LOG_INFO(cat, msg);
     *  * EPROSIMA_LOG_WARNING(cat, msg);
     *  * EPROSIMA_LOG_ERROR(cat, msg);
     *
     * @todo this method takes 2 mutexes (same mutex) internally.
     * This is a very high sensible point of the code and it should be refactored to be as efficient as possible.
     */
    void QueueLog(
            const std::string& message,
            const Log::Context& context,
            Log::Kind kind)
    {
        StartThread();

        std::string timestamp = SystemInfo::get_timestamp();
        logs.Push(Log::Entry{ message, context, kind, timestamp });
        {
            std::unique_lock<std::mutex> guard(cv_mutex);
            work = true;
        }
        cv.notify_all();
    }

    //! Stops the logging thread. It will re-launch on the next call to QueueLog.
    void KillThread()
    {
        {
            std::unique_lock<std::mutex> guard(cv_mutex);
            logging = false;
            work = false;
        }

        if (logging_thread)
        {
            cv.notify_all();
            // The #ifdef workaround here is due to an unsolved MSVC bug, which Microsoft has announced
            // they have no intention of solving: https://connect.microsoft.com/VisualStudio/feedback/details/747145
            // Each VS version deals with post-main deallocation of threads in a very different way.
#if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
            if (logging_thread->joinable() && logging_thread->get_id() != std::this_thread::get_id())
            {
                logging_thread->join();
            }
#endif // if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
            logging_thread.reset();
        }
    }

private:

    void StartThread()
    {
        std::unique_lock<std::mutex> guard(cv_mutex);
        if (!logging && !logging_thread)
        {
            logging = true;
            logging_thread.reset(new thread(&LogResources::run, this));
        }
    }

    void run()
    {
        std::unique_lock<std::mutex> guard(cv_mutex);

        while (logging)
        {
            cv.wait(guard,
                    [&]()
                    {
                        return !logging || work;
                    });

            work = false;

            guard.unlock();
            {
                logs.Swap();
                while (!logs.Empty())
                {
                    std::unique_lock<std::mutex> configGuard(config_mutex);

                    Log::Entry& entry = logs.Front();
                    if (preprocess(entry))
                    {
                        for (auto& consumer : consumers)
                        {
                            consumer->Consume(entry);
                        }
                    }
                    // This Pop() is also a barrier for Log::Flush wait condition
                    logs.Pop();
                }
            }
            guard.lock();

            // avoid overflow
            if (++current_loop > 10000)
            {
                current_loop = 0;
            }

            cv.notify_all();
        }
    }

    bool preprocess(
            Log::Entry& entry)
    {
        if (category_filter && !regex_search(entry.context.category, *category_filter))
        {
            return false;
        }
        if (filename_filter && !regex_search(entry.context.filename, *filename_filter))
        {
            return false;
        }
        if (error_string_filter && !regex_search(entry.message, *error_string_filter))
        {
            return false;
        }
        if (!filenames)
        {
            entry.context.filename = nullptr;
        }
        if (!functions)
        {
            entry.context.function = nullptr;
        }

        return true;
    }

};

std::shared_ptr<LogResources> get_log_resources()
{
    static std::shared_ptr<LogResources> instance = std::make_shared<LogResources>();
    return instance;
}

void Log::RegisterConsumer(
        std::unique_ptr<LogConsumer>&& consumer)
{
    get_log_resources()->RegisterConsumer(std::move(consumer));
}

void Log::ClearConsumers()
{
    get_log_resources()->ClearConsumers();
}

void Log::Reset()
{
    get_log_resources()->Reset();
}

void Log::Flush()
{
    get_log_resources()->Flush();
}

void Log::ReportFilenames(
        bool report)
{
    get_log_resources()->ReportFilenames(report);
}

void Log::ReportFunctions(
        bool report)
{
    get_log_resources()->ReportFunctions(report);
}

void Log::KillThread()
{
    get_log_resources()->KillThread();
}

void Log::QueueLog(
        const std::string& message,
        const Log::Context& context,
        Log::Kind kind)
{
    get_log_resources()->QueueLog(message, context, kind);
}

Log::Kind Log::GetVerbosity()
{
    return get_log_resources()->GetVerbosity();
}

void Log::SetVerbosity(
        Log::Kind kind)
{
    get_log_resources()->SetVerbosity(kind);
}

void Log::SetCategoryFilter(
        const std::regex& filter)
{
    get_log_resources()->SetCategoryFilter(filter);
}

void Log::SetFilenameFilter(
        const std::regex& filter)
{
    get_log_resources()->SetFilenameFilter(filter);
}

void Log::SetErrorStringFilter(
        const std::regex& filter)
{
    get_log_resources()->SetErrorStringFilter(filter);
}

void LogConsumer::print_timestamp(
        std::ostream& stream,
        const Log::Entry& entry,
        bool color) const
{
    std::string white = (color) ? C_B_WHITE : "";
    stream << white << entry.timestamp << " ";
}

void LogConsumer::print_header(
        std::ostream& stream,
        const Log::Entry& entry,
        bool color) const
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

void LogConsumer::print_context(
        std::ostream& stream,
        const Log::Entry& entry,
        bool color) const
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

void LogConsumer::print_message(
        std::ostream& stream,
        const Log::Entry& entry,
        bool color) const
{
    std::string white = (color) ? C_WHITE : "";
    stream << white << entry.message;
}

void LogConsumer::print_new_line(
        std::ostream& stream,
        bool color) const
{
    std::string def = (color) ? C_DEF : "";
    stream << def << std::endl;
}

} //namespace dds
} //namespace fastdds
} //namespace eprosima

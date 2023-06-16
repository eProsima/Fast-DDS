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
#include <mutex>

#include <fastrtps/utils/DBQueue.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <iostream>

using namespace std;
namespace eprosima {
namespace fastdds {
namespace dds {

struct Resources
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

    Resources()
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

    ~Resources()
    {
        Log::KillThread();
    }

<<<<<<< HEAD
=======
    /**
     * Registers an user defined consumer to route log output.
     * There is a default stdout consumer active as default.
     * @param consumer r-value to a consumer unique_ptr. It will be invalidated after the call.
     */
    void RegisterConsumer(
            std::unique_ptr<LogConsumer>&& consumer)
    {
        std::unique_lock<std::mutex> guard(config_mutex_);
        consumers_.emplace_back(std::move(consumer));
    }

    //! Removes all registered consumers_, including the default stdout.
    void ClearConsumers()
    {
        Flush();

        std::lock_guard<std::mutex> guard(config_mutex_);
        consumers_.clear();
    }

    //! Enables the reporting of filenames_ in log entries. Disabled by default.
    void ReportFilenames(
            bool report)
    {
        std::lock_guard<std::mutex> configGuard(config_mutex_);
        filenames_ = report;
    }

    //! Enables the reporting of function names in log entries. Enabled by default when supported.
    void ReportFunctions(
            bool report)
    {
        std::lock_guard<std::mutex> configGuard(config_mutex_);
        functions_ = report;
    }

    //! Sets the verbosity_ level, allowing for messages equal or under that priority to be logged.
    void SetVerbosity(
            Log::Kind kind)
    {
        verbosity_ = kind;
    }

    //! Returns the current verbosity_ level.
    Log::Kind GetVerbosity()
    {
        return verbosity_;
    }

    //! Sets a filter that will pattern-match against log categories, dropping any unmatched categories.
    void SetCategoryFilter(
            const std::regex& filter)
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        category_filter_.reset(new std::regex(filter));
    }

    //! Sets a filter that will pattern-match against filenames_, dropping any unmatched categories.
    void SetFilenameFilter(
            const std::regex& filter)
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        filename_filter_.reset(new std::regex(filter));
    }

    //! Sets a filter that will pattern-match against the provided error string, dropping any unmatched categories.
    void SetErrorStringFilter(
            const std::regex& filter)
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        error_string_filter_.reset(new std::regex(filter));
    }

    //! Returns the logging_ engine to configuration defaults.
    void Reset()
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        category_filter_.reset();
        filename_filter_.reset();
        error_string_filter_.reset();
        filenames_ = false;
        functions_ = true;
        verbosity_ = Log::Error;
        consumers_.clear();

#if STDOUTERR_LOG_CONSUMER
        consumers_.emplace_back(new StdoutErrConsumer);
#else
        consumers_.emplace_back(new StdoutConsumer);
#endif // if STDOUTERR_LOG_CONSUMER
    }

    //! Waits until all info logged up to the call time is consumed
    void Flush()
    {
        std::unique_lock<std::mutex> guard(cv_mutex_);

        if (!logging_ && !logging_thread_)
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
            cv_.wait(guard,
                    [&]()
                    {
                        /* I must avoid:
                         + the two calls be processed without an intermediate Run() loop (by using last_loop sequence number)
                         + deadlock by absence of Run() loop activity (by using BothEmpty() call)
                         */
                        return !logging_ ||
                        (logs_.Empty() &&
                        (last_loop != current_loop_ || logs_.BothEmpty()));
                    });

            last_loop = current_loop_;

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
        logs_.Push(Log::Entry{ message, context, kind, timestamp });
        {
            std::unique_lock<std::mutex> guard(cv_mutex_);
            work_ = true;
            // pessimization
            cv_.notify_all();
            // wait till the thread is initialized
            cv_.wait(guard, [&]
                    {
                        return current_loop_;
                    });
        }
    }

    //! Stops the logging_ thread. It will re-launch on the next call to QueueLog.
    void KillThread()
    {
        {
            std::unique_lock<std::mutex> guard(cv_mutex_);
            logging_ = false;
            work_ = false;
        }

        if (logging_thread_)
        {
            cv_.notify_all();
            // The #ifdef workaround here is due to an unsolved MSVC bug, which Microsoft has announced
            // they have no intention of solving: https://connect.microsoft.com/VisualStudio/feedback/details/747145
            // Each VS version deals with post-main deallocation of threads in a very different way.
#if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
            if (logging_thread_->joinable() && logging_thread_->get_id() != std::this_thread::get_id())
            {
                logging_thread_->join();
            }
#endif // if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
            logging_thread_.reset();
        }
    }

private:

    void StartThread()
    {
        std::unique_lock<std::mutex> guard(cv_mutex_);
        if (!logging_ && !logging_thread_)
        {
            logging_ = true;
            logging_thread_.reset(new std::thread(&LogResources::run, this));
        }
    }

    void run()
    {
        std::unique_lock<std::mutex> guard(cv_mutex_);

        while (logging_)
        {
            cv_.wait(guard,
                    [&]()
                    {
                        return !logging_ || work_;
                    });

            work_ = false;

            guard.unlock();
            {
                logs_.Swap();
                while (!logs_.Empty())
                {
                    Log::Entry& entry = logs_.Front();
                    {
                        std::unique_lock<std::mutex> configGuard(config_mutex_);

                        if (preprocess(entry))
                        {
                            for (auto& consumer : consumers_)
                            {
                                consumer->Consume(entry);
                            }
                        }
                    }
                    // This Pop() is also a barrier for Log::Flush wait condition
                    logs_.Pop();
                }
            }
            guard.lock();

            // avoid overflow
            if (++current_loop_ > 10000)
            {
                current_loop_ = 0;
            }

            cv_.notify_all();
        }
    }

    bool preprocess(
            Log::Entry& entry)
    {
        if (category_filter_ && !regex_search(entry.context.category, *category_filter_))
        {
            return false;
        }
        if (filename_filter_ && !regex_search(entry.context.filename, *filename_filter_))
        {
            return false;
        }
        if (error_string_filter_ && !regex_search(entry.message, *error_string_filter_))
        {
            return false;
        }
        if (!filenames_)
        {
            entry.context.filename = nullptr;
        }
        if (!functions_)
        {
            entry.context.function = nullptr;
        }

        return true;
    }

    fastrtps::DBQueue<Log::Entry> logs_;
    std::vector<std::unique_ptr<LogConsumer>> consumers_;
    std::unique_ptr<std::thread> logging_thread_;

    // Condition variable segment.
    std::condition_variable cv_;
    std::mutex cv_mutex_;
    bool logging_;
    bool work_;
    int current_loop_;

    // Context configuration.
    std::mutex config_mutex_;
    bool filenames_;
    bool functions_;
    std::unique_ptr<std::regex> category_filter_;
    std::unique_ptr<std::regex> filename_filter_;
    std::unique_ptr<std::regex> error_string_filter_;

    std::atomic<Log::Kind> verbosity_;

>>>>>>> 5ffc8a42f (Fix tests with Log: wait for Log background thread initialization on the first queued entry [16948] (#3270))
};

static struct Resources resources_;


void Log::RegisterConsumer(
        std::unique_ptr<LogConsumer>&& consumer)
{
    std::unique_lock<std::mutex> guard(resources_.config_mutex);
    resources_.consumers.emplace_back(std::move(consumer));
}

void Log::ClearConsumers()
{
    std::unique_lock<std::mutex> working(resources_.cv_mutex);
    resources_.cv.wait(working,
            [&]()
            {
                return resources_.logs.BothEmpty();
            });
    std::unique_lock<std::mutex> guard(resources_.config_mutex);
    resources_.consumers.clear();
}

void Log::Reset()
{
    std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
    resources_.category_filter.reset();
    resources_.filename_filter.reset();
    resources_.error_string_filter.reset();
    resources_.filenames = false;
    resources_.functions = true;
    resources_.verbosity = Log::Error;
    resources_.consumers.clear();
#if STDOUTERR_LOG_CONSUMER
    resources_.consumers.emplace_back(new StdoutErrConsumer);
#else
    resources_.consumers.emplace_back(new StdoutConsumer);
#endif // if STDOUTERR_LOG_CONSUMER
}

void Log::Flush()
{
    std::unique_lock<std::mutex> guard(resources_.cv_mutex);

    if (!resources_.logging && !resources_.logging_thread)
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
        resources_.cv.wait(guard,
                [&]()
                {
                    /* I must avoid:
                     + the two calls be processed without an intermediate Run() loop (by using last_loop sequence number)
                     + deadlock by absence of Run() loop activity (by using BothEmpty() call)
                     */
                    return !resources_.logging ||
                    ( resources_.logs.Empty() &&
                    ( last_loop != resources_.current_loop || resources_.logs.BothEmpty()));
                });

        last_loop = resources_.current_loop;

    }
}

void Log::run()
{
    std::unique_lock<std::mutex> guard(resources_.cv_mutex);

    while (resources_.logging)
    {
        resources_.cv.wait(guard,
                [&]()
                {
                    return !resources_.logging || resources_.work;
                });

        resources_.work = false;

        guard.unlock();
        {
            resources_.logs.Swap();
            while (!resources_.logs.Empty())
            {
                std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
                if (preprocess(resources_.logs.Front()))
                {
                    for (auto& consumer : resources_.consumers)
                    {
                        consumer->Consume(resources_.logs.Front());
                    }
                }

                resources_.logs.Pop();
            }
        }
        guard.lock();

        // avoid overflow
        if (++resources_.current_loop > 10000)
        {
            resources_.current_loop = 0;
        }

        resources_.cv.notify_all();
    }
}

void Log::ReportFilenames(
        bool report)
{
    std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
    resources_.filenames = report;
}

void Log::ReportFunctions(
        bool report)
{
    std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
    resources_.functions = report;
}

bool Log::preprocess(
        Log::Entry& entry)
{
    if (resources_.category_filter && !regex_search(entry.context.category, *resources_.category_filter))
    {
        return false;
    }
    if (resources_.filename_filter && !regex_search(entry.context.filename, *resources_.filename_filter))
    {
        return false;
    }
    if (resources_.error_string_filter && !regex_search(entry.message, *resources_.error_string_filter))
    {
        return false;
    }
    if (!resources_.filenames)
    {
        entry.context.filename = nullptr;
    }
    if (!resources_.functions)
    {
        entry.context.function = nullptr;
    }

    return true;
}

void Log::KillThread()
{
    {
        std::unique_lock<std::mutex> guard(resources_.cv_mutex);
        resources_.logging = false;
        resources_.work = false;
    }

    if (resources_.logging_thread)
    {
        resources_.cv.notify_all();
        // The #ifdef workaround here is due to an unsolved MSVC bug, which Microsoft has announced
        // they have no intention of solving: https://connect.microsoft.com/VisualStudio/feedback/details/747145
        // Each VS version deals with post-main deallocation of threads in a very different way.
#if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
        resources_.logging_thread->join();
#endif // if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
        resources_.logging_thread.reset();
    }
}

void Log::QueueLog(
        const std::string& message,
        const Log::Context& context,
        Log::Kind kind)
{
    {
        std::unique_lock<std::mutex> guard(resources_.cv_mutex);
        if (!resources_.logging && !resources_.logging_thread)
        {
            resources_.logging = true;
            resources_.logging_thread.reset(new thread(Log::run));
        }
    }

    std::string timestamp;
    get_timestamp(timestamp);
    resources_.logs.Push(Log::Entry{message, context, kind, timestamp});
    {
        std::unique_lock<std::mutex> guard(resources_.cv_mutex);
        resources_.work = true;
    }
    resources_.cv.notify_all();
}

Log::Kind Log::GetVerbosity()
{
    return resources_.verbosity;
}

void Log::SetVerbosity(
        Log::Kind kind)
{
    std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
    resources_.verbosity = kind;
}

void Log::SetCategoryFilter(
        const std::regex& filter)
{
    std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
    resources_.category_filter.reset(new std::regex(filter));
}

void Log::SetFilenameFilter(
        const std::regex& filter)
{
    std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
    resources_.filename_filter.reset(new std::regex(filter));
}

void Log::SetErrorStringFilter(
        const std::regex& filter)
{
    std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
    resources_.error_string_filter.reset(new std::regex(filter));
}

void Log::get_timestamp(
        std::string& timestamp)
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
#endif // if defined(_WIN32)
    timestamp = stream.str();
}

void LogConsumer::print_timestamp(
        std::ostream& stream,
        const Log::Entry& entry,
        bool color) const
{
    std::string white = (color) ? C_B_WHITE : "";
    stream << white << entry.timestamp;
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

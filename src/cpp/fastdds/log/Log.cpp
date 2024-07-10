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

#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>

#include <utils/DBQueue.hpp>
#include <utils/SystemInfo.hpp>
#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct LogResources
{
    LogResources()
        : logging_(false)
        , work_(false)
        , current_loop_(0)
        , filenames_(false)
        , functions_(true)
        , verbosity_(Log::Error)
    {
#if STDOUTERR_LOG_CONSUMER
        consumers_.emplace_back(new StdoutErrConsumer);
#else
        consumers_.emplace_back(new StdoutConsumer);
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

    void UnsetCategoryFilter()
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        category_filter_.reset();
    }

    bool HasCategoryFilter()
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        return !!category_filter_;
    }

    //! Returns a copy of the current category filter or an empty object otherwise
    std::regex GetCategoryFilter()
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        return category_filter_ ? *category_filter_ : std::regex{};
    }

    //! Sets a filter that will pattern-match against filenames_, dropping any unmatched categories.
    void SetFilenameFilter(
            const std::regex& filter)
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        filename_filter_.reset(new std::regex(filter));
    }

    //! Returns a copy of the current filename filter or an empty object otherwise
    std::regex GetFilenameFilter()
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        return filename_filter_ ? *filename_filter_: std::regex{};
    }

    //! Sets a filter that will pattern-match against the provided error string, dropping any unmatched categories.
    void SetErrorStringFilter(
            const std::regex& filter)
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        error_string_filter_.reset(new std::regex(filter));
    }

    //! Sets thread configuration for the logging thread.
    void SetThreadConfig(
            const rtps::ThreadSettings& config)
    {
        std::lock_guard<std::mutex> guard(cv_mutex_);
        thread_settings_ = config;
    }

    //! Returns a copy of the current filename filter or an empty object otherwise
    std::regex GetErrorStringFilter()
    {
        std::unique_lock<std::mutex> configGuard(config_mutex_);
        return error_string_filter_ ? *error_string_filter_: std::regex{};
    }

    //! Returns the logging_ engine to configuration defaults.
    void Reset()
    {
        rtps::ThreadSettings thr_config{};
        SetThreadConfig(thr_config);

        std::lock_guard<std::mutex> configGuard(config_mutex_);
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

        if (!logging_ && !logging_thread_.joinable())
        {
            // already killed
            return;
        }

        /*   Flush() two steps strategy:

             We must assure Log::Run swaps the queues twice
             because its the only way the content in the background queue
             will be consumed (first Run() loop).

             Then, we must assure the new front queue content is consumed (second Run() loop).
         */

        int last_loop = current_loop_;

        for (int i = 0; i < 2; ++i)
        {
            cv_.wait(guard,
                    [&]()
                    {
                        /* We must avoid:
                         + the two calls be processed without an intermediate Run() loop
                           (by using last_loop sequence number and checking if the foreground queue is empty)
                         + deadlock by absence of Run() loop activity (by using BothEmpty() call)
                         */
                        return !logging_ ||
                        logs_.BothEmpty() ||
                        (last_loop != current_loop_ && logs_.Empty());
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

        if (logging_thread_.joinable())
        {
            cv_.notify_all();
            if (!logging_thread_.is_calling_thread())
            {
                logging_thread_.join();
            }
        }
    }

private:

    void StartThread()
    {
        std::unique_lock<std::mutex> guard(cv_mutex_);
        if (!logging_ && !logging_thread_.joinable())
        {
            logging_ = true;
            auto thread_fn = [this]()
                    {
                        run();
                    };
            logging_thread_ = eprosima::create_thread(thread_fn, thread_settings_, "dds.log");
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

    DBQueue<Log::Entry> logs_;
    std::vector<std::unique_ptr<LogConsumer>> consumers_;
    eprosima::thread logging_thread_;

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
    rtps::ThreadSettings thread_settings_;
};

const std::shared_ptr<LogResources>& get_log_resources()
{
    static std::shared_ptr<LogResources> instance = std::make_shared<LogResources>();
    return instance;
}

}  // namespace detail

void Log::RegisterConsumer(
        std::unique_ptr<LogConsumer>&& consumer)
{
    detail::get_log_resources()->RegisterConsumer(std::move(consumer));
}

void Log::ClearConsumers()
{
    detail::get_log_resources()->ClearConsumers();
}

void Log::Reset()
{
    detail::get_log_resources()->Reset();
}

void Log::Flush()
{
    detail::get_log_resources()->Flush();
}

void Log::ReportFilenames(
        bool report)
{
    detail::get_log_resources()->ReportFilenames(report);
}

void Log::ReportFunctions(
        bool report)
{
    detail::get_log_resources()->ReportFunctions(report);
}

void Log::KillThread()
{
    detail::get_log_resources()->KillThread();
}

void Log::QueueLog(
        const std::string& message,
        const Log::Context& context,
        Log::Kind kind)
{
    detail::get_log_resources()->QueueLog(message, context, kind);
}

Log::Kind Log::GetVerbosity()
{
    return detail::get_log_resources()->GetVerbosity();
}

void Log::SetVerbosity(
        Log::Kind kind)
{
    detail::get_log_resources()->SetVerbosity(kind);
}

void Log::SetCategoryFilter(
        const std::regex& filter)
{
    detail::get_log_resources()->SetCategoryFilter(filter);
}

void Log::SetFilenameFilter(
        const std::regex& filter)
{
    detail::get_log_resources()->SetFilenameFilter(filter);
}

void Log::SetErrorStringFilter(
        const std::regex& filter)
{
    detail::get_log_resources()->SetErrorStringFilter(filter);
}

void Log::SetThreadConfig(
        const rtps::ThreadSettings& config)
{
    detail::get_log_resources()->SetThreadConfig(config);
}

void Log::UnsetCategoryFilter()
{
    return detail::get_log_resources()->UnsetCategoryFilter();
}

bool Log::HasCategoryFilter()
{
    return detail::get_log_resources()->HasCategoryFilter();
}

std::regex Log::GetCategoryFilter()
{
    return detail::get_log_resources()->GetCategoryFilter();
}

std::regex Log::GetFilenameFilter()
{
    return detail::get_log_resources()->GetFilenameFilter();
}

std::regex Log::GetErrorStringFilter()
{
    return detail::get_log_resources()->GetErrorStringFilter();
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

    stream << c_b_color << "[" << white << entry.context.category << c_b_color << " " << entry.kind << "] ";
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

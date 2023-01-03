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
        KillThread();
    }

    void KillThread();

};

static std::shared_ptr<LogResources> resources_instance()
{
    static std::shared_ptr<LogResources> instance = std::make_shared<LogResources>();
    return instance;
}

void Log::RegisterConsumer(
        std::unique_ptr<LogConsumer>&& consumer)
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> guard(resources->config_mutex);
    resources->consumers.emplace_back(std::move(consumer));
}

void Log::ClearConsumers()
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> working(resources->cv_mutex);
    resources->cv.wait(working,
            [&]()
            {
                return resources->logs.BothEmpty();
            });
    std::unique_lock<std::mutex> guard(resources->config_mutex);
    resources->consumers.clear();
}

void Log::Reset()
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> configGuard(resources->config_mutex);
    resources->category_filter.reset();
    resources->filename_filter.reset();
    resources->error_string_filter.reset();
    resources->filenames = false;
    resources->functions = true;
    resources->verbosity = Log::Error;
    resources->consumers.clear();

#if STDOUTERR_LOG_CONSUMER
    resources->consumers.emplace_back(new StdoutErrConsumer);
#else
    resources->consumers.emplace_back(new StdoutConsumer);
#endif // if STDOUTERR_LOG_CONSUMER
}

void Log::Flush()
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> guard(resources->cv_mutex);

    if (!resources->logging && !resources->logging_thread)
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
        resources->cv.wait(guard,
                [&]()
                {
                    /* I must avoid:
                     + the two calls be processed without an intermediate Run() loop (by using last_loop sequence number)
                     + deadlock by absence of Run() loop activity (by using BothEmpty() call)
                     */
                    return !resources->logging ||
                    ( resources->logs.Empty() &&
                    ( last_loop != resources->current_loop || resources->logs.BothEmpty()));
                });

        last_loop = resources->current_loop;

    }
}

void Log::run()
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> guard(resources->cv_mutex);

    while (resources->logging)
    {
        resources->cv.wait(guard,
                [&]()
                {
                    return !resources->logging || resources->work;
                });

        resources->work = false;

        guard.unlock();
        {
            resources->logs.Swap();
            while (!resources->logs.Empty())
            {
                std::unique_lock<std::mutex> configGuard(resources->config_mutex);

                Log::Entry& entry = resources->logs.Front();
                if (preprocess(entry))
                {
                    for (auto& consumer : resources->consumers)
                    {
                        consumer->Consume(entry);
                    }
                }
                // This Pop() is also a barrier for Log::Flush wait condition
                resources->logs.Pop();
            }
        }
        guard.lock();

        // avoid overflow
        if (++resources->current_loop > 10000)
        {
            resources->current_loop = 0;
        }

        resources->cv.notify_all();
    }
}

void Log::ReportFilenames(
        bool report)
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> configGuard(resources->config_mutex);
    resources->filenames = report;
}

void Log::ReportFunctions(
        bool report)
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> configGuard(resources->config_mutex);
    resources->functions = report;
}

bool Log::preprocess(
        Log::Entry& entry)
{
    auto resources = resources_instance();
    if (resources->category_filter && !regex_search(entry.context.category, *resources->category_filter))
    {
        return false;
    }
    if (resources->filename_filter && !regex_search(entry.context.filename, *resources->filename_filter))
    {
        return false;
    }
    if (resources->error_string_filter && !regex_search(entry.message, *resources->error_string_filter))
    {
        return false;
    }
    if (!resources->filenames)
    {
        entry.context.filename = nullptr;
    }
    if (!resources->functions)
    {
        entry.context.function = nullptr;
    }

    return true;
}

void Log::KillThread()
{
    resources_instance()->KillThread();
}

void LogResources::KillThread()
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
        logging_thread->join();
#endif // if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
        logging_thread.reset();
    }
}

void Log::QueueLog(
        const std::string& message,
        const Log::Context& context,
        Log::Kind kind)
{
    auto resources = resources_instance();
    {
        std::unique_lock<std::mutex> guard(resources->cv_mutex);
        if (!resources->logging && !resources->logging_thread)
        {
            resources->logging = true;
            resources->logging_thread.reset(new thread(Log::run));
        }
    }

    std::string timestamp = SystemInfo::get_timestamp();
    resources->logs.Push(Log::Entry{message, context, kind, timestamp});
    {
        std::unique_lock<std::mutex> guard(resources->cv_mutex);
        resources->work = true;
    }
    resources->cv.notify_all();
}

Log::Kind Log::GetVerbosity()
{
    return resources_instance()->verbosity;
}

void Log::SetVerbosity(
        Log::Kind kind)
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> configGuard(resources->config_mutex);
    resources->verbosity = kind;
}

void Log::SetCategoryFilter(
        const std::regex& filter)
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> configGuard(resources->config_mutex);
    resources->category_filter.reset(new std::regex(filter));
}

void Log::SetFilenameFilter(
        const std::regex& filter)
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> configGuard(resources->config_mutex);
    resources->filename_filter.reset(new std::regex(filter));
}

void Log::SetErrorStringFilter(
        const std::regex& filter)
{
    auto resources = resources_instance();
    std::unique_lock<std::mutex> configGuard(resources->config_mutex);
    resources->error_string_filter.reset(new std::regex(filter));
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

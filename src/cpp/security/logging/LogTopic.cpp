#include <security/logging/LogTopic.h>

#include <functional>

#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

LogTopic::LogTopic(
        uint32_t thread_id,
        const fastdds::rtps::ThreadSettings& thr_config)
    : stop_(false)
    , thread_(
        create_thread(
            [this]()
            {
                while (true)
                {
                    // Put the thread asleep until there is
                    // something to process
                    auto p = queue_.wait_pop();

                    if (!p)
                    {
                        if (stop_)
                        {
                            return;
                        }
                        continue;
                    }

                    publish(*p);
                }
            },
            thr_config, "dds.slog.%u", thread_id))
{
    //
}

LogTopic::~LogTopic()
{
    stop();
    queue_.push(BuiltinLoggingTypePtr(nullptr));
    if (thread_.joinable())
    {
        thread_.join();
    }

    if (file_stream_.is_open())
    {
        file_stream_.close();
    }
}

void LogTopic::log_impl(
        const BuiltinLoggingType& message,
        SecurityException& /*exception*/) const
{
    queue_.push(BuiltinLoggingTypePtr(new BuiltinLoggingType(message)));
}

bool LogTopic::enable_logging_impl(
        SecurityException& exception)
{
    LogOptions options;
    if (!get_log_options(options, exception))
    {
        return false;
    }

    if (!options.log_file.empty())
    {
        file_stream_.open(options.log_file, std::ios::out | std::ios::app);

        if ((file_stream_.rdstate() & std::ofstream::failbit ) != 0 )
        {
            exception = SecurityException("Error opening file: " + options.log_file);
            return false;
        }
    }

    return true;
}

void LogTopic::publish(
        BuiltinLoggingType& builtin_msg)
{
    SecurityException exception;
    if (!file_stream_.is_open() || !compose_header(file_stream_, builtin_msg, exception))
    {
        return;
    }

    file_stream_ << " : " << builtin_msg.message << "\n";
    file_stream_.flush();
}

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

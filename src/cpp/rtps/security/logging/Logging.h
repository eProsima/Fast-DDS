// Copyright 2020 Canonical ltd.
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

/*!
 * @file Logging.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_LOGGING_LOGGING_H_
#define _FASTDDS_RTPS_SECURITY_LOGGING_LOGGING_H_

#include <limits>
#include <iomanip>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <rtps/security/logging/BuiltinLoggingType.h>
#include <rtps/security/logging/LogOptions.h>
#include <rtps/security/exceptions/SecurityException.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class SecurityException;

/**
 * @brief The LoggerListener class
 */
class LoggerListener
{
    LoggerListener() = default;
    ~LoggerListener() = default;
};

/**
 * @brief Base class for all security logging plugins.
 */
class Logging
{
public:

    Logging();
    virtual ~Logging() = default;

    /**
     * @brief set_log_options
     * @param log_options
     * @param exception
     * @return TRUE if successful
     */
    bool set_log_options(
            const LogOptions& log_options,
            SecurityException& exception);

    /**
     * @brief get_log_options
     * @param log_options
     * @param exception
     * @return
     */
    bool get_log_options(
            LogOptions& log_options,
            SecurityException& exception) const;

    /**
     * @brief enable_logging
     */
    bool enable_logging(
            SecurityException& exception);

    /**
     * @brief set_listener
     * @param listener
     * @param exception
     * @return
     */
    bool set_listener(
            LoggerListener* listener,
            SecurityException& exception);

    /**
     * @brief log
     * @param log_level
     * @param message
     * @param category
     * @param exception
     */
    void log(
            const LoggingLevel log_level,
            const std::string& message,
            const std::string& category,
            SecurityException& exception) const;

    /**
     * @brief Whether the options are set or not.
     * @return True if the options are set.
     */
    bool options_set() const
    {
        return options_set_;
    }

    /**
     * @brief Whether the logging is enabled or not.
     * @return True if the logging is enabled.
     */
    bool enabled() const
    {
        return logging_enabled_;
    }

    /**
     * @brief Return the LoggerListener.
     * @return A pointer to the (const) LoggerListener.
     */
    LoggerListener const* get_listener() const
    {
        return listener_;
    }

    bool set_guid(
            const GUID_t& guid,
            SecurityException& exception);

    bool set_domain_id(
            const uint32_t id,
            SecurityException& exception);

protected:

    /**
     * @brief enable_logging_impl
     * @return
     */
    virtual bool enable_logging_impl(
            SecurityException& /*exception*/)
    {
        return true;
    }

    /**
     * @brief convert
     * @param log_level
     * @param message
     * @param category
     * @param builtin_msg
     * @param exception
     * @return
     */
    virtual bool convert(
            const LoggingLevel log_level,
            const std::string& message,
            const std::string& category,
            BuiltinLoggingType& builtin_msg,
            SecurityException& exception) const;

    template <typename Stream>
    bool compose_header(
            Stream& header,
            const BuiltinLoggingType& builtin_msg,
            SecurityException& exception) const;

    /**
     * @brief log_impl
     * @param message
     * @param exception
     */
    virtual void log_impl(
            const BuiltinLoggingType& message,
            SecurityException& exception) const = 0;

private:

    LoggerListener* listener_;

    bool logging_enabled_ = false;
    bool options_set_ = false;

    LogOptions log_options_;
    GUID_t guid_;
    std::string guid_str_;
    uint32_t domain_id_ = (std::numeric_limits<uint32_t>::max)();
    std::string domain_id_str_;
};

template <typename Stream>
bool Logging::compose_header(
        Stream& header,
        const BuiltinLoggingType& builtin_msg,
        SecurityException& exception) const
{
    const auto it = builtin_msg.structured_data.find("DDS");

    if (builtin_msg.structured_data.end() == it)
    {
        exception = SecurityException("Could not find expected DDS field.");
        return false;
    }

    std::string severity;
    if (!LogLevel_to_string(builtin_msg.severity, severity, exception))
    {
        return false;
    }

    // header format is:
    // [stamp] [severity] <guid> <domain_id> <plugin_class::plugin_method>
    header << std::setprecision (std::numeric_limits<double>::digits10 + 1)
           << "[" << builtin_msg.timestamp << "] "
           << "[" << severity << "] "
           << it->second.at(0).value << " "
           << it->second.at(1).value << " "
           << it->second.at(2).value << "::" << it->second.at(3).value;

    return true;
}

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima


// gcc expands __VA_ARGS___ before passing it into the macro.
// Visual Studio expands __VA_ARGS__ after passing it.
// This macro is a workaround to support both
#define __FASTDDS_EXPAND(x) x

#define __FASTDDS_SECURITY_LOGGING(LEVEL, CLASS, MESSAGE, EXCEPTION) \
    do {                                                              \
        auto logger = get_logger();                                   \
        if (logger){                                                  \
            logger->log(LEVEL,                                        \
                    MESSAGE,                                          \
                    std::string(CLASS ",") + __func__,                \
                    EXCEPTION);                                       \
        }                                                             \
        else {                                                        \
            switch (LEVEL){                                           \
                case LoggingLevel::EMERGENCY_LEVEL:                   \
                case LoggingLevel::ALERT_LEVEL:                       \
                case LoggingLevel::CRITICAL_LEVEL:                    \
                case LoggingLevel::ERROR_LEVEL:                       \
                    EPROSIMA_LOG_ERROR(SECURITY, MESSAGE);            \
                    break;                                            \
                case LoggingLevel::WARNING_LEVEL:                     \
                    EPROSIMA_LOG_WARNING(SECURITY, MESSAGE);          \
                    break;                                            \
                case LoggingLevel::NOTICE_LEVEL:                      \
                case LoggingLevel::INFORMATIONAL_LEVEL:               \
                case LoggingLevel::DEBUG_LEVEL:                       \
                    EPROSIMA_LOG_INFO(SECURITY, MESSAGE);             \
                    break;                                            \
            }                                                         \
        }                                                             \
    } while (0);

#define __FASTDDS_SECURITY_LOGGING_EX(LEVEL, CLASS, MESSAGE)             \
    do {                                                                  \
        eprosima::fastdds::rtps::security::SecurityException lexception; \
        __FASTDDS_SECURITY_LOGGING(LEVEL, CLASS, MESSAGE, lexception);   \
    } while (0);

#define __FASTDDS_MACRO_SELECTOR(_1, _2, _3, _4, NAME, ...) NAME

#define SECURITY_LOGGING(...)                   \
    __FASTDDS_EXPAND(                          \
        __FASTDDS_MACRO_SELECTOR(__VA_ARGS__,  \
        __FASTDDS_SECURITY_LOGGING,            \
        __FASTDDS_SECURITY_LOGGING_EX,         \
        _UNUSED)(__VA_ARGS__))

#define EMERGENCY_SECURITY_LOGGING(...)     SECURITY_LOGGING(LoggingLevel::EMERGENCY_LEVEL, __VA_ARGS__)
#define ALERT_SECURITY_LOGGING(...)         SECURITY_LOGGING(LoggingLevel::ALERT_LEVEL, __VA_ARGS__)
#define CRITICAL_SECURITY_LOGGING(...)      SECURITY_LOGGING(LoggingLevel::CRITICAL_LEVEL, __VA_ARGS__)
#define ERROR_SECURITY_LOGGING(...)         SECURITY_LOGGING(LoggingLevel::ERROR_LEVEL, __VA_ARGS__)
#define WARNING_SECURITY_LOGGING(...)       SECURITY_LOGGING(LoggingLevel::WARNING_LEVEL, __VA_ARGS__)
#define NOTICE_SECURITY_LOGGING(...)        SECURITY_LOGGING(LoggingLevel::NOTICE_LEVEL, __VA_ARGS__)
#define INFORMATIONAL_SECURITY_LOGGING(...) SECURITY_LOGGING(LoggingLevel::INFORMATIONAL_LEVEL, __VA_ARGS__)
#define DEBUG_SECURITY_LOGGING(...)         SECURITY_LOGGING(LoggingLevel::DEBUG_LEVEL, __VA_ARGS__)


#endif // _FASTDDS_RTPS_SECURITY_LOGGING_LOGGING_H_

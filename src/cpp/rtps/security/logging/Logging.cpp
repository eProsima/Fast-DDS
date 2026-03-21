#include <rtps/security/logging/Logging.h>

#include <rtps/security/exceptions/SecurityException.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

Logging::Logging()
    : listener_(nullptr)
    , logging_enabled_(false)
    , options_set_(false)
{
    //
}

bool Logging::set_log_options(
        const LogOptions& log_options,
        SecurityException& exception)
{
    if (logging_enabled_)
    {
        exception = SecurityException("Cannot set option after enable_logging called.");
        return false;
    }

    log_options_ = log_options;
    options_set_ = true;

    if (log_options_.distribute)
    {
        //TODO(artivis): set up DataWriter/Publisher
    }

    return true;
}

bool Logging::get_log_options(
        LogOptions& log_options,
        SecurityException& exception) const
{
    if (!options_set_)
    {
        exception = SecurityException("Logging options not set!");
        return false;
    }

    log_options = log_options_;
    return true;
}

bool Logging::enable_logging(
        SecurityException& exception)
{
    if (!options_set_)
    {
        exception = SecurityException("Cannot enable logging before logging options are set!");
        return false;
    }

    if (!enable_logging_impl(exception))
    {
        return false;
    }

    logging_enabled_ = true;

    return true;
}

bool Logging::set_listener(
        LoggerListener* /*listener*/,
        SecurityException& exception)
{
    exception = SecurityException("Not implemented yet.");

    return false;
}

void Logging::log(
        const LoggingLevel log_level,
        const std::string& message,
        const std::string& category,
        SecurityException& exception) const
{
    if (logging_enabled_)
    {
        if (log_level <= log_options_.log_level)
        {
            BuiltinLoggingType builtin_msg;

            if (!convert(log_level, message, category, builtin_msg, exception))
            {
                return;
            }

            log_impl(builtin_msg, exception);
        }
    }
}

bool Logging::convert(
        const LoggingLevel log_level,
        const std::string& message,
        const std::string& category,
        BuiltinLoggingType& builtin_msg,
        SecurityException& exception) const
{
    rtps::Time_t::now(builtin_msg.timestamp);

    builtin_msg.facility = 0;
    builtin_msg.severity = log_level;
    builtin_msg.message = message;

    std::string plugin_class;
    std::string plugin_method;

    const std::size_t pos = category.find(',');

    if (pos != std::string::npos)
    {
        plugin_class = category.substr(0, pos);
        plugin_method = category.substr(pos + 1);
    }
    else
    {
        exception = SecurityException("Could not find expected separator.");
        return false;
    }

    builtin_msg.structured_data.emplace(
        "DDS",
        NameValuePairSeq{NameValuePair{"guid", guid_str_},
                         NameValuePair{"domain_id", domain_id_str_},
                         NameValuePair{"plugin_class", plugin_class},
                         NameValuePair{"plugin_method", plugin_method}}
        );

    return true;
}

void Logging::log_impl(
        const BuiltinLoggingType& /*message*/,
        SecurityException& exception) const
{
    exception = SecurityException("Logging not implemented.");
}

bool Logging::set_guid(
        const GUID_t& guid,
        SecurityException& exception)
{
    if (GUID_t::unknown() == guid)
    {
        exception = SecurityException("Invalid guid value.");
        return false;
    }
    else if (GUID_t::unknown() != guid_)
    {
        exception = SecurityException("Guid already set.");
        return false;
    }

    guid_ = guid;

    std::stringstream ss;
    ss << guid_;

    guid_str_ = ss.str();

    return true;
}

bool Logging::set_domain_id(
        const uint32_t id,
        SecurityException& exception)
{
    if ((std::numeric_limits<uint32_t>::max)() == id)
    {
        exception = SecurityException("Invalid domaine id value.");
        return false;
    }
    else if ((std::numeric_limits<uint32_t>::max)() != domain_id_)
    {
        exception = SecurityException("Domaine id already set (" + std::to_string(domain_id_) + ")");
        return false;
    }

    domain_id_ = id;
    domain_id_str_ = std::to_string(domain_id_);

    return true;
}

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file Log.cpp
 *
 * Definitions of the Log mock. Logged messages are discarded and no file is opened.
 */

#include <iostream>

#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include <mock/fastdds/dds/log/LogMock.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

namespace {

Log::Kind verbosity_ = Log::Kind::Error;

} // namespace

LogMock*& LogMock::instance()
{
    static LogMock* instance = nullptr;
    return instance;
}

void Log::RegisterConsumer(
        std::unique_ptr<LogConsumer>&& consumer)
{
    if (nullptr != LogMock::instance())
    {
        LogMock::instance()->RegisterConsumer(std::move(consumer));
    }
}

void Log::ClearConsumers()
{
    if (nullptr != LogMock::instance())
    {
        LogMock::instance()->ClearConsumers();
    }
}

void Log::ReportFilenames(
        bool)
{
}

void Log::ReportFunctions(
        bool)
{
}

void Log::SetVerbosity(
        Log::Kind kind)
{
    verbosity_ = kind;
}

Log::Kind Log::GetVerbosity()
{
    return verbosity_;
}

void Log::SetCategoryFilter(
        const std::regex&)
{
}

void Log::UnsetCategoryFilter()
{
}

bool Log::HasCategoryFilter()
{
    return false;
}

std::regex Log::GetCategoryFilter()
{
    return std::regex();
}

void Log::SetFilenameFilter(
        const std::regex&)
{
}

std::regex Log::GetFilenameFilter()
{
    return std::regex();
}

void Log::SetErrorStringFilter(
        const std::regex&)
{
}

void Log::SetThreadConfig(
        const rtps::ThreadSettings&)
{
    if (nullptr != LogMock::instance())
    {
        LogMock::instance()->SetThreadConfig();
    }
}

std::regex Log::GetErrorStringFilter()
{
    return std::regex();
}

void Log::Reset()
{
    verbosity_ = Log::Kind::Error;
}

void Log::Flush()
{
}

void Log::KillThread()
{
}

void Log::QueueLog(
        const std::string&,
        const Log::Context&,
        Log::Kind)
{
}

void LogConsumer::print_timestamp(
        std::ostream&,
        const Log::Entry&,
        bool) const
{
}

void LogConsumer::print_header(
        std::ostream&,
        const Log::Entry&,
        bool) const
{
}

void LogConsumer::print_context(
        std::ostream&,
        const Log::Entry&,
        bool) const
{
}

void LogConsumer::print_message(
        std::ostream&,
        const Log::Entry&,
        bool) const
{
}

void LogConsumer::print_new_line(
        std::ostream&,
        bool) const
{
}

void OStreamConsumer::Consume(
        const Log::Entry&)
{
}

std::ostream& StdoutConsumer::get_stream(
        const Log::Entry&)
{
    return std::cout;
}

void StdoutErrConsumer::stderr_threshold(
        const Log::Kind& kind)
{
    stderr_threshold_ = kind;
}

Log::Kind StdoutErrConsumer::stderr_threshold() const
{
    return stderr_threshold_;
}

std::ostream& StdoutErrConsumer::get_stream(
        const Log::Entry& entry)
{
    return (entry.kind <= stderr_threshold_) ? std::cerr : std::cout;
}

FileConsumer::FileConsumer()
    : FileConsumer("output.log")
{
}

FileConsumer::FileConsumer(
        const std::string& filename,
        bool append)
    : output_file_(filename)
    , append_(append)
{
}

FileConsumer::~FileConsumer()
{
}

std::ostream& FileConsumer::get_stream(
        const Log::Entry&)
{
    return file_;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include <iomanip>

namespace eprosima {
namespace fastdds {
namespace dds {

void StdoutErrConsumer::Consume(
        const Log::Entry& entry)
{
    // If Log::Kind is stderr_threshold_ or more severe, then use STDERR, else use STDOUT
    if (entry.kind <= stderr_threshold_)
    {
        stream_ = &std::cerr;
    }
    else
    {
        stream_ = &std::cout;
    }

    print_header(entry);
    print_message(stream_, entry, true);
    print_context(entry);
    print_new_line(stream_, true);
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

void StdoutErrConsumer::print_header(
        const Log::Entry& entry) const
{
    print_timestamp(stream_, entry, true);
    LogConsumer::print_header(stream_, entry, true);
}

void StdoutErrConsumer::print_context(
        const Log::Entry& entry) const
{
    LogConsumer::print_context(stream_, entry, true);
}

} // Namespace dds
} // Namespace fastdds
} // Namespace eprosima

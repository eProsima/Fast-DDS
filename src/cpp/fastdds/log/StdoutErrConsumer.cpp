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
    // If Log::Kind is stderr_threshold_ or more severe, then use STDERR, else use STDOUT
    if (entry.kind <= stderr_threshold_)
    {
        return std::cerr;
    }
    return std::cout;
}

} // Namespace dds
} // Namespace fastdds
} // Namespace eprosima

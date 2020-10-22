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

#include <fastdds/dds/log/FileConsumer.hpp>
#include <iomanip>

namespace eprosima {
namespace fastdds {
namespace dds {

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
    if (append_)
    {
        file_.open(output_file_, std::ios::out | std::ios::app);
    }
    else
    {
        file_.open(output_file_, std::ios::out);
    }
}

FileConsumer::~FileConsumer()
{
    file_.close();
}

std::ostream& FileConsumer::get_stream(
        const Log::Entry& entry)
{
    (void) entry;
    return file_;
}

} // Namespace dds
} // Namespace fastdds
} // Namespace eprosima

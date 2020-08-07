// Copyright 2019, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
        file_ = new std::ofstream(output_file_, std::ios::out | std::ios::app);
    }
    else
    {
        file_ = new std::ofstream(output_file_, std::ios::out);
    }
}

FileConsumer::~FileConsumer()
{
    if (file_ != nullptr)
    {
        file_->close();
    }
}

void FileConsumer::Consume(
        const Log::Entry& entry)
{
    print_header(entry);
    print_message(file_, entry, false);
    print_context(entry);
    print_new_line(file_, false);
    file_->flush();
}

void FileConsumer::print_header(
        const Log::Entry& entry)
{
    print_timestamp(file_, entry, false);
    LogConsumer::print_header(file_, entry, false);
}

void FileConsumer::print_context(
        const Log::Entry& entry)
{
    LogConsumer::print_context(file_, entry, false);
}

} // Namespace dds
} // Namespace fastdds
} // Namespace eprosima

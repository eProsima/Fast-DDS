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

#include <fastdds/dds/log/OStreamConsumer.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

void OStreamConsumer::Consume(
        const Log::Entry& entry)
{
    std::ostream& stream = get_stream(entry);
    print_timestamp(stream, entry, true);
    print_header(stream, entry, true);
    print_message(stream, entry, true);
    print_context(stream, entry, true);
    print_new_line(stream, true);
    stream.flush();
}

} // Namespace dds
} // Namespace fastdds
} // Namespace eprosima

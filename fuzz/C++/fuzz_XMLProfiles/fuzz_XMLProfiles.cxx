// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "fuzz_utils.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

using namespace eprosima;
using namespace eprosima::fastdds;

static bool initialized = false;

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    if (!initialized)
    {
        ignore_stdout();
        initialized = true;
    }

    if (!size)
    {
        return EXIT_FAILURE;
    }

    fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_string(reinterpret_cast<const char*>(data), size);

    return 0;
}

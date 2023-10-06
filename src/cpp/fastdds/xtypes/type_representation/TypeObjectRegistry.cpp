// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/xtypes/type_representation/TypeObjectRegistry.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

TypeObjectRegistry::~TypeObjectRegistry()
{
}

void TypeObjectRegistry::register_type_object_info(
        const std::string& type_name,
        const TypeObjectInfo& type_object_info)
{
    static_cast<void>(type_name);
    static_cast<void>(type_object_info);
}

TypeObjectInfo TypeObjectRegistry::get_type_object_info(
        const std::string& type_name)
{
    static_cast<void>(type_name);
    TypeObjectInfo type_object_info;
    return type_object_info;
}

} // xtypes
} // dds
} // fastdds
} // eprosima

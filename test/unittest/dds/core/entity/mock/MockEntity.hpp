// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef MOCK_ENTITY_H
#define MOCK_ENTITY_H

#include <fastdds/dds/core/Entity.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class MockEntity : public Entity
{
public:

    void mock_set_instance_handle(
            const InstanceHandle_t& handle)
    {
        set_instance_handle(handle);
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //MOCK_ENTITY_H

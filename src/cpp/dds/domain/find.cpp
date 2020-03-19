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

/**
 * @file find.cpp
 *
 */
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <dds/domain/find.hpp>

namespace dds {
namespace domain {

DomainParticipant find(
        uint32_t id)
{
    return dds::core::smart_ptr_traits<detail::DomainParticipant>::ref_type(eprosima::fastdds::dds::
                   DomainParticipantFactory::get_instance()
                   ->lookup_participant(id));
}

} // namespace domain
} // namespace dds


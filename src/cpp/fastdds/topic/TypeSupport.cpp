
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
 * @file TypeSupport.cpp
 */

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

using namespace eprosima::fastdds::dds;

ReturnCode_t TypeSupport::register_type(
        DomainParticipant* participant,
        std::string type_name) const
{
    return participant->register_type(*this, type_name.empty() ? get_type_name() : type_name);
}

ReturnCode_t TypeSupport::register_type(
        DomainParticipant* participant) const
{
    return participant->register_type(*this, get_type_name());
}

// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

ReturnCode_t ServiceTypeSupport::register_service_type(
        DomainParticipant* participant,
        std::string service_type_name) const
{
    return participant->register_service_type(*this, service_type_name);
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
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

/**
 * @file DomainParticipantFactory.hpp
 *
 */

#ifndef FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANTFACTORY_HPP
#define FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANTFACTORY_HPP

#include <memory>

#include <rtps/RTPSDomainImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipantImpl;

class DomainParticipantFactory
{
public:

    static DomainParticipantFactory* get_instance()
    {
        static std::shared_ptr<DomainParticipantFactory> instance(new DomainParticipantFactory());
        return instance.get();
    }

    xtypes::ITypeObjectRegistry& type_object_registry()
    {
        return rtps_domain_->type_object_registry();
    }

    DomainParticipantFactory()
        : rtps_domain_(fastdds::rtps::RTPSDomainImpl::get_instance())
    {
    }

    void participant_has_been_deleted(
            DomainParticipantImpl* part)
    {
        static_cast<void>(part);
    }

private:

    std::shared_ptr<fastdds::rtps::RTPSDomainImpl> rtps_domain_;

};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANTFACTORY_HPP

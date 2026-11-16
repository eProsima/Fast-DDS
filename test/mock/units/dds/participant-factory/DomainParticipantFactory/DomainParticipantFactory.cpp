// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of DomainParticipantFactory: replaces src/cpp/fastdds/domain/DomainParticipantFactory.cpp.
// It only implements the singleton access and the TypeObjectRegistry access, which forwards to the RTPSDomainImpl.

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <rtps/domain/RTPSDomainImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

DomainParticipantFactory::DomainParticipantFactory()
    : default_xml_profiles_loaded(false)
    , default_domain_id_(0)
    , rtps_domain_(rtps::RTPSDomainImpl::get_instance())
{
}

DomainParticipantFactory::~DomainParticipantFactory()
{
}

DomainParticipantFactory* DomainParticipantFactory::get_instance()
{
    return get_shared_instance().get();
}

std::shared_ptr<DomainParticipantFactory> DomainParticipantFactory::get_shared_instance()
{
    // Note we need a custom deleter, since the destructor is protected.
    static std::shared_ptr<DomainParticipantFactory> instance(new DomainParticipantFactory(),
            [](DomainParticipantFactory* p)
            {
                delete p;
            });
    return instance;
}

xtypes::ITypeObjectRegistry& DomainParticipantFactory::type_object_registry()
{
    return rtps_domain_->type_object_registry();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

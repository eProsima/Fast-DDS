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

// Link-seam implementation of XMLProfileManager: replaces src/cpp/xmlparser/XMLProfileManager.cpp and forwards the
// mocked static functions to the XMLProfileManagerMock instance, if any. The library settings are stored as the real
// implementation does, and no transport nor dynamic type is ever registered.

#include <xmlparser/XMLProfileManager.h>

#include <mock/xmlparser/XMLProfileManagerMock.hpp>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

XMLProfileManagerMock*& XMLProfileManagerMock::instance()
{
    static XMLProfileManagerMock* instance = nullptr;
    return instance;
}

fastdds::LibrarySettings XMLProfileManager::library_settings_;

const fastdds::LibrarySettings& XMLProfileManager::library_settings()
{
    return library_settings_;
}

void XMLProfileManager::library_settings(
        const fastdds::LibrarySettings& library_settings)
{
    library_settings_ = library_settings;
}

sp_transport_t XMLProfileManager::getTransportById(
        const std::string& transport_id)
{
    if (nullptr != XMLProfileManagerMock::instance())
    {
        return XMLProfileManagerMock::instance()->getTransportById(transport_id);
    }
    return sp_transport_t();
}

bool XMLProfileManager::insertTransportById(
        const std::string&,
        sp_transport_t)
{
    return false;
}

bool XMLProfileManager::insertDynamicTypeBuilderByName(
        const std::string&,
        const eprosima::fastdds::dds::DynamicTypeBuilder::_ref_type&)
{
    return false;
}

XMLP_ret XMLProfileManager::getDynamicTypeBuilderByName(
        eprosima::fastdds::dds::DynamicTypeBuilder::_ref_type&,
        const std::string&)
{
    return XMLP_ret::XML_ERROR;
}

} // namespace xmlparser
} // namespace fastdds
} // namespace eprosima

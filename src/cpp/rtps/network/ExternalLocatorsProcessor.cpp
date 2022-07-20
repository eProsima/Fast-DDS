// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ExternalLocatorsProcessor.cpp
 */

#include <rtps/network/ExternalLocatorsProcessor.hpp>

#include <fastdds/rtps/attributes/ExternalLocators.hpp>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ExternalLocatorsProcessor {

void set_listening_locators(
        ExternalLocators& external_locators,
        const LocatorList& listening_locators)
{
    static_cast<void>(external_locators);
    static_cast<void>(listening_locators);
}

void add_external_locators(
        ParticipantProxyData& data,
        const ExternalLocators& metatraffic_external_locators,
        const ExternalLocators& default_external_locators)
{
    static_cast<void>(data);
    static_cast<void>(metatraffic_external_locators);
    static_cast<void>(default_external_locators);
}

void add_external_locators(
        WriterProxyData& data,
        const ExternalLocators& external_locators)
{
    static_cast<void>(data);
    static_cast<void>(external_locators);
}

void add_external_locators(
        ReaderProxyData& data,
        const ExternalLocators& external_locators)
{
    static_cast<void>(data);
    static_cast<void>(external_locators);
}

void filter_remote_locators(
        ParticipantProxyData& data,
        const ExternalLocators& metatraffic_external_locators,
        const ExternalLocators& default_external_locators,
        bool ignore_non_matching)
{
    static_cast<void>(data);
    static_cast<void>(metatraffic_external_locators);
    static_cast<void>(default_external_locators);
    static_cast<void>(ignore_non_matching);
}

void filter_remote_locators(
        LocatorSelectorEntry& locators,
        const ExternalLocators& external_locators,
        bool ignore_non_matching)
{
    static_cast<void>(locators);
    static_cast<void>(external_locators);
    static_cast<void>(ignore_non_matching);
}

} // namespace ExternalLocatorsProcessor
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

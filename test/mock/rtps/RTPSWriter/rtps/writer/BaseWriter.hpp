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
 * @file BaseWriter.hpp
 */

#ifndef RTPS_WRITER__BASEWRITER_HPP
#define RTPS_WRITER__BASEWRITER_HPP

#include <gmock/gmock.h>

#include <fastdds/rtps/writer/LocatorSelectorSender.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseWriter : public RTPSWriter
{

public:

    BaseWriter()
        : general_locator_selector_(*this, ResourceLimitedContainerConfig())
        , async_locator_selector_(*this, ResourceLimitedContainerConfig())
    {
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD4(deliver_sample_nts, DeliveryRetCode(
            CacheChange_t*,
            RTPSMessageGroup&,
            LocatorSelectorSender&,
            const std::chrono::time_point<std::chrono::steady_clock>&));

    MOCK_METHOD4(send_nts, bool(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>&,
            const uint32_t&,
            const LocatorSelectorSender&,
            std::chrono::steady_clock::time_point&));

    // *INDENT-ON*

    LocatorSelectorSender& get_general_locator_selector()
    {
        return general_locator_selector_;
    }

    LocatorSelectorSender& get_async_locator_selector()
    {
        return async_locator_selector_;
    }

    LocatorSelectorSender general_locator_selector_;

    LocatorSelectorSender async_locator_selector_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_WRITER__BASEWRITER_HPP

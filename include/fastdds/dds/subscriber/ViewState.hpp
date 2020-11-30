// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ViewState.hpp
 */

#ifndef _FASTDDS_DDS_SUBSCRIBER_VIEWSTATE_HPP_
#define _FASTDDS_DDS_SUBSCRIBER_VIEWSTATE_HPP_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

enum DDSViewStateKind : uint16_t
{
    NEW_VIEW_STATE     = 0x0001 << 0,
    NOT_NEW_VIEW_STATE = 0x0001 << 1,
};

using DDSViewStateMask = uint16_t;

constexpr DDSViewStateMask ANY_VIEW_STATE = NEW_VIEW_STATE | NOT_NEW_VIEW_STATE;

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_DDS_SUBSCRIBER_VIEWSTATE_HPP_

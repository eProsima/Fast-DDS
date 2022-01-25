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
 * @file DDSFilterParameter.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERPARAMETER_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERPARAMETER_HPP_

#include "DDSFilterValue.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

struct DDSFilterParameter final : public DDSFilterValue
{
    virtual ~DDSFilterParameter() = default;

    bool has_value() const noexcept final;

    void reset() noexcept final;

    size_t get_index() const noexcept;

    void set_value(
            const char* parameter);
};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERPARAMETER_HPP_

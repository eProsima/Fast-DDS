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

/**
 * A DDSFilterValue for expression parameters (i.e. %nn).
 */
class DDSFilterParameter final : public DDSFilterValue
{

public:

    virtual ~DDSFilterParameter() = default;

    /**
     * Sets the value of this DDSFilterParameter given from an input string.
     *
     * @param [in] parameter  The string from which to set the value.
     *
     * @return whether the parsing of the string correspond to a valid literal value.
     */
    bool set_value(
            const char* parameter);
};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERPARAMETER_HPP_

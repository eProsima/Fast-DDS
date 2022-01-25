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
 * @file DDSFilterValue.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERVALUE_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERVALUE_HPP_

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

struct DDSFilterValue
{
	virtual ~DDSFilterValue() = default;

	virtual bool has_value() const noexcept;

	virtual void reset() noexcept;
};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERVALUE_HPP_

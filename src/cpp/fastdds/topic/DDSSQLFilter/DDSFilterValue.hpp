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

#include <fastrtps/utils/fixed_size_string.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

struct DDSFilterValue
{
	enum class ValueKind
	{
		BOOLEAN,
		CHAR,
		SIGNED_INTEGER,
		UNSIGNED_INTEGER,
		FLOAT,
		STRING,
		ENUM
	};

	ValueKind kind;
	union
	{
		bool boolean_value;
		char char_value;
		int64_t signed_integer_value;
		uint64_t unsigned_integer_value;
		long double float_value;
		eprosima::fastrtps::string_255 string_value;
	};

	DDSFilterValue()
		: kind(ValueKind::STRING)
		, string_value()
	{
	}

	explicit DDSFilterValue(
			ValueKind data_kind)
		: kind(data_kind)
		, string_value()
	{
	}

	virtual ~DDSFilterValue() = default;

	virtual bool has_value() const noexcept
	{
		return true;
	}

	virtual void reset() noexcept
	{
	}
};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERVALUE_HPP_

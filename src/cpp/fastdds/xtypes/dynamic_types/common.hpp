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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_COMMON_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_COMMON_HPP

#include <fastcdr/CdrEncoding.hpp>

#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

inline eprosima::fastcdr::EncodingAlgorithmFlag get_fastcdr_encoding_flag(
        ExtensibilityKind ext_kind,
        eprosima::fastcdr::CdrVersion cdr_version)
{
    eprosima::fastcdr::EncodingAlgorithmFlag ret_value {eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR};

    if (eprosima::fastcdr::CdrVersion::XCDRv2 == cdr_version)
    {
        switch (ext_kind)
        {
            case ExtensibilityKind::MUTABLE:
                ret_value = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2;
                break;
            case ExtensibilityKind::APPENDABLE:
                ret_value = eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2;
                break;
            case ExtensibilityKind::FINAL:
                ret_value = eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2;
                break;
        }
    }
    else
    {
        if (ExtensibilityKind::MUTABLE == ext_kind)
        {
            ret_value = eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR;
        }
    }

    return ret_value;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_COMMON_HPP

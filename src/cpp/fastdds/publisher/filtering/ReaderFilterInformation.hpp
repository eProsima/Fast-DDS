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
 * @file ReaderFilterInformation.hpp
 */

#ifndef _FASTDDS_PUBLISHER_FILTERING_READERFILTERINFORMATION_HPP_
#define _FASTDDS_PUBLISHER_FILTERING_READERFILTERINFORMATION_HPP_

#include <array>
#include <cstdint>

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

struct ReaderFilterInformation
{
    fastcdr::string_255 filter_class_name;
    IContentFilterFactory* filter_factory = nullptr;
    IContentFilter* filter = nullptr;
    std::array<uint8_t, 16> filter_signature{ { 0 } };
};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  //_FASTDDS_PUBLISHER_FILTERING_READERFILTERINFORMATION_HPP_

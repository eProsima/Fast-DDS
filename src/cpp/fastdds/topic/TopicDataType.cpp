// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TopicDataType.cpp
 */

#include <functional>
#include <memory>
#include <string>

#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/utils/md5.h>

namespace eprosima {
namespace fastdds {
namespace dds {

TopicDataType::TopicDataType()
    : m_typeSize(0)
    , m_isGetKeyDefined(false)
    , auto_fill_type_object_(true)
    , auto_fill_type_information_(true)
{
}

TopicDataType::~TopicDataType()
{
}

bool TopicDataType::serialize(
        void* data,
        fastrtps::rtps::SerializedPayload_t* payload,
        DataRepresentationId_t data_representation)
{

    static_cast<void>(data_representation);
    return serialize(data, payload);
}

std::function<uint32_t()> TopicDataType::getSerializedSizeProvider(
        void* data,
        DataRepresentationId_t data_representation)
{
    static_cast<void>(data);
    static_cast<void>(data_representation);
    return []()
           {
               return 0;
           };
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

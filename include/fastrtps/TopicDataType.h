// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TopicDataType.h
 */

#ifndef TOPICDATATYPE_H_
#define TOPICDATATYPE_H_

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastrtps/utils/md5.h>

namespace eprosima {
namespace fastrtps {

// Adding an alias to fastrtps namespace for legacy usage.
using TopicDataType = fastdds::dds::TopicDataType;

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* TOPICDATATYPE_H_ */

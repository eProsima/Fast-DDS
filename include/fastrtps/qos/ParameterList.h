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
 * @file ParameterList.h
 */

#ifndef PARAM_LIST_T_H_
#define PARAM_LIST_T_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/dds/core/policy/ParameterList.hpp>

#include <fastrtps/qos/ParameterTypes.h>
#include <fastdds/rtps/messages/RTPS_messages.h>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/messages/CDRMessage.h>
#include <fastdds/rtps/common/CacheChange.h>

#include <functional>

namespace eprosima {
namespace fastrtps {

using ParameterList = fastdds::dds::ParameterList;

} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* PARAMLISTT_H_ */

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
 * @file WriterQos.h
 *
 */

#ifndef WRITERQOS_H_
#define WRITERQOS_H_

#include <fastdds/dds/publisher/qos/WriterQos.hpp>

#include <fastrtps/qos/QosPolicies.h>  // Needed for old enum constant values

namespace eprosima {
namespace fastrtps {

using WriterQos = fastdds::dds::WriterQos;

} /* namespace  */
} /* namespace eprosima */

#endif /* WRITERQOS_H_ */

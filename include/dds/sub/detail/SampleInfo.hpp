/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef EPROSIMA_DDS_SUB_DETAIL_SAMPLE_INFO_HPP_
#define EPROSIMA_DDS_SUB_DETAIL_SAMPLE_INFO_HPP_

<<<<<<< HEAD
//#include <dds/sub/detail/TSampleInfoImpl.hpp>
=======
#include <fastdds/dds/subscriber/SampleInfo.hpp>
>>>>>>> Feature/psm helloworld [6739] (#822)

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace sub {
namespace detail {

<<<<<<< HEAD
//TODO: Change when fastdds SampleInfo is implemented
class SampleInfo
{
};
=======
using SampleInfo = eprosima::fastdds::dds::SampleInfo_t;
>>>>>>> Feature/psm helloworld [6739] (#822)

} //namespace detail
} //namespace sub
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_SUB_DETAIL_SAMPLE_INFO_HPP_


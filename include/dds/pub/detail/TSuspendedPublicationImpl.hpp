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

#ifndef EPROSIMA_DDS_PUB_TSUSPENDEDPUBLICATION_IMPL_HPP_
#define EPROSIMA_DDS_PUB_TSUSPENDEDPUBLICATION_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/pub/SuspendedPublication.hpp>
//#include <org/opensplice/core/ReportUtils.hpp>

namespace dds {
namespace pub {

template<typename DELEGATE>
TSuspendedPublication<DELEGATE>::TSuspendedPublication(
        const Publisher& pub)
    : dds::core::Value<DELEGATE>(pub)
{
}

template<typename DELEGATE>
void TSuspendedPublication<DELEGATE>::resume()
{
    //To implement
//    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->get_publisher().delegate().get());
//    this->delegate().resume();
}

template<typename DELEGATE>
TSuspendedPublication<DELEGATE>::~TSuspendedPublication()
{
    //To implement
//    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->get_publisher().delegate().get());
//    try {
//        this->delegate().resume();
//    } catch (...) {
//        /* Empty: the exception throw should have already traced an error. */
//    }
}

} //namespace pub
} //namespace dds

#endif //EPROSIMA_DDS_PUB_TSUSPENDEDPUBLICATION_IMPL_HPP_

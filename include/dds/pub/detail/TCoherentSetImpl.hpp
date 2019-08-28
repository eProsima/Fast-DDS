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

#ifndef EPROSIMA_DDS_PUB_TCOHERENTSET_IMPL_HPP_
#define EPROSIMA_DDS_PUB_TCOHERENTSET_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/pub/CoherentSet.hpp>
//#include <org/opensplice/core/ReportUtils.hpp>


namespace dds {
namespace pub {

template<typename DELEGATE>
TCoherentSet<DELEGATE>::TCoherentSet(
        const dds::pub::Publisher& pub)
    : dds::core::Value<DELEGATE>(pub)
{
}

template<typename DELEGATE>
void TCoherentSet<DELEGATE>::end()
{
    //To implement
}

template<typename DELEGATE>
TCoherentSet<DELEGATE>::~TCoherentSet()
{
    //To implement
}

} //namespace pub
} //namespace dds

#endif //EPROSIMA_DDS_PUB_TCOHERENTSET_HPP_

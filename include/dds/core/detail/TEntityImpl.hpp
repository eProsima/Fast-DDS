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

#ifndef EPROSIMA_DDS_CORE_TENTITY_IMPL_HPP_
#define EPROSIMA_DDS_CORE_TENTITY_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/detail/ReferenceImpl.hpp>
#include <dds/core/Entity.hpp>
//#include <org/opensplice/core/ReportUtils.hpp>

namespace dds {
namespace core {

template<typename DELEGATE>
TEntity<DELEGATE>::~TEntity()
{
}

template<typename DELEGATE>
void TEntity<DELEGATE>::enable()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->enable();
}

template<typename DELEGATE>
const status::StatusMask TEntity<DELEGATE>::status_changes()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->status_changes();
}

template<typename DELEGATE>
const InstanceHandle TEntity<DELEGATE>::instance_handle() const
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->instance_handle();
}

template<typename DELEGATE>
void TEntity<DELEGATE>::close()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->close();
}

template<typename DELEGATE>
void TEntity<DELEGATE>::retain()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->retain();
}

} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_TENTITY_IMPL_HPP_

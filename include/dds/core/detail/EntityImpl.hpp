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

#ifndef EPROSIMA_DDS_CORE_ENTITY_IMPL_HPP_
#define EPROSIMA_DDS_CORE_ENTITY_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/Entity.hpp>
#include <dds/core/cond/StatusCondition.hpp>

namespace dds {
namespace core {

template<typename DELEGATE>
TEntity<DELEGATE>::~TEntity()
{
}

template<typename DELEGATE>
void TEntity<DELEGATE>::enable()
{
    this->delegate()->enable();
}

template<typename DELEGATE>
const dds::core::status::StatusMask TEntity<DELEGATE>::status_changes()
{
    return this->delegate()->get_status_changes();
}

template<typename DELEGATE>
const InstanceHandle TEntity<DELEGATE>::instance_handle() const
{
    return this->delegate()->get_instance_handle();
}

template<typename DELEGATE>
void TEntity<DELEGATE>::close()
{
    this->delegate()->close();
}

template<typename DELEGATE>
void TEntity<DELEGATE>::retain()
{
    // this->delegate()->retain();
}

} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_ENTITY_IMPL_HPP_

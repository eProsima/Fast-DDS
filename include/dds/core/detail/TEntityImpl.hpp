/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OSPL_DDS_CORE_TENTITY_IMPL_HPP_
#define OSPL_DDS_CORE_TENTITY_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/detail/ReferenceImpl.hpp>
#include <dds/core/TEntity.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

// Implementation

template <typename DELEGATE>
dds::core::TEntity<DELEGATE>::~TEntity()
{
}

template <typename DELEGATE>
void
dds::core::TEntity<DELEGATE>::enable()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->enable();
}

template <typename DELEGATE>
const dds::core::status::StatusMask
dds::core::TEntity<DELEGATE>::status_changes()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->status_changes();
}

template <typename DELEGATE>
const dds::core::InstanceHandle
dds::core::TEntity<DELEGATE>::instance_handle() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->instance_handle();
}

template <typename DELEGATE>
void
dds::core::TEntity<DELEGATE>::close()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->close();
}

template <typename DELEGATE>
void
dds::core::TEntity<DELEGATE>::retain()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->retain();
}

// End of implementation

#endif /* OSPL_DDS_CORE_TENTITY_IMPL_HPP_ */

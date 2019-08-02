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
#ifndef OSPL_DDS_CORE_COND_DETAIL_CONDITION_HPP_
#define OSPL_DDS_CORE_COND_DETAIL_CONDITION_HPP_

/**
 * @cond
 * Ignore this file in the API
 */

// Implementation

#include <dds/core/cond/detail/TConditionImpl.hpp>
#include <org/opensplice/core/cond/ConditionDelegate.hpp>

namespace dds
{
namespace core
{
namespace cond
{
namespace detail
{
typedef dds::core::cond::TCondition<org::opensplice::core::cond::ConditionDelegate> Condition;
}
}
}
}

// End of implementation

/** @endcond */

#endif /* OSPL_DDS_CORE_COND_DETAIL_CONDITION_HPP_ */

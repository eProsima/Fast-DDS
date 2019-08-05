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
#ifndef OSPL_DDS_CORE_COND_DETAIL_GUARDCONDITION_HPP_
#define OSPL_DDS_CORE_COND_DETAIL_GUARDCONDITION_HPP_

/**
 * @cond
 * Ignore this file in the API
 */

// Implementation

namespace org
{
namespace opensplice
{
namespace core
{
namespace cond
{
class GuardConditionDelegate;
}
}
}
}

namespace dds
{
namespace core
{
namespace cond
{

template <typename DELEGATE>
class TGuardCondition;

namespace detail
{
typedef dds::core::cond::TGuardCondition<org::opensplice::core::cond::GuardConditionDelegate> GuardCondition;
}
}
}
}

// End of implementation

/** @endcond */

#endif /* OSPL_DDS_CORE_COND_DETAIL_GUARDCONDITION_HPP_ */

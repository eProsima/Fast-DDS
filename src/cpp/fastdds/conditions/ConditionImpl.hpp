// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ConditionImpl.hpp
 *
 */

#ifndef _FASTDDS_CONDITIONIMPL_H_
#define _FASTDDS_CONDITIONIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

namespace eprosima {
namespace fastdds {

class Condition;

/**
 * Class ConditionImpl, contains the actual implementation of the behaviour of the Condition.
 *  @ingroup FASTDDS_MODULE
 */
class ConditionImpl 
{
public:

    bool get_trigger_value() const;
};


} // end of namespace fastdds
} // end of namespace eprosima
#endif
#endif // _FASTDDS_CONDITIONIMPL_H_ 

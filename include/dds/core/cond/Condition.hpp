/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * All rights reserved.
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
 */

#ifndef OMG_DDS_CORE_COND_CONDITION_HPP_
#define OMG_DDS_CORE_COND_CONDITION_HPP_

#include <dds/core/cond/detail/Condition.hpp>

#include <dds/core/Reference.hpp>
#include <dds/core/cond/detail/GuardCondition.hpp>
#include <dds/core/cond/detail/StatusCondition.hpp>
//#include <dds/sub/cond/detail/ReadCondition.hpp>
//#include <dds/sub/cond/detail/QueryCondition.hpp>

namespace dds {
namespace core {
namespace cond {


/**
 * @brief
 * This class is the base class for all the conditions that may be attached to a dds::core::cond::WaitSet.
 *
 * This base class is specialized in three classes by the Data Distribution Service:
 * - dds::core::cond::GuardCondition
 * - dds::core::cond::StatusCondition
 * - dds::sub::cond::ReadCondition
 *      - dds::sub::cond::QueryCondition
 *
 * Each Condition has a trigger_value that can be TRUE or FALSE and is set by
 * the Data Distribution Service (except a GuardCondition) depending on the
 * evaluation of the Condition.
 *
 * @see @ref DCPS_Modules_Infrastructure_Status  "Status concept"
 * @see @ref DCPS_Modules_Infrastructure_Waitset "WaitSet concept"
 */
template<typename DELEGATE>
class TCondition : public virtual Reference<DELEGATE>
{
public:

    OMG_DDS_REF_TYPE_DELEGATE_C(
        TCondition,
        dds::core::Reference,
        DELEGATE)


    OMG_DDS_EXPLICIT_REF_BASE_DECL(
        TCondition,
        detail::StatusCondition)

    OMG_DDS_EXPLICIT_REF_BASE_DECL(
        TCondition,
        detail::GuardCondition)

/*
    OMG_DDS_EXPLICIT_REF_BASE_DECL(
        TCondition,
        dds::sub::cond::detail::ReadCondition)

    OMG_DDS_EXPLICIT_REF_BASE_DECL(
        TCondition,
        dds::sub::cond::detail::QueryCondition)
*/

    /** @cond */
    OMG_DDS_API ~TCondition()
    {
    }
    /** @endcond */

    /**
     * Registers a functor as custom handler with this Condition.
     *
     * The supplied functor will be called when this Condition is triggered
     * and either the dds::core::cond::Condition::dispatch() is called or the
     * dds::core::cond::WaitSet::dispatch() on the WaitSet to which this
     * Condition is attached to.
     *
     * @tparam Functor The functor to be called when the StatusCondition triggers.
     * @return void
     * @throw  dds::core::Exception
     */
    template<typename Functor>
    OMG_DDS_API void handler(
            Functor& func)
    {
        //To implement
        //    this->delegate()->set_handler(func);

    }

    /** @copydoc dds::core::cond::TCondition::handler(Functor& func) */
    template<typename Functor>
    OMG_DDS_API void handler(
            const Functor& func)
    {
        //To implement
        //        this->delegate()->set_handler(func);
    }

    /**
     * Resets the handler for this Condition.
     *
     * After the invocation of this function no handler will be registered with
     * this Condition.
     *
     * @return void
     * @throw  dds::core::Exception
     */
    OMG_DDS_API void reset_handler()
    {
        //To implement
        //    this->delegate()->reset_handler();
    }

    /**
     * Dispatches the functor that have been registered with the Condition.
     *
     * The Condition has to have been triggered for the functor will be called
     * by this function.
     *
     * @return void
     * @throw  dds::core::Exception
     */
    OMG_DDS_API void dispatch()
    {
        //To implement
        //    this->delegate()->dispatch();
    }

    /**
     * This operation retrieves the trigger_value of the Condition.
     *
     * A Condition has a trigger_value that can be TRUE or FALSE and is set by the
     * Data Distribution Service (except a GuardCondition). This operation returns the
     * trigger_value of the Condition.
     *
     * @return bool The boolean value to which the Condition is set.
     * @throw  dds::core::Exception
     */
    OMG_DDS_API bool trigger_value() const
    {
        return this->delegate()->get_trigger_value();
    }

};


typedef TCondition<detail::Condition> Condition;

} //namespace cond
} //namespace core
} //namespace dds

#endif  //OMG_DDS_CORE_COND_CONDITION_HPP_

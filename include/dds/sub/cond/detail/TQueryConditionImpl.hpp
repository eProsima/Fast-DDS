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

#ifndef EPROSIMA_DDS_SUB_COND_TQUERYCONDITION_IMPL_HPP_
#define EPROSIMA_DDS_SUB_COND_TQUERYCONDITION_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/cond/QueryCondition.hpp>
//#include <org/opensplice/sub/cond/QueryConditionDelegate.hpp>

namespace dds {
namespace sub {
namespace cond {

template<typename DELEGATE>
TQueryCondition<DELEGATE>::TQueryCondition(
        const dds::sub::Query& query,
        const dds::sub::status::DataState& status)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(query);

//    this->set_ref(new DELEGATE(query.data_reader(),
//                               query.expression(), query.delegate()->parameters(), status));
//    this->delegate()->init(this->impl_);
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template<typename DELEGATE>
template<typename FUN>
TQueryCondition<DELEGATE>::TQueryCondition(
        const dds::sub::Query& query,
        const dds::sub::status::DataState& status, FUN& functor)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(query);

//    this->set_ref(new DELEGATE(query.data_reader(),
//                               query.expression(), query.delegate()->parameters(), status));
//    this->delegate()->set_handler(functor);
//	this->delegate()->init(this->impl_);
}

template<typename DELEGATE>
template<typename FUN>
TQueryCondition<DELEGATE>::TQueryCondition(
        const dds::sub::Query& query,
        const dds::sub::status::DataState& status, const FUN& functor)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(query);

//    this->set_ref(new DELEGATE(query.data_reader(),
//                               query.expression(), query.delegate()->parameters(), status));
//    this->delegate()->set_handler(functor);
//    this->delegate()->init(this->impl_);
}
/** @endcond */

template<typename DELEGATE>
TQueryCondition<DELEGATE>::TQueryCondition(
        const dds::sub::AnyDataReader& dr,
        const std::string& expression,
        const std::vector<std::string>& params,
        const dds::sub::status::DataState& status)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dr);

//    this->set_ref(new DELEGATE(dr, expression, params, status));
//	this->delegate()->init(this->impl_);
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template<typename DELEGATE>
template<typename FUN>
TQueryCondition<DELEGATE>::TQueryCondition(
        const dds::sub::AnyDataReader& dr,
        const std::string& expression,
        const std::vector<std::string>& params,
        const dds::sub::status::DataState& status,
        FUN& functor)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dr);

//    this->set_ref(new DELEGATE(dr, expression, params, status));
//    this->delegate()->set_handler(functor);
//	this->delegate()->init(this->impl_);
}

template<typename DELEGATE>
template<typename FUN>
TQueryCondition<DELEGATE>::TQueryCondition(
        const dds::sub::AnyDataReader& dr,
        const std::string& expression,
        const std::vector<std::string>& params,
        const dds::sub::status::DataState& status,
        const FUN& functor)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dr);

//    this->set_ref(new DELEGATE(dr, expression, params, status));
//    this->delegate()->set_handler(functor);
//    this->delegate()->init(this->impl_);
}
/** @endcond */

template<typename DELEGATE>
TQueryCondition<DELEGATE>::~TQueryCondition()
{
}

template<typename DELEGATE>
template<typename FWIterator>
void TQueryCondition<DELEGATE>::parameters(
        const FWIterator& begin,
        const FWIterator end)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

//    std::vector<std::string> params(begin, end);
//    this->delegate()->parameters(params);
}

template<typename DELEGATE>
void TQueryCondition<DELEGATE>::expression(
    const std::string& expr)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->expression(expr);
}

template<typename DELEGATE>
const std::string& TQueryCondition<DELEGATE>::expression()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->expression();

}

template<typename DELEGATE>
typename TQueryCondition<DELEGATE>::const_iterator TQueryCondition<DELEGATE>::begin() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->begin();
}

template<typename DELEGATE>
typename TQueryCondition<DELEGATE>::const_iterator TQueryCondition<DELEGATE>::end() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->end();
}

template<typename DELEGATE>
typename TQueryCondition<DELEGATE>::iterator TQueryCondition<DELEGATE>::begin()
{
    //To implement;
//    return this->delegate()->begin();
}

template<typename DELEGATE>
typename TQueryCondition<DELEGATE>::iterator TQueryCondition<DELEGATE>::end()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->end();
}

template<typename DELEGATE>
void TQueryCondition<DELEGATE>::add_parameter(
    const std::string& param)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->add_parameter(param);
}

template<typename DELEGATE>
uint32_t TQueryCondition<DELEGATE>::parameters_length() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->parameters_length();
}

template<typename DELEGATE>
const AnyDataReader& TQueryCondition<DELEGATE>::data_reader() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->data_reader();
}

} //namespace cond
} //namespace sub

namespace core {
namespace cond {

template<typename DELEGATE>
TCondition<DELEGATE>::TCondition(
        /*const dds::sub::cond::TQueryCondition<org::opensplice::sub::cond::QueryConditionDelegate>& h*/)
{
    //To implement
//    if (h.is_nil()) {
//        /* We got a null object and are not really able to do a typecheck here. */
//        /* So, just set a null object. */
//        *this = dds::core::null;
//    } else {
//        this->::dds::core::Reference<DELEGATE>::impl_ = OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DELEGATE_T>(h.delegate());
//        if (h.delegate() != this->::dds::core::Reference<DELEGATE>::impl_) {
//            throw dds::core::IllegalOperationError(std::string("Attempted invalid cast: ") + typeid(h).name() + " to " + typeid(*this).name());
//        }
//    }
}

template<typename DELEGATE>
TCondition<DELEGATE>& TCondition<DELEGATE>::operator=(
        /*const dds::sub::cond::TQueryCondition<org::opensplice::sub::cond::QueryConditionDelegate>& rhs*/)
{
    //To implement
//    if (this != (TCondition*)&rhs) {
//        if (rhs.is_nil()) {
//            /* We got a null object and are not really able to do a typecheck here. */
//            /* So, just set a null object. */
//            *this = dds::core::null;
//        } else {
//            TCondition other(rhs);
//            /* Dont have to copy when the delegate is the same. */
//            if (other.delegate() != this->::dds::core::Reference<DELEGATE>::impl_) {
//                *this = other;
//            }
//        }
//    }
//    return *this;
}

} //namespace cond
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_SUB_COND_TQUERYCONDITION_IMPL_HPP_

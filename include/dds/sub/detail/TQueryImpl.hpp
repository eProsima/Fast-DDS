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
#ifndef OSPL_DDS_SUB_DETAIL_QUERY_IMPL_HPP_
#define OSPL_DDS_SUB_DETAIL_QUERY_IMPL_HPP_

/**
 * @file
 */

// Implementation
#include <dds/sub/TQuery.hpp>

template <typename DELEGATE>
dds::sub::TQuery<DELEGATE>::TQuery(
    const dds::sub::AnyDataReader& dr,
    const std::string& expression) :
        dds::core::Reference<DELEGATE>(new DELEGATE(AnyDataReader(dr), expression))
{
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template <typename DELEGATE>
template<typename FWIterator>
dds::sub::TQuery<DELEGATE>::TQuery(
    const dds::sub::AnyDataReader& dr,
    const std::string& expression,
    const FWIterator& params_begin,
    const FWIterator& params_end) :
        dds::core::Reference<DELEGATE>(new DELEGATE(AnyDataReader(dr), expression))
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(dr);

    for (FWIterator it = params_begin; it != params_end; ++it) {
        add_parameter(*it);
    }
}
/** @endcond */

template <typename DELEGATE>
dds::sub::TQuery<DELEGATE>::TQuery(
    const dds::sub::AnyDataReader& dr,
    const std::string& expression,
    const std::vector<std::string>& params) :
        dds::core::Reference<DELEGATE>(new DELEGATE(AnyDataReader(dr), expression, params))
{

}

template <typename DELEGATE>
const std::string& dds::sub::TQuery<DELEGATE>::expression() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->expression();
}

template <typename DELEGATE>
void dds::sub::TQuery<DELEGATE>::expression(const std::string& expr)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    this->delegate()->expression(expr);
}

/**
 * Provides the begin iterator to the parameter list.
 */
template <typename DELEGATE>
typename dds::sub::TQuery<DELEGATE>::const_iterator dds::sub::TQuery<DELEGATE>::begin() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->begin();
}

/**
 * The end iterator to the parameter list.
 */
template <typename DELEGATE>
typename dds::sub::TQuery<DELEGATE>::const_iterator dds::sub::TQuery<DELEGATE>::end() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->end();
}

/**
 * Provides the begin iterator to the parameter list.
 */
template <typename DELEGATE>
typename dds::sub::TQuery<DELEGATE>::iterator dds::sub::TQuery<DELEGATE>::begin()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->begin();
}

/**
 * The end iterator to the parameter list.
 */
template <typename DELEGATE>
typename dds::sub::TQuery<DELEGATE>::iterator dds::sub::TQuery<DELEGATE>::end()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->end();
}

template <typename DELEGATE>
template<typename FWIterator>
void dds::sub::TQuery<DELEGATE>::parameters(
    const FWIterator& begin,
    const FWIterator end)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    for (FWIterator it = begin; it != end; ++it) {
        add_parameter(*it);
    }
}

template <typename DELEGATE>
void dds::sub::TQuery<DELEGATE>::add_parameter(
    const std::string& param)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    this->delegate()->add_parameter(param);
}

template <typename DELEGATE>
uint32_t dds::sub::TQuery<DELEGATE>::parameters_length() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->parameters_length();
}

template <typename DELEGATE>
const dds::sub::AnyDataReader&
dds::sub::TQuery<DELEGATE>::data_reader() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->data_reader();
}




#endif /* OSPL_DDS_SUB_DETAIL_QUERY_IMPL_HPP_ */

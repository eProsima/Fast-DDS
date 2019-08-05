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
#ifndef OSPL_DDS_SUB_COND_QUERYCONDITION_IMPL_HPP_
#define OSPL_DDS_SUB_COND_QUERYCONDITION_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/cond/QueryCondition.hpp>

// Implementation

namespace dds
{
namespace sub
{
namespace cond
{

template <typename FUN>
QueryCondition::QueryCondition(
    const dds::sub::Query& query,
    const dds::sub::status::DataState& status, const FUN& functor)
    : dds::sub::cond::TReadCondition<DELEGATE>(new DELEGATE(query.delegate().data_reader(), query.delegate(), status, functor))
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(query);

	this->delegate()->init(this->impl_);
}

template <typename T>
QueryCondition::QueryCondition(
    const dds::sub::DataReader<T>& dr,
    const std::string& expression,
    const std::vector<std::string>& params,
    const dds::sub::status::DataState& status)
    : dds::sub::cond::TReadCondition<DELEGATE>(new DELEGATE(dds::sub::AnyDataReader(dr), expression, params, status))
{
	ISOCPP_REPORT_STACK_DDS_BEGIN(dr);

	this->delegate()->init(this->impl_);
}


template <typename T, typename FUN>
QueryCondition::QueryCondition(const dds::sub::DataReader<T>& dr,
    const std::string& expression,
    const std::vector<std::string>& params,
    const dds::sub::status::DataState& status,
    const FUN& functor)
    : dds::sub::cond::TReadCondition<DELEGATE>(new DELEGATE(dds::sub::AnyDataReader(dr), expression, params, status, functor))
{
	ISOCPP_REPORT_STACK_DDS_BEGIN(dr);

	this->delegate()->init(this->impl_);
}

template<typename FWIterator>
void QueryCondition::parameters(const FWIterator& begin, const FWIterator end)
{
	ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    std::vector<std::string> params(begin, end);
    this->delegate()->parameters(params);
}

}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_COND_QUERYCONDITION_IMPL_HPP_ */

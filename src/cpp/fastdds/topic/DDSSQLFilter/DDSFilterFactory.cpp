// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DDSFilterFactory.cpp
 */

#include "DDSFilterFactory.hpp"

#include <cstring>

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include <fastrtps/types/TypeObjectFactory.h>

#include "DDSFilterGrammar.hpp"
#include "DDSFilterExpressionParser.hpp"

#include "DDSFilterExpression.hpp"
#include "DDSFilterCompoundCondition.hpp"
#include "DDSFilterCondition.hpp"
#include "DDSFilterConditionState.hpp"
#include "DDSFilterEmptyExpression.hpp"
#include "DDSFilterExpression.hpp"
#include "DDSFilterField.hpp"
#include "DDSFilterParameter.hpp"
#include "DDSFilterPredicate.hpp"
#include "DDSFilterValue.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

DDSFilterFactory::~DDSFilterFactory()
{
    auto& pool = expression_pool_.collection();
    for (DDSFilterExpression* item : pool)
    {
        delete item;
    }
    pool.clear();
}

IContentFilterFactory::ReturnCode_t DDSFilterFactory::create_content_filter(
        const char* filter_class_name,
        const char* type_name,
        const TopicDataType* data_type,
        const char* filter_expression,
        const IContentFilterFactory::ParameterSeq& filter_parameters,
        IContentFilter*& filter_instance)
{
    using eprosima::fastrtps::types::TypeObjectFactory;

    static_cast<void>(filter_class_name);
    static_cast<void>(type_name);
    static_cast<void>(data_type);
    static_cast<void>(filter_expression);
    static_cast<void>(filter_parameters);
    static_cast<void>(filter_instance);

    ReturnCode_t ret = ReturnCode_t::RETCODE_UNSUPPORTED;

    if ((filter_expression == nullptr) || (std::strlen(filter_expression) == 0))
    {
        filter_instance = &empty_expression_;
        ret = ReturnCode_t::RETCODE_OK;
    }
    else
    {
        auto type_object = TypeObjectFactory::get_instance()->get_type_object(type_name, true);
        if (!type_object)
        {
            logError(DDSSQLFILTER, "No TypeObject found for type " << type_name);
            ret = ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        else
        {
            auto node = parser::parse_filter_expression(filter_expression, type_object);
            if (node)
            {
            }
            else
            {
                ret = ReturnCode_t::RETCODE_BAD_PARAMETER;
            }
        }
    }

    return ret;
}

IContentFilterFactory::ReturnCode_t DDSFilterFactory::delete_content_filter(
        const char* filter_class_name,
        IContentFilter* filter_instance)
{
    static_cast<void>(filter_class_name);
    static_cast<void>(filter_instance);

    if (&empty_expression_ != filter_instance)
    {
        expression_pool_.put(static_cast<DDSFilterExpression*>(filter_instance));
    }
    return ReturnCode_t::RETCODE_OK;
}

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

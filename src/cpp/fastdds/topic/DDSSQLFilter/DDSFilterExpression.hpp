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
 * @file DDSFilterExpression.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTEREXPRESSION_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTEREXPRESSION_HPP_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastcdr/Cdr.h>

#include "DDSFilterCondition.hpp"

#include "DDSFilterField.hpp"
#include "DDSFilterParameter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

/**
 * An IContentFilter that evaluates DDS-SQL filter expressions
 */
struct DDSFilterExpression final : public IContentFilter
{
    bool evaluate(
            const SerializedPayload& payload,
            const FilterSampleInfo& sample_info,
            const GUID_t& reader_guid) const final
    {
        static_cast<void>(sample_info);
        static_cast<void>(reader_guid);

        using namespace eprosima::fastrtps::types;
        using namespace eprosima::fastcdr;

        dyn_data_->clear_all_values();
        try
        {
            FastBuffer fastbuffer((char* const)(payload.data), payload.length);
            Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            deser.read_encapsulation();
            dyn_data_->deserialize(deser);
        }
        catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        root->reset();
        for (auto it = fields.begin();
                it != fields.end() && DDSFilterConditionState::UNDECIDED == root->get_state();
                ++it)
        {
            if (!it->second->set_value(*dyn_data_))
            {
                return false;
            }
        }

        return DDSFilterConditionState::RESULT_TRUE == root->get_state();
    }

    /**
     * Clear the information held by this object.
     */
    void clear()
    {
        dyn_data_.reset();
        dyn_type_.reset();
        parameters.clear();
        fields.clear();
        root.reset();
    }

    void set_type(
            const eprosima::fastrtps::types::DynamicType_ptr& type)
    {
        dyn_type_ = type;
        dyn_data_.reset(eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(type));
    }

    /// The root condition of the expression tree.
    std::unique_ptr<DDSFilterCondition> root;
    /// The fields referenced by this expression.
    std::map<std::string, std::shared_ptr<DDSFilterField>> fields;
    /// The parameters referenced by this expression.
    std::vector<std::shared_ptr<DDSFilterParameter>> parameters;

private:

    struct DynDataDeleter
    {
        void operator ()(eprosima::fastrtps::types::DynamicData* ptr)
        {
            eprosima::fastrtps::types::DynamicDataFactory::get_instance()->delete_data(ptr);
        }
    };

    /// The Dynamic type used to deserialize the payloads
    eprosima::fastrtps::types::DynamicType_ptr dyn_type_;
    /// The Dynamic data used to deserialize the payloads
    std::unique_ptr<eprosima::fastrtps::types::DynamicData, DynDataDeleter> dyn_data_;
};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTEREXPRESSION_HPP_

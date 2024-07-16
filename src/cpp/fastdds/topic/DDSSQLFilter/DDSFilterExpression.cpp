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
 * @file DDSFilterExpression.cpp
 */
#include "DDSFilterExpression.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/exceptions/Exception.h>

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

#include "DDSFilterConditionState.hpp"
#include "DDSFilterField.hpp"
#include "DDSFilterParameter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

bool DDSFilterExpression::evaluate(
        const IContentFilter::SerializedPayload& payload,
        const IContentFilter::FilterSampleInfo& sample_info,
        const IContentFilter::GUID_t& reader_guid) const
{
    static_cast<void>(sample_info);
    static_cast<void>(reader_guid);

    using namespace eprosima::fastdds::dds::xtypes;
    using namespace eprosima::fastcdr;

    dyn_data_->clear_all_values();
    try
    {
        FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);
        Cdr deser(fastbuffer
                );
        deser.read_encapsulation();
        traits<DynamicData>::narrow<DynamicDataImpl>(dyn_data_)->deserialize(deser);
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    root->reset();
    for (auto it = fields.begin();
            it != fields.end() && DDSFilterConditionState::UNDECIDED == root->get_state();
            ++it)
    {
        if (!it->second->set_value(dyn_data_))
        {
            return false;
        }
    }

    return DDSFilterConditionState::RESULT_TRUE == root->get_state();
}

void DDSFilterExpression::clear()
{
    DynamicDataFactory::get_instance()->delete_data(dyn_data_);
    DynamicTypeBuilderFactory::get_instance()->delete_type(dyn_type_);
    parameters.clear();
    fields.clear();
    root.reset();
}

void DDSFilterExpression::set_type(
        DynamicType::_ref_type type)
{
    dyn_type_ = type;
    dyn_data_ = traits<DynamicData>::narrow<DynamicDataImpl>(DynamicDataFactory::get_instance()->create_data(type));
}

} // namespace DDSSQLFilter
} // namespace dds
} // namespace fastdds
} // namespace eprosima

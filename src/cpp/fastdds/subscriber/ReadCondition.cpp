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
 * @file ReadCondition.cpp
 */

#include <algorithm>
#include <forward_list>
#include <memory>

#include <fastdds/core/condition/ConditionNotifier.hpp>
#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/ReadCondition.hpp>
#include <fastdds/dds/subscriber/SampleState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>
#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/subscriber/DataReaderImpl/StateFilter.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

namespace detail {

using fastrtps::types::ReturnCode_t;

class ReadConditionImpl : std::enable_shared_from_this<ReadConditionImpl>
{
    DataReaderImpl& data_reader_;
    const StateFilter state_;
    std::forward_list<const ReadCondition*> conditions_;

    public:

    ReadConditionImpl(DataReaderImpl& data_reader, const StateFilter& state)
        : data_reader_(data_reader)
        , state_(state)
    {}

    bool get_trigger_value(const StateFilter& state)
    {
        return state.sample_states & state_.sample_states ||
               state.view_states & state_.view_states ||
               state.instance_states & state_.instance_states;
    }

    bool get_trigger_value() const
    {
        static_assert(false, "TODO");
    }

    DataReader* get_datareader() const
    {
        return data_reader_.user_datareader_;
    }

    const SampleStateMask& get_sample_state_mask() const
    {
        return state_.sample_states;
    }

    const ViewStateMask& get_view_state_mask() const
    {
        return state_.view_states;
    }

    const InstanceStateMask& get_instance_state_mask() const
    {
        return state_.instance_states;
    }

    /**
     * Attach a new ReadCondition to this object.
     * @param [in] pRC reader to attach
     * @pre The DataWriterImpl recursive mutex must be taken because it protects the collection
     * @return RETCODE_OK on success
     */
    ReturnCode_t attach_condition(const ReadCondition* pRC)
    {
        using namespace std;

        auto it = conditions_.begin();
        auto pit = conditions_.before_begin();

        while(it != conditions_.end())
        {
            if(*it < pRC)
            {
                pit = it++;
            }
            else if ( *it == pRC )
            {
                // already there
                return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
            }
            else
            {
                break;
            }
        }

        // insert
        conditions_.insert_after(pit, pRC);

        return ReturnCode_t::RETCODE_OK;
    }

    /**
     * Detach a ReadCondition from this object.
     * @param [in] pRC reader to detach
     * @pre The DataWriterImpl recursive mutex must be taken because it protects the collection
     * @return RETCODE_OK on success
     */
    ReturnCode_t detach_condition(const ReadCondition* pRC)
    {
        using namespace std;

        auto it = conditions_.begin();
        auto pit = conditions_.before_begin();

        while(it != conditions_.end())
        {
            if(*it < pRC)
            {
                pit = it++;
            }
            else if ( *it == pRC )
            {
                conditions_.erase_after(pit);
                return ReturnCode_t::RETCODE_OK;
            }
            else
            {
                break;
            }
        }

        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    /**
     * Notify all the associated ReadConditions
     * @pre The DataWriterImpl recursive mutex must be taken because it protects the collection
     */
    void notify() const
    {
        for(auto cond : conditions_)
        {
            auto pN = cond->get_notifier();
            assert(nullptr != pN);
            pN->notify();
        }
    }
};

}  // namespace detail


ReadCondition::ReadCondition()
{
}

ReadCondition::~ReadCondition()
{
}

bool ReadCondition::get_trigger_value() const
{
    return false;
}

DataReader* ReadCondition::get_datareader() const
{
    return nullptr;
}

SampleStateMask ReadCondition::get_sample_state_mask() const
{
    return ANY_SAMPLE_STATE;
}

ViewStateMask ReadCondition::get_view_state_mask() const
{
    return ANY_VIEW_STATE;
}

InstanceStateMask ReadCondition::get_instance_state_mask() const
{
    return ANY_INSTANCE_STATE;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

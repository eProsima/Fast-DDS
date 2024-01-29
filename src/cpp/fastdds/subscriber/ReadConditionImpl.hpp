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
 * @file ReadConditionImpl.hpp
 */

#include <algorithm>
#include <forward_list>
#include <memory>
#include <mutex>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/ReadCondition.hpp>
#include <fastdds/dds/subscriber/SampleState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>

#include <fastdds/core/condition/ConditionNotifier.hpp>
#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/subscriber/DataReaderImpl/StateFilter.hpp>

#ifndef _FASTDDS_READCONDITIONIMPL_HPP_
#define _FASTDDS_READCONDITIONIMPL_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

class ReadConditionImpl : public std::enable_shared_from_this<ReadConditionImpl>
{
    DataReaderImpl& data_reader_;
    const StateFilter state_;
    StateFilter value_;
    mutable std::mutex value_mtx_;
    std::recursive_mutex& mutex_;
    std::forward_list<const ReadCondition*> conditions_;

    using length = std::forward_list<const ReadCondition*>::difference_type;

public:

    ReadConditionImpl(
            DataReaderImpl& data_reader,
            const StateFilter& state)
        : data_reader_(data_reader)
        , state_(state)
        , value_()
        , mutex_(data_reader.get_conditions_mutex())
    {
    }

    ~ReadConditionImpl()
    {
        // On destruction no ReadCondition should be associated
        assert(conditions_.empty());
    }

    /**
     * Detach all ReadConditions from this object.
     */
    void detach_all_conditions() noexcept
    {
        std::lock_guard<std::recursive_mutex> _(mutex_);

        if (conditions_.empty())
        {
            return;
        }

        auto keep_alive_in_stack = shared_from_this();

        for (const ReadCondition* cond : conditions_)
        {
            delete cond;
        }

        conditions_.clear();
    }

    bool get_trigger_value(
            const StateFilter& state) const noexcept
    {
        return state.sample_states & state_.sample_states &&
               state.view_states & state_.view_states &&
               state.instance_states & state_.instance_states;
    }

    bool get_trigger_value() const noexcept
    {
        std::lock_guard<std::mutex> _(value_mtx_);
        return get_trigger_value(value_);
    }

    DataReader* get_datareader() const noexcept
    {
        return data_reader_.user_datareader_;
    }

    const SampleStateMask& get_sample_state_mask() const noexcept
    {
        return state_.sample_states;
    }

    const ViewStateMask& get_view_state_mask() const noexcept
    {
        return state_.view_states;
    }

    const InstanceStateMask& get_instance_state_mask() const noexcept
    {
        return state_.instance_states;
    }

    /**
     * Attach a new ReadCondition to this object.
     * @param [in] pRC reader to attach
     * @return RETCODE_OK on success
     */
    ReturnCode_t attach_condition(
            ReadCondition* pRC)
    {
        using namespace std;

        lock_guard<recursive_mutex> _(mutex_);

        auto it = conditions_.begin();
        auto pit = conditions_.before_begin();

        while (it != conditions_.end())
        {
            if (*it < pRC)
            {
                pit = it++;
            }
            else if ( *it == pRC )
            {
                // already there
                return RETCODE_PRECONDITION_NOT_MET;
            }
            else
            {
                break;
            }
        }

        // insert
        conditions_.insert_after(pit, pRC);

        // associate
        pRC->impl_ = shared_from_this();

        return RETCODE_OK;
    }

    /**
     * Detach a ReadCondition from this object.
     * @param [in] pRC reader to detach
     * @return RETCODE_OK on success
     */
    ReturnCode_t detach_condition(
            ReadCondition* pRC) noexcept
    {
        using namespace std;

        lock_guard<recursive_mutex> _(mutex_);

        auto it = conditions_.begin();
        auto pit = conditions_.before_begin();

        while (it != conditions_.end())
        {
            if (*it < pRC)
            {
                pit = it++;
            }
            else if ( *it == pRC )
            {
                conditions_.erase_after(pit);

                return RETCODE_OK;
            }
            else
            {
                break;
            }
        }

        return RETCODE_PRECONDITION_NOT_MET;
    }

    void set_trigger_value(
            const StateFilter& value) noexcept
    {
        std::lock_guard<std::mutex> _(value_mtx_);
        value_ = value;
    }

    /**
     * Notify all the associated ReadConditions
     */
    void notify() const noexcept
    {
        std::lock_guard<std::recursive_mutex> _(mutex_);

        for (auto cond : conditions_)
        {
            auto pN = cond->get_notifier();
            assert(nullptr != pN);
            pN->notify();
        }
    }

};

} // namespace detail
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_READCONDITIONIMPL_HPP_ */

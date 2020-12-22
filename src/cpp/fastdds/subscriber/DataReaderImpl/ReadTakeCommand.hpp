// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ReadTakeCommand.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_DATAREADERIMPL_READTAKECOMMAND_HPP_
#define _FASTDDS_SUBSCRIBER_DATAREADERIMPL_READTAKECOMMAND_HPP_

#include <cstdint>

#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastrtps/subscriber/SubscriberHistory.h>
#include <fastrtps/types/TypesBase.h>

#include <fastdds/subscriber/DataReaderImpl/StateFilter.hpp>
#include <fastdds/subscriber/DataReaderImpl/SampleInfoPool.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct ReadTakeCommand
{
    using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;
    using history_type = eprosima::fastrtps::SubscriberHistory;

    ReadTakeCommand(
            history_type& history,
            SampleInfoPool& info_pool,
            LoanableCollection& data_values,
            LoanableCollection& sample_infos,
            int32_t max_samples,
            const StateFilter& states,
            history_type::instance_iterator instance,
            bool single_instance = false)
    {
    }

    void add_instance(
            bool take_samples)
    {
    }

    inline bool is_finished() const
    {
        return finished_;
    }

    inline ReturnCode_t return_value() const
    {
        return return_value_;
    }

private:

    bool finished_ = false;
    ReturnCode_t return_value_ = ReturnCode_t::RETCODE_OK;
};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_DATAREADERIMPL_READTAKECOMMAND_HPP_

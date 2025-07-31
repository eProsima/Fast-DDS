// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cassert>
#include <cstdint>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableTypedCollection.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/subscriber/DataReaderImpl/DataReaderLoanManager.hpp>
#include <fastdds/subscriber/DataReaderImpl/StateFilter.hpp>
#include <fastdds/subscriber/DataReaderImpl/SampleInfoPool.hpp>
#include <fastdds/subscriber/DataReaderImpl/SampleLoanManager.hpp>
#include <fastdds/subscriber/history/DataReaderHistory.hpp>

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/WriterProxy.h>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>


namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct ReadTakeCommand
{
    using history_type = eprosima::fastdds::dds::detail::DataReaderHistory;
    using CacheChange_t = eprosima::fastdds::rtps::CacheChange_t;
    using RTPSReader = eprosima::fastdds::rtps::RTPSReader;
    using WriterProxy = eprosima::fastdds::rtps::WriterProxy;
    using SampleInfoSeq = LoanableTypedCollection<SampleInfo>;
    using DataSharingPayloadPool = eprosima::fastdds::rtps::DataSharingPayloadPool;

    ReadTakeCommand(
            DataReaderImpl& reader,
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos,
            int32_t max_samples,
            const StateFilter& states,
            const history_type::instance_info& instance,
            bool single_instance,
            bool loop_for_data)
        : type_(reader.type_)
        , loan_manager_(reader.loan_manager_)
        , history_(reader.history_)
        , reader_(reader.reader_)
        , info_pool_(reader.sample_info_pool_)
        , sample_pool_(reader.sample_pool_)
        , data_values_(data_values)
        , sample_infos_(sample_infos)
        , remaining_samples_(max_samples)
        , states_(states)
        , instance_(instance)
        , handle_(instance->first)
        , single_instance_(single_instance)
        , loop_for_data_(loop_for_data)
    {
        assert(0 <= remaining_samples_);

        current_slot_ = data_values_.length();
        finished_ = false;
    }

    ~ReadTakeCommand()
    {
        if (!data_values_.has_ownership() && RETCODE_NO_DATA == return_value_)
        {
            loan_manager_.return_loan(data_values_, sample_infos_);
            data_values_.unloan();
            sample_infos_.unloan();
        }
    }

    bool add_instance(
            bool take_samples)
    {
        // Advance to the first instance with a valid state
        if (!go_to_first_valid_instance())
        {
            return false;
        }

        // Traverse changes on current instance
        bool ret_val = false;
        LoanableCollection::size_type first_slot = current_slot_;
        auto it = instance_->second->cache_changes.begin();
        while (!finished_ && it != instance_->second->cache_changes.end())
        {
            CacheChange_t* change = *it;
            SampleStateKind check;
            check = change->isRead ? SampleStateKind::READ_SAMPLE_STATE : SampleStateKind::NOT_READ_SAMPLE_STATE;
            if ((check & states_.sample_states) != 0)
            {
                WriterProxy* wp = nullptr;
                bool is_future_change = false;
                bool remove_change = false;
                if (rtps::BaseReader::downcast(reader_)->begin_sample_access_nts(change, wp, is_future_change))
                {
                    //Check if the payload is dirty
                    remove_change = !check_datasharing_validity(change, data_values_.has_ownership());
                }
                else
                {
                    remove_change = true;
                }

                if (remove_change)
                {
                    // Remove from history
                    history_.remove_change_sub(change, it);

                    // Current iterator will point to change next to the one removed. Avoid incrementing.
                    continue;
                }

                // If the change is in the future we can skip the remaining changes in the history, as they will be
                // in the future also
                if (!is_future_change)
                {
                    // Add sample and info to collections
                    ReturnCode_t previous_return_value = return_value_;
                    bool added = add_sample(*it, remove_change);
                    history_.change_was_processed_nts(change, added);
                    rtps::BaseReader::downcast(reader_)->end_sample_access_nts(change, wp, added);

                    // Check if the payload is dirty
                    if (added && !check_datasharing_validity(change, data_values_.has_ownership()))
                    {
                        // Decrement length of collections
                        --current_slot_;
                        ++remaining_samples_;
                        data_values_.length(current_slot_);
                        sample_infos_.length(current_slot_);

                        return_value_ = previous_return_value;
                        finished_ = false;

                        remove_change = true;
                        added = false;
                    }

                    if (remove_change || (added && take_samples))
                    {
                        // Remove from history
                        history_.remove_change_sub(change, it);

                        // Current iterator will point to change next to the one removed. Avoid incrementing.
                        continue;
                    }
                }
            }

            // Go to next sample on instance
            ++it;
        }

        if (current_slot_ > first_slot)
        {
            history_.instance_viewed_nts(instance_->second);
            ret_val = true;

            // complete sample infos
            LoanableCollection::size_type slot = current_slot_;
            LoanableCollection::size_type n = 0;
            while (slot > first_slot)
            {
                --slot;
                sample_infos_[slot].sample_rank = n;
                ++n;
            }
        }

        // Check if further iteration is required
        if (single_instance_ && (!loop_for_data_ || (loop_for_data_ && ret_val)))
        {
            finished_ = true;
            history_.check_and_remove_instance(instance_);
        }
        else
        {
            next_instance();
        }

        return ret_val;
    }

    inline bool is_finished() const
    {
        return finished_;
    }

    inline ReturnCode_t return_value() const
    {
        return return_value_;
    }

    static void generate_info(
            SampleInfo& info,
            const DataReaderInstance& instance,
            const DataReaderCacheChange& item)
    {
        info.sample_state = item->isRead ? READ_SAMPLE_STATE : NOT_READ_SAMPLE_STATE;
        info.instance_state = instance.instance_state;
        info.view_state = instance.view_state;
        info.disposed_generation_count = item->reader_info.disposed_generation_count;
        info.no_writers_generation_count = item->reader_info.no_writers_generation_count;
        info.sample_rank = 0;
        info.generation_rank = 0;
        info.absolute_generation_rank = 0;
        info.source_timestamp = item->sourceTimestamp;
        info.reception_timestamp = item->reader_info.receptionTimestamp;
        info.instance_handle = item->instanceHandle;
        info.publication_handle = InstanceHandle_t(item->writerGUID);

        /*
         * TODO(eduponz): The sample identity should be taken from the sample identity parameter.
         * More importantly, the related sample identity should be taken from the related sample identity
         * in write_params.
         */
        FASTDDS_TODO_BEFORE(4, 0, "Fill both sample_identity and related_sample_identity with write_params");
        info.sample_identity.writer_guid(item->writerGUID);
        info.sample_identity.sequence_number(item->sequenceNumber);
        info.related_sample_identity = item->write_params.sample_identity();

        info.valid_data = true;

        switch (item->kind)
        {
            case eprosima::fastdds::rtps::NOT_ALIVE_DISPOSED:
            case eprosima::fastdds::rtps::NOT_ALIVE_DISPOSED_UNREGISTERED:
            case eprosima::fastdds::rtps::NOT_ALIVE_UNREGISTERED:
                info.valid_data = false;
                break;
            case eprosima::fastdds::rtps::ALIVE:
            default:
                break;
        }
    }

private:

    const TypeSupport& type_;
    DataReaderLoanManager& loan_manager_;
    history_type& history_;
    RTPSReader* reader_;
    SampleInfoPool& info_pool_;
    std::shared_ptr<detail::SampleLoanManager> sample_pool_;
    LoanableCollection& data_values_;
    SampleInfoSeq& sample_infos_;
    int32_t remaining_samples_;
    StateFilter states_;
    history_type::instance_info instance_;
    InstanceHandle_t handle_;
    bool single_instance_;
    bool loop_for_data_;

    bool finished_ = false;
    ReturnCode_t return_value_ = RETCODE_NO_DATA;

    LoanableCollection::size_type current_slot_ = 0;

    bool go_to_first_valid_instance()
    {
        while (!is_current_instance_valid())
        {
            if ((single_instance_ && !loop_for_data_) || !next_instance())
            {
                finished_ = true;
                return false;
            }
        }

        return true;
    }

    bool is_current_instance_valid()
    {
        // Check instance_state against states_.instance_states and view_state against states_.view_states
        auto instance_state = instance_->second->instance_state;
        auto view_state = instance_->second->view_state;
        return (0 != (states_.instance_states & instance_state)) && (0 != (states_.view_states & view_state));
    }

    bool next_instance()
    {
        history_.check_and_remove_instance(instance_);

        auto result = history_.next_available_instance_nts(handle_, instance_);
        if (!result.first)
        {
            finished_ = true;
            return false;
        }

        instance_ = result.second;
        handle_ = instance_->first;
        return true;
    }

    bool add_sample(
            const DataReaderCacheChange& item,
            bool& deserialization_error)
    {
        bool ret_val = false;
        deserialization_error = false;

        if (remaining_samples_ > 0)
        {
            // Increment length of collections
            auto new_len = current_slot_ + 1;
            data_values_.length(new_len);
            sample_infos_.length(new_len);

            // Add information
            generate_info(item);
            if (sample_infos_[current_slot_].valid_data)
            {
                if (!deserialize_sample(item))
                {
                    // Decrement length of collections
                    data_values_.length(current_slot_);
                    sample_infos_.length(current_slot_);
                    deserialization_error = true;
                    return false;
                }
            }

            // Mark that some data is available
            return_value_ = RETCODE_OK;
            ++current_slot_;
            --remaining_samples_;
            ret_val = true;
        }

        // Finish when there are no remaining samples
        finished_ = (remaining_samples_ == 0);
        return ret_val;
    }

    bool deserialize_sample(
            CacheChange_t* change)
    {
        auto payload = &(change->serializedPayload);
        if (data_values_.has_ownership())
        {
            // perform deserialization
            return type_->deserialize(*payload, data_values_.buffer()[current_slot_]);
        }
        else
        {
            // loan
            void* sample;
            sample_pool_->get_loan(change, sample);
            const_cast<void**>(data_values_.buffer())[current_slot_] = sample;
            return true;
        }
    }

    void generate_info(
            const DataReaderCacheChange& item)
    {
        // Loan when necessary
        if (!sample_infos_.has_ownership())
        {
            SampleInfo* pool_item = info_pool_.get_item();
            assert(pool_item != nullptr);
            const_cast<void**>(sample_infos_.buffer())[current_slot_] = pool_item;
        }

        SampleInfo& info = sample_infos_[current_slot_];
        generate_info(info, *instance_->second, item);
    }

    bool check_datasharing_validity(
            CacheChange_t* change,
            bool has_ownership)
    {
        bool is_valid = true;
        if (has_ownership)  //< On loans the user must check the validity anyways
        {
            DataSharingPayloadPool* pool =
                    dynamic_cast<DataSharingPayloadPool*>(change->serializedPayload.payload_owner);
            if (pool)
            {
                //Check if the payload is dirty
                is_valid = pool->is_sample_valid(*change);
            }
        }

        if (!is_valid)
        {
            EPROSIMA_LOG_WARNING(RTPS_READER,
                    "Change " << change->sequenceNumber << " from " << change->writerGUID << " is overidden");
            return false;
        }

        return true;
    }

};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_DATAREADERIMPL_READTAKECOMMAND_HPP_

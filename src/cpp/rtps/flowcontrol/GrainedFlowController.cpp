/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include "GrainedFlowController.hpp"

#include <cassert>

namespace eprosima {
namespace fastdds {
namespace rtps {

void GrainedFlowControllerFifoSchedule::trigger_bandwidth_limit_reset(
        std::unique_lock<fastdds::TimedMutex>& lock,
        GUID_t reader_guid,
        uint32_t allowed_bytes)
{
    for (auto it = writers_queue_.begin(); it != writers_queue_.end(); ++it)
    {
        if (std::get<1>(*it))
        {
            auto writer {std::get<0>(*it)};
            lock.unlock();
            writer->reschedule_unsent_changes(reader_guid, allowed_bytes);
            lock.lock();
            it = writers_queue_.begin();
        }
    }
}

GrainedFlowControllerLimitedAsyncPublishMode::GrainedFlowControllerLimitedAsyncPublishMode(
        RTPSParticipantImpl* participant,
        const FlowControllerDescriptor* descriptor)
    : FlowControllerAsyncPublishMode(participant, descriptor)
{
    group.set_limitation(this);
}

void GrainedFlowControllerLimitedAsyncPublishMode::add_sent_bytes_by_group(
        uint32_t bytes,
        RTPSMessageSenderInterface& sender)
{
    LocatorSelectorSender& locator_sender = dynamic_cast<LocatorSelectorSender&>(sender);
    locator_sender.locator_selector.for_every_entry([&](LocatorSelectorEntry* entry, size_t)
            {
                if (entry->enabled && entry->allowed_to_send)
                {
                    auto it = remote_readers_.find(entry->remote_guid);
                    assert(it != remote_readers_.end());
                    if (it != remote_readers_.end())
                    {
                        it->second.current_bytes_per_period += bytes;
                    }
                }

                return true; // Always continue
            });

}

bool GrainedFlowControllerLimitedAsyncPublishMode::data_exceeds_limitation(
        CacheChange_t&,
        uint32_t size_to_add,
        uint32_t pending_to_send,
        RTPSMessageSenderInterface& sender)
{
    LocatorSelectorSender& locator_sender {dynamic_cast<LocatorSelectorSender&>(sender)};
    bool one_can_send {false};

    bool ret_for = locator_sender.locator_selector.for_every_entry([&](LocatorSelectorEntry* entry, size_t index)
                    {
                        bool ret_value {true};
                        if (entry->enabled)
                        {
                            auto it = remote_readers_.find(entry->remote_guid);
                            assert(it != remote_readers_.end());
                            if (it != remote_readers_.end())
                            {
                                if (!entry->allowed_to_send)
                                {
                                    // If selection was rebuilt due to a forced reset, we need to unselect
                                    // the entries again, as they are added back regardless of their allowed_to_send state.
                                    // The veto was already decided for this period. Re-apply it in case the
                                    // selection was rebuilt by a forced reset (select_locators() re-adds
                                    // entries purely from their 'enabled' flag, ignoring 'allowed_to_send').
                                    locator_sender.locator_selector.unselect(index);
                                }
                                else if (it->second.max_bytes_per_period <=
                                (it->second.current_bytes_per_period + size_to_add + pending_to_send))
                                {
                                    if (locator_sender.locator_selector.initial_allow_to_send())
                                    {
                                        entry->allowed_to_send = false;
                                        locator_sender.locator_selector.unselect(index);
                                    }
                                    else
                                    {
                                        ret_value = false;
                                    }
                                }
                                else
                                {
                                    one_can_send = true;
                                }
                            }
                        }

                        return ret_value;
                    });
    bool ret_value = !ret_for || !one_can_send;
    if (!ret_value)
    {
        locator_sender.locator_selector.initial_allow_to_send(false);
    }

    return ret_value;
}

void GrainedFlowControllerLimitedAsyncPublishMode::register_remote_reader(
        const GUID_t& remote_reader_guid,
        const int32_t initial_bytes_per_period)
{
    auto ret = remote_readers_.insert({remote_reader_guid, {}});

    if (ret.second)
    {
        ret.first->second.number_of_registrations = 1;
        ret.first->second.max_bytes_per_period = static_cast<uint32_t>(initial_bytes_per_period);
    }
    else
    {
        ret.first->second.number_of_registrations++;
    }
}

void GrainedFlowControllerLimitedAsyncPublishMode::unregister_remote_reader(
        const GUID_t& remote_reader_guid)
{
    auto ret = remote_readers_.find(remote_reader_guid);

    if (remote_readers_.end() != ret)
    {
        assert(ret->second.number_of_registrations > 0);
        ret->second.number_of_registrations--;
        if (0 == ret->second.number_of_registrations)
        {
            remote_readers_.erase(ret);
        }
    }
}

void GrainedFlowControllerLimitedAsyncPublishMode::update_remote_reader_bytes_per_period(
        const GUID_t& remote_reader_guid,
        uint32_t new_bytes_per_period)
{
    auto ret = remote_readers_.find(remote_reader_guid);

    if (remote_readers_.end() != ret)
    {
        ret->second.max_bytes_per_period = new_bytes_per_period;
    }
}

bool GrainedFlowControllerLimitedAsyncPublishMode::fast_check_is_there_slot_for_change(
        CacheChange_t* change)
{
    // Not fragmented sample, the fast check is if the serialized payload fit.
    uint32_t size_to_check = change->serializedPayload.length;

    if (0 != change->getFragmentCount())
    {
        // For fragmented sample, the fast check is the minor fragments fit.
        size_to_check = change->serializedPayload.length % change->getFragmentSize();

        if (0 == size_to_check)
        {
            size_to_check = change->getFragmentSize();
        }


    }

    bool one_can_send {false};
    for (auto& reader_info : remote_readers_)
    {
        if ((reader_info.second.max_bytes_per_period >= reader_info.second.current_bytes_per_period)
                && (reader_info.second.max_bytes_per_period - reader_info.second.current_bytes_per_period) >
                size_to_check)
        {
            one_can_send = true;
        }
    }

    force_wait_ = !one_can_send;

    return one_can_send;
}

bool GrainedFlowControllerLimitedAsyncPublishMode::wait(
        std::unique_lock<fastdds::TimedMutex>& lock)
{
    auto lapse = std::chrono::steady_clock::now() - last_period_;
    bool reset_limit = true;

    if (lapse < period_ms)
    {
        if (std::cv_status::no_timeout == cv.wait_for(lock, period_ms - lapse))
        {
            reset_limit = false;
        }
    }

    if (reset_limit)
    {
        last_period_ = std::chrono::steady_clock::now();
        force_wait_ = false;
        // Reset current bytes per period for each reader
        for (auto& reader_info : remote_readers_)
        {
            reader_info.second.current_bytes_per_period = 0;
        }
    }

    return reset_limit;
}

void GrainedFlowControllerLimitedAsyncPublishMode::prepare_locator_selector(
        LocatorSelectorSender& sender)
{
    if (sender.locator_selector.initial_allow_to_send())
    {
        sender.locator_selector.for_every_entry([&](LocatorSelectorEntry* entry, size_t)
                {
                    entry->allowed_to_send = true;
                    return true; // Always continue
                });
    }
}

void GrainedFlowControllerLimitedAsyncPublishMode::loop(
        std::function<void(GUID_t reader_guid, uint32_t allowed_bytes)> func)
{
    auto reader_info = remote_readers_.begin();

    while (reader_info != remote_readers_.end())
    {
        reader_info->second.processed = true; // Marked as processed.
        if (reader_info->second.max_bytes_per_period >= reader_info->second.current_bytes_per_period)
        {
            GUID_t remote_guid = reader_info->first;
            // The provided function might unlock changes_interested_mutex. Taken this into account.
            func(remote_guid,
                    reader_info->second.max_bytes_per_period - reader_info->second.current_bytes_per_period);

            // Try to find same reader to get the next one.
            reader_info = remote_readers_.find(remote_guid);
            if (reader_info != remote_readers_.end())
            {
                ++reader_info;
            }
            else
            {
                // Get next one no being processed.
                reader_info = remote_readers_.begin();
                while (reader_info != remote_readers_.end() && reader_info->second.processed)
                {
                    ++reader_info;
                }

            }
        }
        else
        {
            ++reader_info;
        }
    }

    // Set all readers as not processed for next time.
    reader_info = remote_readers_.begin();
    while (reader_info != remote_readers_.end())
    {
        reader_info->second.processed = false;
        ++reader_info;
    }
}

GrainedFlowController::GrainedFlowController(
        RTPSParticipantImpl* participant,
        uint32_t async_index,
        ThreadSettings thread_settings,
        uint32_t minimum_bytes_per_period,
        uint64_t period_ms
        )
    : FlowControllerImpl<GrainedFlowControllerLimitedAsyncPublishMode, GrainedFlowControllerFifoSchedule>(participant,
            nullptr,
            async_index,
            thread_settings)
{
    minimum_bytes_per_period_ = minimum_bytes_per_period;
    async_mode.period_ms = std::chrono::milliseconds(period_ms);
}

void GrainedFlowController::register_remote_reader(
        const GUID_t& remote_reader_guid,
        const int32_t initial_bytes_per_period)
{
    std::unique_lock<fastdds::TimedMutex> lock(mutex_);
    async_mode.register_remote_reader(remote_reader_guid, initial_bytes_per_period);
}

void GrainedFlowController::unregister_remote_reader(
        const GUID_t& remote_reader_guid)
{
    std::unique_lock<fastdds::TimedMutex> lock(mutex_);
    async_mode.unregister_remote_reader(remote_reader_guid);
}

void GrainedFlowController::update_remote_reader_bytes_per_period(
        const GUID_t& remote_reader_guid,
        uint32_t new_bytes_per_period)
{
    std::unique_lock<fastdds::TimedMutex> lock(mutex_);
    async_mode.update_remote_reader_bytes_per_period(remote_reader_guid, new_bytes_per_period);
}

void GrainedFlowController::trigger_bandwidth_limit_reset(
        std::unique_lock<fastdds::TimedMutex>& lock)
{
    async_mode.loop([&](GUID_t reader_guid, uint32_t allowed_bytes)
            {
                sched.trigger_bandwidth_limit_reset(lock, reader_guid, allowed_bytes);
            });
    sched.reset_writer_processing_state();
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

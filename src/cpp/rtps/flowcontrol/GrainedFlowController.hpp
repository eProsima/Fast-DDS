/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef _RTPS_FLOWCONTROL_GRAINEDFLOWCONTROLLERIMPL_HPP_
#define _RTPS_FLOWCONTROL_GRAINEDFLOWCONTROLLERIMPL_HPP_

#include <rtps/flowcontrol/FlowControllerImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

const char* const grained_flow_controller_name = "GrainedFlowController";

struct GrainedFlowControllerFifoSchedule : public FlowControllerFifoSchedule
{
    using element = std::tuple<BaseWriter*, bool>;
    using container = std::vector<element>;
    using iterator = container::iterator;

    void register_writer(
            BaseWriter* writer)
    {
        assert(writers_queue_.end() == find(writer));
        writers_queue_.emplace_back(writer, false);
    }

    void unregister_writer(
            BaseWriter* writer)
    {
        auto it = find(writer);
        assert(it != writers_queue_.end());
        writers_queue_.erase(it);
    }

    void trigger_bandwidth_limit_reset(
            std::unique_lock<fastdds::TimedMutex>&) const
    {
        assert(false); // Should not be called
    }

    void trigger_bandwidth_limit_reset(
            std::unique_lock<fastdds::TimedMutex>& lock,
            GUID_t reader_guid,
            uint32_t allowed_bytes);

    void processing_change_of_writer(
            BaseWriter* writer)
    {
        auto it = find(writer);
        assert(it != writers_queue_.end());
        std::get<1>(*it) = true;
    }

    void reset_writer_processing_state()
    {
        for (auto it = writers_queue_.begin(); it != writers_queue_.end(); ++it)
        {
            std::get<1>(*it) = false;
        }
    }

private:

    iterator find(
            const BaseWriter* writer)
    {
        return std::find_if(writers_queue_.begin(), writers_queue_.end(),
                       [writer](const element& current_writer) -> bool
                       {
                           return writer == std::get<0>(current_writer);
                       });
    }

    container writers_queue_;
};

struct GrainedFlowControllerLimitedAsyncPublishMode : public FlowControllerAsyncPublishMode,
    public IRTPSMessageGroupLimitation
{
    struct RemoteReaderFlowInfo
    {
        uint32_t number_of_registrations {0};
        uint32_t current_bytes_per_period {0};
        uint32_t max_bytes_per_period {0};
        bool processed {false};
    };

    GrainedFlowControllerLimitedAsyncPublishMode(
            RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor);

    void add_sent_bytes_by_group(
            uint32_t bytes,
            RTPSMessageSenderInterface& sender) override;

    bool data_exceeds_limitation(
            CacheChange_t& change,
            uint32_t size_to_add,
            uint32_t pending_to_send,
            RTPSMessageSenderInterface& sender) override;

    void register_remote_reader(
            const GUID_t& remote_reader_guid,
            const int32_t initial_bytes_per_period);

    void unregister_remote_reader(
            const GUID_t& remote_reader_guid);

    void update_remote_reader_bytes_per_period(
            const GUID_t& remote_reader_guid,
            uint32_t new_bytes_per_period);

    bool fast_check_is_there_slot_for_change(
            CacheChange_t* change);

    bool wait(
            std::unique_lock<fastdds::TimedMutex>& lock);

    bool force_wait() const
    {
        return force_wait_;
    }

    void prepare_locator_selector(
            LocatorSelectorSender&);

    void process_deliver_retcode(
            const DeliveryRetCode& ret_value)
    {
        if (DeliveryRetCode::EXCEEDED_LIMIT == ret_value)
        {
            force_wait_ = true;
        }
    }

    void loop(
            std::function<void(GUID_t reader_guid, uint32_t allowed_bytes)> func);

    std::chrono::milliseconds period_ms;

private:

    std::map<GUID_t, RemoteReaderFlowInfo> remote_readers_;

    bool force_wait_ {false};

    std::chrono::steady_clock::time_point last_period_ {std::chrono::steady_clock::now()};
};

class GrainedFlowController : public FlowControllerImpl<GrainedFlowControllerLimitedAsyncPublishMode,
            GrainedFlowControllerFifoSchedule>
{
public:

    GrainedFlowController(
            RTPSParticipantImpl* participant,
            uint32_t async_index,
            ThreadSettings thread_settings,
            uint32_t minimum_bytes_per_period,
            uint64_t period_ms);

    void register_remote_reader(
            const GUID_t& remote_reader_guid,
            const int32_t initial_bytes_per_period);

    void unregister_remote_reader(
            const GUID_t& remote_reader_guid);

    void update_remote_reader_bytes_per_period(
            const GUID_t& remote_reader_guid,
            uint32_t new_bytes_per_period);

    uint32_t get_max_payload() override
    {
        return minimum_bytes_per_period_;
    }

    void trigger_bandwidth_limit_reset(
            std::unique_lock<fastdds::TimedMutex>& lock) override;

private:

    uint32_t minimum_bytes_per_period_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_GRAINEDFLOWCONTROLLERIMPL_HPP_

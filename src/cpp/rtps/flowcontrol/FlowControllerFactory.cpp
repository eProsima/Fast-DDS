#include "FlowControllerFactory.hpp"
#include "FlowControllerImpl.hpp"

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

void FlowControllerFactory::init(
        fastdds::rtps::RTPSParticipantImpl* participant)
{
    participant_ = participant;
    // Create default flow controllers.

    const ThreadSettings& sender_thread_settings =
            (nullptr == participant_) ? ThreadSettings{}
            : participant_->get_attributes().builtin_controllers_sender_thread;

    // PureSyncFlowController -> used by volatile besteffort writers.
    flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                pure_sync_flow_controller_name,
                std::unique_ptr<FlowController>(
                    new FlowControllerImpl<FlowControllerPureSyncPublishMode,
                    FlowControllerFifoSchedule>(participant_, nullptr, 0, sender_thread_settings))));
    // SyncFlowController -> used by rest of besteffort writers.
    flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                sync_flow_controller_name,
                std::unique_ptr<FlowController>(
                    new FlowControllerImpl<FlowControllerSyncPublishMode,
                    FlowControllerFifoSchedule>(participant_, nullptr, async_controller_index_++,
                    sender_thread_settings))));
    // AsyncFlowController
    flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                async_flow_controller_name,
                std::unique_ptr<FlowController>(
                    new FlowControllerImpl<FlowControllerAsyncPublishMode,
                    FlowControllerFifoSchedule>(participant_, nullptr, async_controller_index_++,
                    sender_thread_settings))));

#ifdef FASTDDS_STATISTICS
    flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                async_statistics_flow_controller_name,
                std::unique_ptr<FlowController>(
                    new FlowControllerImpl<FlowControllerAsyncPublishMode,
                    FlowControllerFifoSchedule>(participant_, nullptr, async_controller_index_++,
                    sender_thread_settings))));
#endif // ifndef FASTDDS_STATISTICS
}

void FlowControllerFactory::register_flow_controller (
        const FlowControllerDescriptor& flow_controller_descr)
{
    if (flow_controllers_.end() != flow_controllers_.find(flow_controller_descr.name))
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "Error registering FlowController " << flow_controller_descr.name << ". Already registered");
        return;
    }

    const ThreadSettings& sender_thread_settings = flow_controller_descr.sender_thread;

    if (0 < flow_controller_descr.max_bytes_per_period)
    {
        switch (flow_controller_descr.scheduler)
        {
            case FlowControllerSchedulerPolicy::FIFO:
                flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                            flow_controller_descr.name,
                            std::unique_ptr<FlowController>(
                                new FlowControllerImpl<FlowControllerLimitedAsyncPublishMode,
                                FlowControllerFifoSchedule>(participant_,
                                &flow_controller_descr, async_controller_index_++, sender_thread_settings))));
                break;
            case FlowControllerSchedulerPolicy::ROUND_ROBIN:
                flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                            flow_controller_descr.name,
                            std::unique_ptr<FlowController>(
                                new FlowControllerImpl<FlowControllerLimitedAsyncPublishMode,
                                FlowControllerRoundRobinSchedule>(participant_,
                                &flow_controller_descr, async_controller_index_++, sender_thread_settings))));
                break;
            case FlowControllerSchedulerPolicy::HIGH_PRIORITY:
                flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                            flow_controller_descr.name,
                            std::unique_ptr<FlowController>(
                                new FlowControllerImpl<FlowControllerLimitedAsyncPublishMode,
                                FlowControllerHighPrioritySchedule>(participant_,
                                &flow_controller_descr, async_controller_index_++, sender_thread_settings))));
                break;
            case FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION:
                flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                            flow_controller_descr.name,
                            std::unique_ptr<FlowController>(
                                new FlowControllerImpl<FlowControllerLimitedAsyncPublishMode,
                                FlowControllerPriorityWithReservationSchedule>(participant_,
                                &flow_controller_descr, async_controller_index_++, sender_thread_settings))));
                break;
            default:
                assert(false);
        }
    }
    else
    {
        switch (flow_controller_descr.scheduler)
        {
            case FlowControllerSchedulerPolicy::FIFO:
                flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                            flow_controller_descr.name,
                            std::unique_ptr<FlowController>(
                                new FlowControllerImpl<FlowControllerAsyncPublishMode,
                                FlowControllerFifoSchedule>(participant_,
                                &flow_controller_descr, async_controller_index_++, sender_thread_settings))));
                break;
            case FlowControllerSchedulerPolicy::ROUND_ROBIN:
                flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                            flow_controller_descr.name,
                            std::unique_ptr<FlowController>(
                                new FlowControllerImpl<FlowControllerAsyncPublishMode,
                                FlowControllerRoundRobinSchedule>(participant_,
                                &flow_controller_descr, async_controller_index_++, sender_thread_settings))));
                break;
            case FlowControllerSchedulerPolicy::HIGH_PRIORITY:
                flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                            flow_controller_descr.name,
                            std::unique_ptr<FlowController>(
                                new FlowControllerImpl<FlowControllerAsyncPublishMode,
                                FlowControllerHighPrioritySchedule>(participant_,
                                &flow_controller_descr, async_controller_index_++, sender_thread_settings))));
                break;
            case FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION:
                flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                            flow_controller_descr.name,
                            std::unique_ptr<FlowController>(
                                new FlowControllerImpl<FlowControllerAsyncPublishMode,
                                FlowControllerPriorityWithReservationSchedule>(participant_,
                                &flow_controller_descr, async_controller_index_++, sender_thread_settings))));
                break;
            default:
                assert(false);
        }
    }
}

/*!
 * Get a FlowController given its name.
 *
 * @param flow_controller_name Name of the interested FlowController.
 * @return Pointer to the FlowController. nullptr if no registered FlowController with that name.
 */
FlowController* FlowControllerFactory::retrieve_flow_controller(
        const std::string& flow_controller_name,
        const fastdds::rtps::WriterAttributes& writer_attributes)
{
    FlowController* returned_flow = nullptr;

    // Detect it has to be returned a default flow_controller.
    if (0 == flow_controller_name.compare(FASTDDS_FLOW_CONTROLLER_DEFAULT))
    {
        if (fastdds::rtps::SYNCHRONOUS_WRITER == writer_attributes.mode)
        {
            if (fastdds::rtps::BEST_EFFORT == writer_attributes.endpoint.reliabilityKind)
            {
                returned_flow = flow_controllers_[pure_sync_flow_controller_name].get();
            }
            else
            {
                returned_flow = flow_controllers_[sync_flow_controller_name].get();
            }
        }
        else
        {
            returned_flow = flow_controllers_[async_flow_controller_name].get();
        }
    }
#ifdef FASTDDS_STATISTICS
    else if (0 == flow_controller_name.compare(FASTDDS_STATISTICS_FLOW_CONTROLLER_DEFAULT))
    {
        assert(fastdds::rtps::ASYNCHRONOUS_WRITER == writer_attributes.mode);
        returned_flow = flow_controllers_[async_statistics_flow_controller_name].get();
    }
#endif // ifdef FASTDDS_STATISTICS
    else
    {
        auto it = flow_controllers_.find(flow_controller_name);

        if (flow_controllers_.end() != it)
        {
            returned_flow = it->second.get();
        }
    }

    if (nullptr != returned_flow)
    {
        returned_flow->init();
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "Cannot find FlowController " << flow_controller_name << ".");
    }

    return returned_flow;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

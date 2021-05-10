#include "FlowControllerFactory.hpp"
#include "FlowControllerImpl.hpp"

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

constexpr char* const pure_sync_flow_controller_name = "PureSyncFlowController";
constexpr char* const sync_flow_controller_name = "SyncFlowController";
constexpr char* const async_flow_controller_name = "AsyncFlowController";

FlowControllerFactory::~FlowControllerFactory()
{
    std::for_each(flow_controllers_.begin(), flow_controllers_.end(),
            [](const std::pair<std::string, FlowController*>& flow_controller)
            {
                delete flow_controller.second;
            });
    flow_controllers_.clear();
}

void FlowControllerFactory::init(
        fastrtps::rtps::RTPSParticipantImpl* participant)
{
    participant_ = participant;
    // Create default flow controllers.

    // PureSyncFlowController -> used by volatile besteffort writers.
    flow_controllers_.insert({pure_sync_flow_controller_name,
                              new FlowControllerImpl<FlowControllerPureSyncPublishMode,
                              FlowControllerFifoSchedule>(participant_, nullptr)});
    // SyncFlowController -> used by rest of besteffort writers.
    flow_controllers_.insert({sync_flow_controller_name,
                              new FlowControllerImpl<FlowControllerSyncPublishMode,
                              FlowControllerFifoSchedule>(participant_, nullptr)});
    // AsyncFlowController
    flow_controllers_.insert({async_flow_controller_name,
                              new FlowControllerImpl<FlowControllerAsyncPublishMode,
                              FlowControllerFifoSchedule>(participant_, nullptr)});
}

void FlowControllerFactory::register_flow_controller (
        const FlowControllerDescriptor& flow_controller_descr)
{
    if (flow_controllers_.end() != flow_controllers_.find(flow_controller_descr.name))
    {
        logError(RTPS_PARTICIPANT,
                "Error registering FlowController " << flow_controller_descr.name << ". Already registered");
        return;
    }

    if (0 < flow_controller_descr.max_bytes_per_period)
    {
        if (FlowControllerSchedulerPolicy::FIFO == flow_controller_descr.scheduler)
        {
            flow_controllers_.insert({flow_controller_descr.name,
                                      new FlowControllerImpl<FlowControllerLimitedAsyncPublishMode,
                                      FlowControllerFifoSchedule>(participant_, &flow_controller_descr)});
        }
    }
    else
    {
        if (FlowControllerSchedulerPolicy::FIFO == flow_controller_descr.scheduler)
        {
            flow_controllers_.insert({flow_controller_descr.name,
                                      new FlowControllerImpl<FlowControllerAsyncPublishMode,
                                      FlowControllerFifoSchedule>(participant_, &flow_controller_descr)});
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
        const fastrtps::rtps::WriterAttributes& writer_attributes)
{
    FlowController* returned_flow = nullptr;

    // Detect it has to be returned a default flow_controller.
    if (0 == flow_controller_name.compare(FASTDDS_FLOW_CONTROLLER_DEFAULT))
    {
        if (fastrtps::rtps::SYNCHRONOUS_WRITER == writer_attributes.mode)
        {
            if (fastrtps::rtps::BEST_EFFORT == writer_attributes.endpoint.reliabilityKind)
            {
                returned_flow = flow_controllers_[pure_sync_flow_controller_name];
            }
            else
            {
                returned_flow = flow_controllers_[sync_flow_controller_name];
            }
        }
        else
        {
            returned_flow = flow_controllers_[async_flow_controller_name];
        }
    }
    else
    {
        auto it = flow_controllers_.find(flow_controller_name);

        if (flow_controllers_.end() != it)
        {
            returned_flow = it->second;
        }
    }

    if (nullptr != returned_flow)
    {
        returned_flow->init();
    }
    else
    {
        logError(RTPS_PARTICIPANT,
                "Cannot find FlowController " << flow_controller_name << ".");
    }

    return returned_flow;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

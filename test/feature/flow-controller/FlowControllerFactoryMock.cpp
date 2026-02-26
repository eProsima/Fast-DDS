/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include "FlowControllerFactoryMock.hpp"
#include <rtps/flowcontrol/GrainedFlowController.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

GrainedFlowController* FlowControllerFactoryMock::grained_flow_controller {nullptr};

void FlowControllerFactoryMock::init(
        fastdds::rtps::RTPSParticipantImpl* participant)
{
    FlowControllerFactory::init(participant);

    const ThreadSettings& sender_thread_settings =
            (nullptr == participant_) ? ThreadSettings{}
            : participant_->get_attributes().builtin_controllers_sender_thread;

    // GrainedFlowControllerTest
    GrainedFlowController* ptr = new GrainedFlowController(participant_, async_controller_index_++,
                    sender_thread_settings, 1000, 100);

    if (!grained_flow_controller)
    {
        grained_flow_controller = ptr;
    }

    flow_controllers_.insert(decltype(flow_controllers_)::value_type(
                test_grained_flow_controller_name,
                std::unique_ptr<FlowController>(ptr)));
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

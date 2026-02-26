/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef _RTPS_FLOWCONTROL_FLOWCONTROLLERFACTORYMOCK_HPP_
#define _RTPS_FLOWCONTROL_FLOWCONTROLLERFACTORYMOCK_HPP_

#include <rtps/flowcontrol/FlowControllerFactory.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class GrainedFlowController;

const char* const test_grained_flow_controller_name = "GrainedFlowControllerTest";

class FlowControllerFactoryMock : public FlowControllerFactory
{
public:

    void init(
            fastdds::rtps::RTPSParticipantImpl* participant) override;

    static GrainedFlowController* grained_flow_controller;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_FLOWCONTROLLERFACTORYPROMOCK_HPP_

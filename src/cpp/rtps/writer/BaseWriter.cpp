// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BaseWriter.cpp
 */

#include <rtps/writer/BaseWriter.hpp>

#include <memory>

#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <rtps/flowcontrol/FlowController.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

BaseWriter::BaseWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& att,
        FlowController* flow_controller,
        WriterHistory* hist,
        WriterListener* listen)
    : RTPSWriter(impl, guid, att, flow_controller, hist, listen)
{
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

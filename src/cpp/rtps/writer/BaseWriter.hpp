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
 * @file BaseWriter.hpp
 */

#ifndef RTPS_WRITER__BASEWRITER_HPP
#define RTPS_WRITER__BASEWRITER_HPP

#include <memory>

#include <fastdds/rtps/writer/RTPSWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CacheChange_t;
class DataSharingNotifier;
class FlowController;
struct GUID_t;
class ICacheChangePool;
class IPayloadPool;
class RTPSMessageGroup;
class RTPSParticipantImpl;
class WriterAttributes;
class WriterHistory;
class WriterListener;

class BaseWriter : public fastdds::rtps::RTPSWriter
{

public:

    virtual ~BaseWriter();

protected:

    BaseWriter(
            RTPSParticipantImpl* impl,
            const GUID_t& guid,
            const WriterAttributes& att,
            FlowController* flow_controller,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

    void deinit();

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_WRITER__BASEWRITER_HPP

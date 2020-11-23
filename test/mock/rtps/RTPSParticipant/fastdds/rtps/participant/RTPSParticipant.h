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


#ifndef _FASTDDS_RTPS_RTPSParticipant_H_
#define _FASTDDS_RTPS_RTPSParticipant_H_

#include <cstdlib>
#include <memory>
#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastrtps/qos/ReaderQos.h>
#include <fastrtps/qos/WriterQos.h>

#include <gmock/gmock.h>

namespace eprosima {

namespace fastdds {
namespace dds {
namespace builtin {

class TypeLookupManager;

} // namespace builtin
} // namespace dds
} // namespace fastdds

namespace fastrtps {

class TopicAttributes;

namespace rtps {

class RTPSParticipantImpl;
class RTPSParticipantListener;
class RTPSWriter;
class RTPSReader;
class WriterProxyData;
class ReaderProxyData;
class ResourceEvent;
class WLP;

/**
 * @brief Class RTPSParticipant, contains the public API for a RTPSParticipant.
 * @ingroup RTPS_MODULE
 */
class RTPS_DllAPI RTPSParticipant
{
    friend class RTPSParticipantImpl;
    friend class RTPSDomain;

public:

    RTPSParticipant(
            RTPSParticipantImpl*)
    {
    }

    RTPSParticipant()
    {
    }

    virtual ~RTPSParticipant()
    {
    }

    bool set_listener(
            RTPSParticipantListener* listener)
    {
        listener_ = listener;
        return true;
    }

    const GUID_t& getGuid() const
    {
        return m_guid;
    }

    MOCK_METHOD0(enable, void());

    MOCK_CONST_METHOD0(wlp, WLP * ());

    void set_check_type_function(
            std::function<bool(const std::string&)>&&)
    {
    }

    MOCK_CONST_METHOD0(getParticipantNames, std::vector<std::string>());

    MOCK_METHOD2(newRemoteWriterDiscovered, bool(
                const GUID_t& pguid,
                int16_t userDefinedId));

    MOCK_METHOD2(newRemoteReaderDiscovered, bool(
                const GUID_t& pguid,
                int16_t userDefinedId));

    ResourceEvent& get_resource_event()
    {
        return mp_event_thr;
    }

    MOCK_CONST_METHOD0(typelookup_manager, fastdds::dds::builtin::TypeLookupManager* ());

    MOCK_METHOD3(registerWriter, bool(
                RTPSWriter * Writer,
                const TopicAttributes& topicAtt,
                const WriterQos& wqos));

    MOCK_METHOD3(updateWriter, bool(
                RTPSWriter * Writer,
                const TopicAttributes& topicAtt,
                const WriterQos& wqos));

    MOCK_METHOD3(registerReader, bool(
                RTPSReader * Reader,
                const TopicAttributes& topicAtt,
                const ReaderQos& rqos));

    MOCK_METHOD3(updateReader, bool(
                RTPSReader * Reader,
                const TopicAttributes& topicAtt,
                const ReaderQos& rqos));

    const RTPSParticipantAttributes& getRTPSParticipantAttributes()
    {
        return attributes_;
    }

#if HAVE_SECURITY

    MOCK_METHOD1(is_security_enabled_for_writer, bool(
                const WriterAttributes& writer_attributes));

    MOCK_METHOD1(is_security_enabled_for_reader, bool(
                const ReaderAttributes& reader_attributes));

#endif // if HAVE_SECURITY

    RTPSParticipantListener* listener_;
    const GUID_t m_guid;
    ResourceEvent mp_event_thr;
    RTPSParticipantAttributes attributes_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_RTPSParticipant_H_ */

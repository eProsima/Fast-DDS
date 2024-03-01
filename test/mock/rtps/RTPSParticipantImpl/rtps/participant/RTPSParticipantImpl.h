// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSParticipantImpl.h
 */

#ifndef _RTPS_PARTICIPANT_RTPSPARTICIPANTIMPL_H_
#define _RTPS_PARTICIPANT_RTPSPARTICIPANTIMPL_H_

// Include first possible mocks (depending on include on CMakeLists.txt)
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <rtps/network/NetworkFactory.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/common/LocatorList.hpp>

#if HAVE_SECURITY
#include <rtps/security/SecurityManager.h>
#endif // if HAVE_SECURITY

#include <gmock/gmock.h>

#include <atomic>
#include <map>
#include <sstream>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class Endpoint;
class RTPSParticipant;
class WriterHistory;
class ReaderHistory;
class WriterListener;
class ReaderListener;
class PDPSimple;
struct EntityId_t;
class ReaderProxyData;
class WriterProxyData;
class ReaderAttributes;
class NetworkFactory;

#if HAVE_SECURITY
namespace security {
class SecurityManager;
struct ParticipantSecurityAttributes;
} // namespace security
#endif // if HAVE_SECURITY

class MockParticipantListener : public RTPSParticipantListener
{
public:

    void onParticipantDiscovery(
            RTPSParticipant* participant,
            ParticipantDiscoveryInfo&& info) override
    {
        onParticipantDiscovery_mock(participant, info);
    }

    MOCK_METHOD2(onParticipantDiscovery_mock, void (RTPSParticipant*, const ParticipantDiscoveryInfo&));

    void onParticipantDiscovery(
            RTPSParticipant* participant,
            ParticipantDiscoveryInfo&& info,
            bool& should_be_ignored) override
    {
        onParticipantDiscovery_mock(participant, info, should_be_ignored);
    }

    MOCK_METHOD3(onParticipantDiscovery_mock, void (RTPSParticipant*, const ParticipantDiscoveryInfo&, bool&));

#if HAVE_SECURITY
    void onParticipantAuthentication(
            RTPSParticipant* participant,
            ParticipantAuthenticationInfo&& info) override
    {
        onParticipantAuthentication(participant, info);
    }

    MOCK_METHOD2(onParticipantAuthentication, void (RTPSParticipant*, const ParticipantAuthenticationInfo&));
#endif // if HAVE_SECURITY
};

class RTPSParticipantImpl
{
public:

    RTPSParticipantImpl()
    {
        events_.init_thread();
    }

    MOCK_CONST_METHOD0(get_domain_id, uint32_t());

    MOCK_CONST_METHOD0(getGuid, const GUID_t& ());

    MOCK_CONST_METHOD0(network_factory, const NetworkFactory& ());

    MOCK_METHOD0(is_intraprocess_only, bool());

    MOCK_METHOD0(get_persistence_guid_prefix, GuidPrefix_t());

#if HAVE_SECURITY
    MOCK_CONST_METHOD0(security_attributes, const security::ParticipantSecurityAttributes& ());

    MOCK_METHOD2(pairing_remote_reader_with_local_writer_after_security, bool(const GUID_t&, const ReaderProxyData&));

    MOCK_METHOD2(pairing_remote_writer_with_local_reader_after_security,
            bool(const GUID_t&, const WriterProxyData& remote_writer_data));

    MOCK_CONST_METHOD0(is_secure, bool ());

    MOCK_METHOD0(security_manager, security::SecurityManager& ());

#endif // if HAVE_SECURITY

    MOCK_METHOD1(setGuid, void(GUID_t &));

    MOCK_METHOD1(check_type, bool(std::string));

    MOCK_METHOD2(on_entity_discovery,
            void(const fastrtps::rtps::GUID_t&, const fastdds::dds::ParameterPropertyList_t&));

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD6(createWriter_mock,
            bool (RTPSWriter** writer, WriterAttributes& param, WriterHistory* hist,
            WriterListener* listen,
            const EntityId_t& entityId, bool isBuiltin));

    MOCK_METHOD7(createReader_mock,
            bool (RTPSReader** reader, ReaderAttributes& param, ReaderHistory* hist,
            ReaderListener* listen,
            const EntityId_t& entityId, bool isBuiltin, bool enable));
    // *INDENT-ON*

    MOCK_CONST_METHOD0(getParticipantMutex, std::recursive_mutex* ());

    bool createWriter(
            RTPSWriter** writer,
            WriterAttributes& param,
            WriterHistory* hist,
            WriterListener* listen,
            const EntityId_t& entityId = c_EntityId_Unknown,
            bool isBuiltin = false)
    {
        bool ret = createWriter_mock(writer, param, hist, listen, entityId, isBuiltin);
        if (*writer != nullptr)
        {
            (*writer)->history_ = hist;

            auto guid = generate_endpoint_guid();
            (*writer)->m_guid = guid;
            endpoints_.emplace(guid, *writer);
        }
        return ret;
    }

    bool createWriter(
            RTPSWriter** writer,
            WriterAttributes& param,
            const std::shared_ptr<IPayloadPool>&,
            WriterHistory* hist,
            WriterListener* listen,
            const EntityId_t& entityId = c_EntityId_Unknown,
            bool isBuiltin = false)
    {
        bool ret = createWriter_mock(writer, param, hist, listen, entityId, isBuiltin);
        if (*writer != nullptr)
        {
            (*writer)->history_ = hist;

            auto guid = generate_endpoint_guid();
            (*writer)->m_guid = guid;
            endpoints_.emplace(guid, *writer);
        }
        return ret;
    }

    bool createReader(
            RTPSReader** reader,
            ReaderAttributes& param,
            ReaderHistory* hist,
            ReaderListener* listen,
            const EntityId_t& entityId = c_EntityId_Unknown,
            bool isBuiltin = false,
            bool enable = true)
    {
        bool ret = createReader_mock(reader, param, hist, listen, entityId, isBuiltin, enable);
        if (*reader != nullptr)
        {
            (*reader)->history_ = hist;
            (*reader)->listener_ = listen;

            auto guid = generate_endpoint_guid();
            (*reader)->m_guid = guid;
            endpoints_.emplace(guid, *reader);
        }
        return ret;
    }

    bool createReader(
            RTPSReader** reader,
            ReaderAttributes& param,
            const std::shared_ptr<IPayloadPool>&,
            ReaderHistory* hist,
            ReaderListener* listen,
            const EntityId_t& entityId = c_EntityId_Unknown,
            bool isBuiltin = false,
            bool enable = true)
    {
        bool ret = createReader_mock(reader, param, hist, listen, entityId, isBuiltin, enable);
        if (*reader != nullptr)
        {
            (*reader)->history_ = hist;
            (*reader)->listener_ = listen;

            auto guid = generate_endpoint_guid();
            (*reader)->m_guid = guid;
            endpoints_.emplace(guid, *reader);
        }
        return ret;
    }

    bool deleteUserEndpoint(
            const GUID_t& e)
    {
        // Check the map
        auto it = endpoints_.find(e);
        if ( it != endpoints_.end())
        {
            delete it->second;
            endpoints_.erase(it);
        }

        return true;
    }

    MOCK_METHOD0(pdp, PDP * ());

    MOCK_METHOD0(pdpsimple, PDPSimple * ());

    MockParticipantListener* getListener()
    {
        return &listener_;
    }

    RTPSParticipant* getUserRTPSParticipant()
    {
        return nullptr;
    }

    ResourceEvent& getEventResource()
    {
        return events_;
    }

    void set_endpoint_rtps_protection_supports(
            Endpoint* /*endpoint*/,
            bool /*support*/)
    {
    }

    void ResourceSemaphoreWait()
    {
    }

    void ResourceSemaphorePost()
    {
    }

    uint32_t getMaxMessageSize() const
    {
        return 65536;
    }

    const RTPSParticipantAttributes& getRTPSParticipantAttributes() const
    {
        return attr_;
    }

    RTPSParticipantAttributes& getAttributes()
    {
        return attr_;
    }

    void get_sending_locators(
            rtps::LocatorList_t& /*locators*/) const
    {
    }

    template <EndpointKind_t kind, octet no_key, octet with_key>
    static bool preprocess_endpoint_attributes(
            const EntityId_t&,
            std::atomic<uint32_t>&,
            EndpointAttributes&,
            EntityId_t&)
    {
        return true;
    }

    template<class Functor>
    Functor forEachUserWriter(
            Functor f)
    {
        return f;
    }

    template<class Functor>
    Functor forEachUserReader(
            Functor f)
    {
        return f;
    }

    MOCK_METHOD(bool, should_match_local_endpoints, ());

    MOCK_METHOD(bool, ignore_participant, (const GuidPrefix_t&));

    MOCK_METHOD(bool, update_removed_participant, (rtps::LocatorList_t&));

private:

    MockParticipantListener listener_;

    ResourceEvent events_;

    RTPSParticipantAttributes attr_;

    std::map<GUID_t, Endpoint*> endpoints_;

    GUID_t generate_endpoint_guid() const
    {
        static uint32_t counter = 0;
        const char* prefix = "49.20.48.61.74.65.20.47.4D.6F.63.6B";

        GUID_t res;
        std::istringstream is(prefix);
        is >> res.guidPrefix;
        res.entityId = ++counter;
        return res;
    }

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_PARTICIPANT_RTPSPARTICIPANTIMPL_H_

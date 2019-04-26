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
 * @file RTPSParticipantAttributes.h
 */

#ifndef _RTPSPARTICIPANTPARAMETERS_H_
#define _RTPSPARTICIPANTPARAMETERS_H_

#include "../common/Time_t.h"
#include "../common/Locator.h"
#include "../common/PortParameters.h"
#include "PropertyPolicy.h"
#include "../flowcontrol/ThroughputControllerDescriptor.h"
#include "../../transport/TransportInterface.h"
#include "../resources/ResourceManagement.h"
#include "../../utils/fixed_size_string.hpp"

#include <memory>

namespace eprosima {
namespace fastrtps{
namespace rtps {

/**
 * Class SimpleEDPAttributes, to define the attributes of the Simple Endpoint Discovery Protocol.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class SimpleEDPAttributes
{
public:

    //!Default value true.
    bool use_PublicationWriterANDSubscriptionReader;

    //!Default value true.
    bool use_PublicationReaderANDSubscriptionWriter;

#if HAVE_SECURITY
    bool enable_builtin_secure_publications_writer_and_subscriptions_reader;

    bool enable_builtin_secure_subscriptions_writer_and_publications_reader;
#endif

    SimpleEDPAttributes()
        : use_PublicationWriterANDSubscriptionReader(true)
        , use_PublicationReaderANDSubscriptionWriter(true)
#if HAVE_SECURITY
        , enable_builtin_secure_publications_writer_and_subscriptions_reader(true)
        , enable_builtin_secure_subscriptions_writer_and_publications_reader(true)
#endif
    {
    }

    bool operator==(const SimpleEDPAttributes& b) const
    {
        return (this->use_PublicationWriterANDSubscriptionReader == b.use_PublicationWriterANDSubscriptionReader) &&
#if HAVE_SECURITY
                (this->enable_builtin_secure_publications_writer_and_subscriptions_reader ==
                b.enable_builtin_secure_publications_writer_and_subscriptions_reader) &&
                (this->enable_builtin_secure_subscriptions_writer_and_publications_reader ==
                b.enable_builtin_secure_subscriptions_writer_and_publications_reader) &&
#endif
                (this->use_PublicationReaderANDSubscriptionWriter == b.use_PublicationReaderANDSubscriptionWriter);
    }
};

/**
 * Class BuiltinAttributes, to define the behavior of the RTPSParticipant builtin protocols.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class BuiltinAttributes
{
    public:
        /**
         * If set to false, NO discovery whatsoever would be used.
         * Publisher and Subscriber defined with the same topic name would NOT be linked. All matching must be done
         * manually through the addReaderLocator, addReaderProxy, addWriterProxy methods.
         */
        bool use_SIMPLE_RTPSParticipantDiscoveryProtocol;

        //!Indicates to use the WriterLiveliness protocol.
        bool use_WriterLivelinessProtocol;

        /**
         * If set to true, SimpleEDP would be used.
         */
        bool use_SIMPLE_EndpointDiscoveryProtocol;

        /**
         * If set to true, StaticEDP based on an XML file would be implemented.
         * The XML filename must be provided.
         */
        bool use_STATIC_EndpointDiscoveryProtocol;

        /**
         * DomainId to be used by the RTPSParticipant (80 by default).
         */
        uint32_t domainId;

        /**
         * Lease Duration of the RTPSParticipant,
         * indicating how much time remote RTPSParticipants should consider this RTPSParticipant alive.
         */
        Duration_t leaseDuration;

        /**
         * The period for the RTPSParticipant to send its Discovery Message to all other discovered RTPSParticipants
         * as well as to all Multicast ports.
         */
        Duration_t leaseDuration_announcementperiod;

        //!Attributes of the SimpleEDP protocol
        SimpleEDPAttributes m_simpleEDP;

        //!Metatraffic Unicast Locator List
        LocatorList_t metatrafficUnicastLocatorList;

        //!Metatraffic Multicast Locator List.
        LocatorList_t metatrafficMulticastLocatorList;

        //! Initial peers.
        LocatorList_t initialPeersList;

        //! Memory policy for builtin readers
        MemoryManagementPolicy_t readerHistoryMemoryPolicy;

        //! Memory policy for builtin writers
        MemoryManagementPolicy_t writerHistoryMemoryPolicy;

        //! Mutation tries if the port is being used.
        uint32_t mutation_tries;

        BuiltinAttributes()
        {
            use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
            use_SIMPLE_EndpointDiscoveryProtocol = true;
            use_STATIC_EndpointDiscoveryProtocol = false;
            m_staticEndpointXMLFilename = "";
            domainId = 0;
            leaseDuration.seconds = 130;
            leaseDuration_announcementperiod.seconds = 40;
            use_WriterLivelinessProtocol = true;
            readerHistoryMemoryPolicy = MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE;
            writerHistoryMemoryPolicy = MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE;
            mutation_tries = 100u;
        }
        virtual ~BuiltinAttributes() {}

        bool operator==(const BuiltinAttributes& b) const
        {
            return (this->use_SIMPLE_RTPSParticipantDiscoveryProtocol ==
                       b.use_SIMPLE_RTPSParticipantDiscoveryProtocol) &&
                   (this->use_WriterLivelinessProtocol == b.use_WriterLivelinessProtocol) &&
                   (this->use_SIMPLE_EndpointDiscoveryProtocol == b.use_SIMPLE_EndpointDiscoveryProtocol) &&
                   (this->use_STATIC_EndpointDiscoveryProtocol == b.use_STATIC_EndpointDiscoveryProtocol) &&
                   (this->domainId == b.domainId) &&
                   (this->leaseDuration == b.leaseDuration) &&
                   (this->leaseDuration_announcementperiod == b.leaseDuration_announcementperiod) &&
                   (this->m_simpleEDP == b.m_simpleEDP) &&
                   (this->metatrafficUnicastLocatorList == b.metatrafficUnicastLocatorList) &&
                   (this->metatrafficMulticastLocatorList == b.metatrafficMulticastLocatorList) &&
                   (this->initialPeersList == b.initialPeersList) &&
                   (this->readerHistoryMemoryPolicy == b.readerHistoryMemoryPolicy) &&
                   (this->writerHistoryMemoryPolicy == b.writerHistoryMemoryPolicy) &&
                   (this->m_staticEndpointXMLFilename == b.m_staticEndpointXMLFilename) &&
                   (this->mutation_tries == b.mutation_tries);
        }

        /**
         * Get the static endpoint XML filename
         * @return Static endpoint XML filename
         */
        const char* getStaticEndpointXMLFilename() const { return m_staticEndpointXMLFilename.c_str(); }

        /**
         * Set the static endpoint XML filename
         * @param str Static endpoint XML filename
         */
        void setStaticEndpointXMLFilename(const char* str) { m_staticEndpointXMLFilename = std::string(str); }

    private:
        //! StaticEDP XML filename, only necessary if use_STATIC_EndpointDiscoveryProtocol=true
        std::string m_staticEndpointXMLFilename;
};

/**
 * Class RTPSParticipantAttributes used to define different aspects of a RTPSParticipant.
 *@ingroup RTPS_ATTRIBUTES_MODULE
 */
class RTPSParticipantAttributes
{
    public:

        RTPSParticipantAttributes()
        {
            setName("RTPSParticipant");
            sendSocketBufferSize = 0;
            listenSocketBufferSize = 0;
            participantID = -1;
            useBuiltinTransports = true;
        }

        virtual ~RTPSParticipantAttributes() {}

        bool operator==(const RTPSParticipantAttributes& b) const
        {
            return (this->name == b.name) &&
                   (this->defaultUnicastLocatorList == b.defaultUnicastLocatorList) &&
                   (this->defaultMulticastLocatorList == b.defaultMulticastLocatorList) &&
                   (this->sendSocketBufferSize == b.sendSocketBufferSize) &&
                   (this->listenSocketBufferSize == b.listenSocketBufferSize) &&
                   (this->builtin == b.builtin) &&
                   (this->port == b.port) &&
                   (this->userData == b.userData) &&
                   (this->participantID == b.participantID) &&
                   (this->throughputController == b.throughputController) &&
                   (this->useBuiltinTransports == b.useBuiltinTransports) &&
                   (this->properties == b.properties);
        }

        /**
         * Default list of Unicast Locators to be used for any Endpoint defined inside this RTPSParticipant in the case
         * that it was defined with NO UnicastLocators. At least ONE locator should be included in this list.
         */
        LocatorList_t defaultUnicastLocatorList;

        /**
         * Default list of Multicast Locators to be used for any Endpoint defined inside this RTPSParticipant in the
         * case that it was defined with NO UnicastLocators. This is usually left empty.
         */
        LocatorList_t defaultMulticastLocatorList;

        /*!
         * @brief Send socket buffer size for the send resource. Zero value indicates to use default system buffer size.
         * Default value: 0.
         */
        uint32_t sendSocketBufferSize;

        /*! Listen socket buffer for all listen resources. Zero value indicates to use default system buffer size.
         * Default value: 0.
         */
        uint32_t listenSocketBufferSize;

        //! Builtin parameters.
        BuiltinAttributes builtin;

        //!Port Parameters
        PortParameters port;

        //!User Data of the participant
        std::vector<octet> userData;

        //!Participant ID
        int32_t participantID;

        //!Throughput controller parameters. Leave default for uncontrolled flow.
        ThroughputControllerDescriptor throughputController;

        //!User defined transports to use alongside or in place of builtins.
        std::vector<std::shared_ptr<TransportDescriptorInterface>> userTransports;

        //!Set as false to disable the default UDPv4 implementation.
        bool useBuiltinTransports;

        //! Property policies
        PropertyPolicy properties;

        //!Set the name of the participant.
        inline void setName(const char* nam) { name = nam; }

        //!Get the name of the participant.
        inline const char* getName() const { return name.c_str(); }

    private:
        //!Name of the participant.
        string_255 name;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _RTPSPARTICIPANTPARAMETERS_H_ */

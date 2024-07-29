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
 * @file BuiltinProtocols.h
 *
 */

#ifndef FASTDDS_RTPS_BUILTIN__BUILTINPROTOCOLS_H
#define FASTDDS_RTPS_BUILTIN__BUILTINPROTOCOLS_H
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <list>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>

#include <utils/shared_mutex.hpp>

namespace eprosima {

namespace fastdds {
namespace dds {

namespace builtin {

class TypeLookupManager;

} // namespace builtin

class ReaderQos;
class WriterQos;
} // namespace dds

class TopicAttributes;

namespace rtps {

class PDP;
class WLP;
class RTPSParticipantImpl;
class RTPSWriter;
class RTPSReader;
class NetworkFactory;
struct PublicationBuiltinTopicData;
struct SubscriptionBuiltinTopicData;

/**
 * Class BuiltinProtocols that contains builtin endpoints implementing the discovery and liveliness protocols.
 * *@ingroup BUILTIN_MODULE
 */
class BuiltinProtocols
{
    friend class RTPSParticipantImpl;

private:

    BuiltinProtocols();
    virtual ~BuiltinProtocols();

    /*
     * Mutex to protect the m_DiscoveryServers collection. Element access is not protected by this mutex, the PDP mutex
     * needs to be used when querying or modifying mutable members of the collection.
     */
    mutable eprosima::shared_mutex discovery_mutex_;

public:

    /**
     * Initialize the builtin protocols.
     * @param attributes Discovery configuration attributes
     * @param p_part Pointer to the Participant implementation
     * @return True if correct.
     */
    bool initBuiltinProtocols(
            RTPSParticipantImpl* p_part,
            BuiltinAttributes& attributes);

    /**
     * Enable the builtin protocols
     */
    void enable();

    /**
     * Update the metatraffic locatorlist after it was created. Because when you create
     * the EDP readers you are not sure the selected endpoints can be used.
     * @param loclist LocatorList to update
     * @return True on success
     */
    bool updateMetatrafficLocators(
            LocatorList_t& loclist);

    /**
     * Traverses the list of discover servers filtering out unsupported or not allowed remote locators
     * @param nf NetworkFactory used to make the filtering
     */
    void filter_server_remote_locators(
            NetworkFactory& nf);

    //!BuiltinAttributes of the builtin protocols.
    BuiltinAttributes m_att;
    //!Pointer to the RTPSParticipantImpl.
    RTPSParticipantImpl* mp_participantImpl;
    //!Pointer to the PDPSimple.
    PDP* mp_PDP;
    //!Pointer to the WLP
    WLP* mp_WLP;
    //!Pointer to the TypeLookupManager
    fastdds::dds::builtin::TypeLookupManager* typelookup_manager_;
    //!Locator list for metatraffic
    LocatorList_t m_metatrafficMulticastLocatorList;
    //!Locator List for metatraffic unicast
    LocatorList_t m_metatrafficUnicastLocatorList;
    //! Initial peers
    LocatorList_t m_initialPeersList;
    //! Known discovery and backup server container
    LocatorList_t m_DiscoveryServers;

    /**
     * Add a local writer to the BuiltinProtocols.
     *
     * @param writer  Pointer to the RTPSWriter
     * @param topic   Information regarding the topic where the writer is registering
     * @param qos     QoS policies dictated by the publisher
     *
     * @return True if correct.
     */
    bool add_writer(
            RTPSWriter* rtps_writer,
            const TopicDescription& topic,
            const fastdds::dds::WriterQos& qos);
    /**
     * Add a local reader to the BuiltinProtocols.
     *
     * @param rtps_reader     Pointer to the RTPSReader.
     * @param topic           Information regarding the topic where the writer is registering
     * @param qos             QoS policies dictated by the subscriber
     * @param content_filter  Optional content filtering information.
     * @return True if correct.
     */
    bool add_reader(
            RTPSReader* rtps_reader,
            const TopicDescription& topic,
            const fastdds::dds::ReaderQos& qos,
            const fastdds::rtps::ContentFilterProperty* content_filter = nullptr);

    /**
     * Update a local Writer QOS
     * @param rtps_writer      Writer to update
     * @param wqos             New Writer QoS
     * @return
     */
    bool update_writer(
            RTPSWriter* rtps_writer,
            const fastdds::dds::WriterQos& wqos);
    /**
     * Update a local Reader QOS
     * @param rtps_reader      Reader to update
     * @param rqos             New Reader QoS
     * @param content_filter   Optional content filtering information.
     * @return
     */
    bool update_reader(
            RTPSReader* rtps_reader,
            const fastdds::dds::ReaderQos& rqos,
            const fastdds::rtps::ContentFilterProperty* content_filter = nullptr);
    /**
     * Remove a local Writer from the builtinProtocols.
     * @param W Pointer to the writer.
     * @return True if correctly removed.
     */
    bool remove_writer(
            RTPSWriter* rtps_writer);
    /**
     * Remove a local Reader from the builtinProtocols.
     * @param R Pointer to the reader.
     * @return True if correctly removed.
     */
    bool remove_reader(
            RTPSReader* rtps_reader);

    //! Announce RTPSParticipantState (force the sending of a DPD message.)
    void announceRTPSParticipantState();
    //!Stop the RTPSParticipant Announcement (used in tests to avoid multiple packets being send)
    void stopRTPSParticipantAnnouncement();
    //!Reset to timer to make periodic RTPSParticipant Announcements.
    void resetRTPSParticipantAnnouncement();

    /**
     * Get Discovery mutex
     * @return Associated Mutex
     */
    inline eprosima::shared_mutex& getDiscoveryMutex() const
    {
        return discovery_mutex_;
    }

};

} // namespace rtps
} // namespace rtps
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_BUILTIN__BUILTINPROTOCOLS_H

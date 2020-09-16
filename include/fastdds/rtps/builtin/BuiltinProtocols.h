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

#ifndef _FASTDDS_RTPS_BUILTINPROTOCOLS_H_
#define _FASTDDS_RTPS_BUILTINPROTOCOLS_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <list>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/network/NetworkFactory.h>

namespace eprosima {

namespace fastdds {
namespace dds {
namespace builtin {

class TypeLookupManager;

} // namespace builtin

class ReaderQos;
class WriterQos;
} // namespace dds
} // namespace fastdds

namespace fastrtps {

class TopicAttributes;

namespace rtps {

class PDP;
class WLP;
class RTPSParticipantImpl;
class RTPSWriter;
class RTPSReader;

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
     * Update the metatraffic locatorlist after it was created. Because when you create
     * the EDP readers you are not sure the selected endpoints can be used.
     * @param loclist LocatorList to update
     * @return True on success
     */
    bool updateMetatrafficLocators(
            LocatorList_t& loclist);

    /**
     * Traverses the list of discover servers translating from remote to local locators
     * if possible
     * @param nf NetworkFactory used to make the translation
     */
    void transform_server_remote_locators(
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
    fastdds::dds::builtin::TypeLookupManager* tlm_;
    //!Locator list for metatraffic
    LocatorList_t m_metatrafficMulticastLocatorList;
    //!Locator List for metatraffic unicast
    LocatorList_t m_metatrafficUnicastLocatorList;
    //! Initial peers
    LocatorList_t m_initialPeersList;
    //! Known discovery and backup server container
    std::list<eprosima::fastdds::rtps::RemoteServerAttributes> m_DiscoveryServers;

    /**
     * Add a local Writer to the BuiltinProtocols.
     * @param w Pointer to the RTPSWriter
     * @param topicAtt Attributes of the associated topic
     * @param wqos QoS policies dictated by the publisher
     * @return True if correct.
     */
    bool addLocalWriter(
            RTPSWriter* w,
            const TopicAttributes& topicAtt,
            const fastdds::dds::WriterQos& wqos);
    /**
     * Add a local Reader to the BuiltinProtocols.
     * @param R Pointer to the RTPSReader.
     * @param topicAtt Attributes of the associated topic
     * @param rqos QoS policies dictated by the subscriber
     * @return True if correct.
     */
    bool addLocalReader(
            RTPSReader* R,
            const TopicAttributes& topicAtt,
            const fastdds::dds::ReaderQos& rqos);
    /**
     * Update a local Writer QOS
     * @param W Writer to update
     * @param topicAtt Attributes of the associated topic
     * @param wqos New Writer QoS
     * @return
     */
    bool updateLocalWriter(
            RTPSWriter* W,
            const TopicAttributes& topicAtt,
            const fastdds::dds::WriterQos& wqos);
    /**
     * Update a local Reader QOS
     * @param R Reader to update
     * @param topicAtt Attributes of the associated topic
     * @param qos New Reader QoS
     * @return
     */
    bool updateLocalReader(
            RTPSReader* R,
            const TopicAttributes& topicAtt,
            const fastdds::dds::ReaderQos& qos);
    /**
     * Remove a local Writer from the builtinProtocols.
     * @param W Pointer to the writer.
     * @return True if correctly removed.
     */
    bool removeLocalWriter(
            RTPSWriter* W);
    /**
     * Remove a local Reader from the builtinProtocols.
     * @param R Pointer to the reader.
     * @return True if correctly removed.
     */
    bool removeLocalReader(
            RTPSReader* R);

    //! Announce RTPSParticipantState (force the sending of a DPD message.)
    void announceRTPSParticipantState();
    //!Stop the RTPSParticipant Announcement (used in tests to avoid multiple packets being send)
    void stopRTPSParticipantAnnouncement();
    //!Reset to timer to make periodic RTPSParticipant Announcements.
    void resetRTPSParticipantAnnouncement();

};

} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_BUILTINPROTOCOLS_H_ */

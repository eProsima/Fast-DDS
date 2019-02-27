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
 * @file ParticipantImpl.h
 *
 */

#ifndef PARTICIPANTIMPL_H_
#define PARTICIPANTIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <fastrtps/rtps/common/Guid.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

namespace eprosima{
namespace fastrtps{

namespace rtps{
class RTPSParticipant;
class WriterProxyData;
class ReaderProxyData;
}



class Participant;
class ParticipantListener;

class TopicDataType;
class Publisher;
class PublisherImpl;
class PublisherAttributes;
class PublisherListener;
class Subscriber;
class SubscriberImpl;
class SubscriberAttributes;
class SubscriberListener;


/**
 * This is the implementation class of the Participant.
 * @ingroup FASTRTPS_MODULE
 */
class ParticipantImpl
{
    friend class Domain;
    typedef std::pair<Publisher*,PublisherImpl*> t_p_PublisherPair;
    typedef std::pair<Subscriber*,SubscriberImpl*> t_p_SubscriberPair;
    typedef std::vector<t_p_PublisherPair> t_v_PublisherPairs;
    typedef std::vector<t_p_SubscriberPair> t_v_SubscriberPairs;

    private:
    ParticipantImpl(
        const ParticipantAttributes& patt,
        Participant* pspart,
        ParticipantListener* listen = nullptr);
    virtual ~ParticipantImpl();

    public:

    /**
     * Register a type in this participant.
     * @param type Pointer to the TopicDatType.
     * @return True if registered.
     */
    bool registerType(TopicDataType* type);

    /**
     * Unregister a type in this participant.
     * @param typeName Name of the type
     * @return True if unregistered.
     */
    bool unregisterType(const char* typeName);

    /**
     * Create a Publisher in this Participant.
     * @param att Attributes of the Publisher.
     * @param listen Pointer to the listener.
     * @return Pointer to the created Publisher.
     */
    Publisher* createPublisher(
        const PublisherAttributes& att,
        PublisherListener* listen=nullptr);

    /**
     * Create a Subscriber in this Participant.
     * @param att Attributes of the Subscriber
     * @param listen Pointer to the listener.
     * @return Pointer to the created Subscriber.
     */
    Subscriber* createSubscriber(
        const SubscriberAttributes& att,
        SubscriberListener* listen=nullptr);

    /**
     * Remove a Publisher from this participant.
     * @param pub Pointer to the Publisher.
     * @return True if correctly removed.
     */
    bool removePublisher(Publisher* pub);

    /**
     * Remove a Subscriber from this participant.
     * @param sub Pointer to the Subscriber.
     * @return True if correctly removed.
     */
    bool removeSubscriber(Subscriber* sub);

    /**
     * Get the GUID_t of the associated RTPSParticipant.
     * @return GUID_t.
     */
    const rtps::GUID_t& getGuid() const;

    /**
     * Get the participant attributes
     * @return Participant attributes
     */
    inline const ParticipantAttributes& getAttributes() const {return m_att;};

    std::pair<rtps::StatefulReader*,rtps::StatefulReader*> getEDPReaders();

    std::vector<std::string> getParticipantNames() const;

    /**
     * This method can be used when using a StaticEndpointDiscovery mechanism differnet that the one
     * included in FastRTPS, for example when communicating with other implementations.
     * It indicates the Participant that an Endpoint from the XML has been discovered and
     * should be activated.
     * @param partguid Participant GUID_t.
     * @param userId User defined ID as shown in the XML file.
     * @param kind EndpointKind (WRITER or READER)
     * @return True if correctly found and activated.
     */
    bool newRemoteEndpointDiscovered(
        const rtps::GUID_t& partguid,
        uint16_t userId,
        rtps::EndpointKind_t kind);

    bool get_remote_writer_info(
        const rtps::GUID_t& writerGuid,
        rtps::WriterProxyData& returnedInfo);

    bool get_remote_reader_info(
        const rtps::GUID_t& readerGuid,
        rtps::ReaderProxyData& returnedInfo);

    ResourceEvent& get_resource_event() const;

    private:
    //!Participant Attributes
    ParticipantAttributes m_att;
    //!RTPSParticipant
	rtps::RTPSParticipant* mp_rtpsParticipant;
    //!Participant*
    Participant* mp_participant;
    //!Participant Listener
    ParticipantListener* mp_listener;
    //!Publisher Vector
    t_v_PublisherPairs m_publishers;
    //!Subscriber Vector
    t_v_SubscriberPairs m_subscribers;
    //!TOpicDatType vector
    std::vector<TopicDataType*> m_types;

    bool getRegisteredType(const char* typeName, TopicDataType** type);

    class MyRTPSParticipantListener : public rtps::RTPSParticipantListener
    {
        public:

            MyRTPSParticipantListener(ParticipantImpl* impl): mp_participantimpl(impl) {}

            virtual ~MyRTPSParticipantListener() {}

            void onParticipantDiscovery(rtps::RTPSParticipant* participant, rtps::ParticipantDiscoveryInfo&& info) override;

#if HAVE_SECURITY
            void onParticipantAuthentication(rtps::RTPSParticipant* participant, rtps::ParticipantAuthenticationInfo&& info) override;
#endif

            void onReaderDiscovery(rtps::RTPSParticipant* participant, rtps::ReaderDiscoveryInfo&& info) override;

            void onWriterDiscovery(rtps::RTPSParticipant* participant, rtps::WriterDiscoveryInfo&& info) override;

            ParticipantImpl* mp_participantimpl;

    } m_rtps_listener;

};

} /* namespace  */
} /* namespace eprosima */
#endif
#endif /* PARTICIPANTIMPL_H_ */

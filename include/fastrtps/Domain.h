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
 * @file Domain.h
 *
 */

#ifndef DOMAIN_H_
#define DOMAIN_H_

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <mutex>

#ifdef USE_DEPRECATION
#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#define DEPRECATED /** --Deprecated-- */
#endif
#else
#define DEPRECATED
#endif

namespace eprosima{
namespace fastrtps{

class ParticipantListener;
class Participant;
class ParticipantImpl;
class Publisher;
class PublisherAttributes;
class PublisherListener;
class Subscriber;
class SubscriberAttributes;
class SubscriberListener;

namespace types{
class DynamicPubSubType;
}

/**
 * Class Domain, use to interact with the Publisher Subscriber API of the Fast RTPS implementation.
 *  @ingroup FASTRTPS_MODULE
 */
class Domain
{
    public:
        /**
         * Create a Participant from a profile name.
         * @param participant_profile Participant profile name.
         * @param listen ParticipantListener Pointer.
         * @return Participant pointer. (nullptr if not created.)
         */
        RTPS_DllAPI DEPRECATED static Participant* createParticipant(
                const std::string& participant_profile,
                ParticipantListener* listen = nullptr);

        /**
         * Create a Participant.
         * @param att Participant Attributes.
         * @param listen ParticipantListener Pointer.
         * @return Participant pointer. (nullptr if not created.)
         */
        RTPS_DllAPI DEPRECATED static Participant* createParticipant(
                const ParticipantAttributes& att,
                ParticipantListener* listen = nullptr);

        //!Fills participant_attributes with the default values.
        RTPS_DllAPI static void getDefaultParticipantAttributes(ParticipantAttributes& participant_attributes);

        /**
         * Create a Publisher in a Participant from a profile name.
         * @param part Pointer to the participant where you want to create the Publisher.
         * @param publisher_profile Publisher profile name.
         * @param listen Pointer to the PublisherListener.
         * @return Pointer to the created Publisher (nullptr if not created).
         */
        RTPS_DllAPI static Publisher* createPublisher(
                Participant* part,
                const std::string& publisher_profile,
                PublisherListener* listen = nullptr);

        /**
         * Create a Publisher in a Participant.
         * @param part Pointer to the participant where you want to create the Publisher.
         * @param att PublisherAttributes.
         * @param listen Pointer to the PublisherListener.
         * @return Pointer to the created Publisher (nullptr if not created).
         */
        RTPS_DllAPI static Publisher* createPublisher(
                Participant* part,
                const PublisherAttributes& att,
                PublisherListener* listen = nullptr);

        //!Fills publisher_attributes with the default values.
        RTPS_DllAPI static void getDefaultPublisherAttributes(PublisherAttributes& publisher_attributes);

        /**
         * Create a Subscriber in a Participant from a profile name.
         * @param part Pointer to the participant where you want to create the Publisher.
         * @param subscriber_profile Subscriber profile name.
         * @param listen Pointer to the SubscriberListener.
         * @return Pointer to the created Subscriber (nullptr if not created).
         */
        RTPS_DllAPI static Subscriber* createSubscriber(
                Participant* part,
                const std::string& subscriber_profile,
                SubscriberListener* listen = nullptr);

        /**
         * Create a Subscriber in a Participant.
         * @param part Pointer to the participant where you want to create the Publisher.
         * @param att SubscriberAttributes.
         * @param listen Pointer to the SubscriberListener.
         * @return Pointer to the created Subscriber (nullptr if not created).
         */
        RTPS_DllAPI static Subscriber* createSubscriber(
                Participant* part,
                const SubscriberAttributes& att,
                SubscriberListener* listen = nullptr);

        //!Fills subscriber_attributes with the default values.
        RTPS_DllAPI static void getDefaultSubscriberAttributes(SubscriberAttributes& subscriber_attributes);

        /**
         * Remove a Participant and all associated publishers and subscribers.
         * @param part Pointer to the participant.
         * @return True if correctly removed.
         */
        RTPS_DllAPI static bool removeParticipant(Participant* part);

        /**
         * Remove a Publisher.
         * @param pub Pointer to the Publisher.
         * @return True if correctly removed.
         */
        RTPS_DllAPI static bool removePublisher(Publisher* pub);

        /**
         * Remove a Subscriber.
         * @param sub Pointer to the Subscriber.
         * @return True if correctly removed.
         */
        RTPS_DllAPI static bool removeSubscriber(Subscriber* sub);

        /**
         * Return a registered type.
         * @param part Pointer to the Participant.
         * @param typeName Name of the type.
         * @param type Returned type.
         * @return True if type was found.
         */
        RTPS_DllAPI static bool getRegisteredType(
                Participant* part,
                const char* typeName,
                fastdds::dds::TopicDataType** type);

        /**
         * Register a type in a participant.
         * @param part Pointer to the Participant.
         * @param type Pointer to the Type.
         * @return True if correctly registered.
         */
        RTPS_DllAPI static bool registerType(
                Participant* part,
                fastdds::dds::TopicDataType * type);

        /**
         * Register a type in a participant.
         * @param part Pointer to the Participant.
         * @param type Pointer to the Type.
         * @return True if correctly registered.
         */
        RTPS_DllAPI static bool registerDynamicType(
                Participant* part,
                types::DynamicPubSubType* type);

        /**
         * Unregister a type in a participant.
         * @param part Pointer to the Participant.
         * @param typeName Name of the type.
         * @return True if correctly unregistered.
         */
        RTPS_DllAPI static bool unregisterType(
                Participant* part,
                const char* typeName);

        /**
         * Stop and remove all participants, publishers and subscribers in this Domain.
         */
        RTPS_DllAPI static void stopAll();

        /**
         * Load profiles from XML file.
         * @param xml_profile_file XML profile file.
         * @return True if correctly loaded.
         */
        RTPS_DllAPI static bool loadXMLProfilesFile(const std::string& xml_profile_file);

    private:

        typedef std::pair<Participant*,ParticipantImpl*> t_p_Participant;

        Domain();

        virtual ~Domain();

        static std::mutex m_mutex;

        static std::vector<t_p_Participant> m_participants;

        static bool default_xml_profiles_loaded;
};

} /* namespace  */
} /* namespace eprosima */

#endif /* DOMAIN_H_ */

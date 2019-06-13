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
 * @file Publisher.h
 */



#ifndef PUBLISHERIMPL_H_
#define PUBLISHERIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/rtps/common/Guid.h>

#include <fastrtps/attributes/PublisherAttributes.h>


#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/timedevent/TimedCallback.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>

#include <map>

namespace eprosima {
namespace fastrtps{
namespace rtps
{
class RTPSWriter;
class RTPSParticipant;
}

class TopicAttributes;
class PublisherListener;
class ParticipantImpl;
class Publisher;
class WriterQos;


/**
 * Class PublisherImpl, contains the actual implementation of the behaviour of the Publisher.
 * @ingroup FASTRTPS_MODULE
 */
class PublisherImpl
{
    friend class ParticipantImpl;

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using DomainRTPSParticipant static function.
     */
    PublisherImpl(
            ParticipantImpl* p,
            const PublisherAttributes& att,
            PublisherListener* p_listen = nullptr);

public:

    virtual ~PublisherImpl();

    /**
     * Removes the cache change with the minimum sequence number
     * @return True if correct.
     */
    bool removeMinSeqChange();
    /**
     * Removes all changes from the History.
     * @param[out] removed Number of removed elements
     * @return True if correct.
     */
    bool removeAllChange(size_t* removed);

    /**
     * Update the Attributes of the publisher;
     * @param att Reference to a PublisherAttributes object to update the parameters;
     * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
     */
    bool updateAttributes(const PublisherAttributes& att);

    /**
     * Get the Attributes of the Subscriber.
     * @return Attributes of the Subscriber.
     */
    inline const PublisherAttributes& getAttributes(){ return m_att; };

    ParticipantImpl* participant()
    {
        return mp_participant;
    }

    /**
     * @brief Created a new writer
     * @param topic_att TopicAttributes
     */
    rtps::RTPSWriter* create_writer(
            const TopicAttributes& topic_att);

    bool update_writer(
            rtps::RTPSWriter* Writer,
            const TopicAttributes& topicAtt,
            const WriterQos& wqos) const;

    Publisher* user_publisher()
    {
        return mp_userPublisher;
    }

private:
    ParticipantImpl* mp_participant;

    //! Pointer to the associated Data Writer.
    rtps::RTPSWriter* mp_writer;

    //!Attributes of the Publisher
    PublisherAttributes m_att;

    //!PublisherListener
    PublisherListener* mp_listener;

    //!Listener to capture the events of the Writer
    class PublisherWriterListener: public rtps::WriterListener
    {
        public:
            PublisherWriterListener(PublisherImpl* p):mp_publisherImpl(p){};
            virtual ~PublisherWriterListener(){};
            void onWriterMatched(rtps::RTPSWriter* writer, rtps::MatchingInfo& info);
            void onWriterChangeReceivedByAll(rtps::RTPSWriter* writer, rtps::CacheChange_t* change);
            PublisherImpl* mp_publisherImpl;
    }m_writerListener;

    Publisher* mp_userPublisher;

    rtps::RTPSParticipant* mp_rtpsParticipant;

};


} /* namespace  */
} /* namespace eprosima */
#endif
#endif /* PUBLISHER_H_ */

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

#ifndef _FASTDDS_ENTITY_MOCKS_HPP_
#define _FASTDDS_ENTITY_MOCKS_HPP_

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/topic/TopicImpl.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>


namespace eprosima {
namespace fastdds {
namespace dds {

// Mocked DataReaderImpl with access to the inner members
class DataReaderImplMock : public DataReaderImpl
{
public:

    DataReaderImplMock(
            SubscriberImpl* s,
            TypeSupport& type,
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener = nullptr)
        : DataReaderImpl(s, type, topic, qos, listener)
    {
    }

    void set_user_datareader(
            DataReader* reader)
    {
        user_datareader_ = reader;
    }

    fastrtps::rtps::ReaderListener* get_inner_listener()
    {
        return &reader_listener_;
    }

};

// Mocked DataReader with access to the DataReaderImpl
class DataReaderMock : public DataReader
{
public:

    DataReaderMock(
            DataReaderImpl* impl,
            const StatusMask& mask = StatusMask::all())
        : DataReader(impl, mask)
    {
    }

    DataReaderMock(
            Subscriber* s,
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
        : DataReader(s, topic, qos, listener, mask)
    {
    }

    DataReaderImplMock* get_implementation()
    {
        return dynamic_cast<DataReaderImplMock*>(impl_);
    }

};

// Mocked SubscriberImpl with access to the inner members
class SubscriberImplMock : public SubscriberImpl
{
public:

    SubscriberImplMock(
            DomainParticipantImpl* p,
            const SubscriberQos& qos,
            SubscriberListener* p_listen = nullptr)
        : SubscriberImpl(p, qos, p_listen)
    {
    }

    DataReaderListener* get_inner_listener()
    {
        return &subscriber_listener_;
    }

    void set_user_subscriber(
            Subscriber* sub)
    {
        user_subscriber_ = sub;
    }

    void set_rtps_participant(
            fastrtps::rtps::RTPSParticipant* part)
    {
        rtps_participant_ = part;
    }

    void set_handle(
            fastrtps::rtps::InstanceHandle_t handle)
    {
        handle_ = handle;
    }

    DataReaderMock* create_datareader(
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener,
            const StatusMask& mask)
    {
        //Look for the correct type registration
        TypeSupport type_support = participant_->find_type(topic->get_type_name());

        /// Preconditions
        // Check the type was registered.
        if (type_support.empty())
        {
            return nullptr;
        }

        if (!DataReaderImpl::check_qos(qos))
        {
            return nullptr;
        }

        topic->get_impl()->reference();

        DataReaderImplMock* impl = new DataReaderImplMock(
            this,
            type_support,
            topic,
            qos,
            listener);

        DataReaderMock* reader = new DataReaderMock(impl, mask);
        impl->set_user_datareader(reader);

        {
            std::lock_guard<std::mutex> lock(mtx_readers_);
            readers_[topic->get_name()].push_back(impl);
        }

        if (user_subscriber_->is_enabled() && qos_.entity_factory().autoenable_created_entities)
        {
            if (ReturnCode_t::RETCODE_OK != reader->enable())
            {
                delete_datareader(reader);
                return nullptr;
            }
        }

        return reader;
    }

};

// Mocked Subscriber with access to the SubscriberImpl
class SubscriberMock : public Subscriber
{
public:

    SubscriberMock(
            SubscriberImpl* p,
            const StatusMask& mask = StatusMask::all())
        : Subscriber(p, mask)
    {
    }

    SubscriberMock(
            DomainParticipant* dp,
            const SubscriberQos& qos = SUBSCRIBER_QOS_DEFAULT,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
        : Subscriber(dp, qos, listener, mask)
    {
    }

    SubscriberImplMock* get_implementation()
    {
        return dynamic_cast<SubscriberImplMock*>(impl_);
    }

    DataReaderMock* create_datareader(
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        return dynamic_cast<SubscriberImplMock*>(impl_)->create_datareader(topic, qos, listener, mask);
    }

};

// Mocked DataWriterImpl with access to the inner members
class DataWriterImplMock : public DataWriterImpl
{
public:

    DataWriterImplMock(
            PublisherImpl* p,
            TypeSupport& type,
            Topic* topic,
            const DataWriterQos& qos,
            DataWriterListener* listener = nullptr)
        : DataWriterImpl(p, type, topic, qos, listener)
    {
    }

    fastrtps::rtps::WriterListener* get_inner_listener()
    {
        return &writer_listener_;
    }

    void set_user_datawriter(
            DataWriter* writer)
    {
        user_datawriter_ = writer;
    }

};

// Mocked DataWriter with access to the DataWriterImpl
class DataWriterMock : public DataWriter
{
public:

    DataWriterMock(
            DataWriterImpl* impl,
            const StatusMask& mask = StatusMask::all())
        : DataWriter(impl, mask)
    {
    }

    DataWriterMock(
            Publisher* p,
            Topic* topic,
            const DataWriterQos& qos,
            DataWriterListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
        : DataWriter(p, topic, qos, listener, mask)
    {
    }

    DataWriterImplMock* get_implementation()
    {
        return dynamic_cast<DataWriterImplMock*>(impl_);
    }

};

// Mocked PublisherImpl with access to the inner members
class PublisherImplMock : public PublisherImpl
{
public:

    PublisherImplMock(
            DomainParticipantImpl* p,
            const PublisherQos& qos,
            PublisherListener* p_listen = nullptr)
        : PublisherImpl(p, qos, p_listen)
    {
    }

    DataWriterListener* get_inner_listener()
    {
        return &publisher_listener_;
    }

    void set_user_publisher(
            Publisher* pub)
    {
        user_publisher_ = pub;
    }

    void set_rtps_participant(
            fastrtps::rtps::RTPSParticipant* part)
    {
        rtps_participant_ = part;
    }

    void set_handle(
            fastrtps::rtps::InstanceHandle_t handle)
    {
        handle_ = handle;
    }

    DataWriterMock* create_datawriter(
            Topic* topic,
            const DataWriterQos& qos,
            DataWriterListener* listener,
            const StatusMask& mask)
    {
        //Look for the correct type registration
        TypeSupport type_support = participant_->find_type(topic->get_type_name());

        if (type_support.empty())
        {
            logError(PUBLISHER, "Type: " << topic->get_type_name() << " Not Registered");
            return nullptr;
        }

        topic->get_impl()->reference();

        DataWriterImplMock* impl = new DataWriterImplMock(
            this,
            type_support,
            topic,
            qos,
            listener);

        DataWriterMock* writer = new DataWriterMock(impl, mask);
        impl->set_user_datawriter(writer);

        {
            std::lock_guard<std::mutex> lock(mtx_writers_);
            writers_[topic->get_name()].push_back(impl);
        }

        if (user_publisher_->is_enabled() && qos_.entity_factory().autoenable_created_entities)
        {
            if (ReturnCode_t::RETCODE_OK != writer->enable())
            {
                delete_datawriter(writer);
                return nullptr;
            }
        }

        return writer;
    }

};

// Mocked Publisher with access to the PublisherImpl
class PublisherMock : public Publisher
{
public:

    PublisherMock(
            PublisherImpl* p,
            const StatusMask& mask = StatusMask::all())
        : Publisher(p, mask)
    {
    }

    PublisherMock(
            DomainParticipant* dp,
            const PublisherQos& qos = PUBLISHER_QOS_DEFAULT,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
        : Publisher(dp, qos, listener, mask)
    {
    }

    PublisherImplMock* get_implementation()
    {
        return dynamic_cast<PublisherImplMock*>(impl_);
    }

    DataWriterMock* create_datawriter(
            Topic* topic,
            const DataWriterQos& qos,
            DataWriterListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        return dynamic_cast<PublisherImplMock*>(impl_)->create_datawriter(topic, qos, listener, mask);
    }

};

// Mocked DomainParticipantImpl with access to the inner members
class DomainParticipantImplMock : public DomainParticipantImpl
{
public:

    DomainParticipantImplMock(
            DomainParticipant* dp,
            DomainId_t did,
            const DomainParticipantQos& qos,
            DomainParticipantListener* listen = nullptr)
        : DomainParticipantImpl(dp, did, qos, listen)
    {
    }

    fastrtps::rtps::RTPSParticipantListener* get_inner_listener()
    {
        return &rtps_listener_;
    }

    PublisherMock* create_publisher(
            const PublisherQos& qos,
            PublisherListener* listener,
            const StatusMask& mask)
    {
        //TODO CONSTRUIR LA IMPLEMENTACION DENTRO DEL OBJETO DEL USUARIO.
        PublisherImplMock* pubimpl = new PublisherImplMock(this, qos, listener);
        PublisherMock* pub = new PublisherMock(pubimpl, mask);
        pubimpl->set_user_publisher(pub);
        pubimpl->set_rtps_participant(rtps_participant_);

        bool enabled = rtps_participant_ != nullptr;

        // Create InstanceHandle for the new publisher
        fastrtps::rtps::InstanceHandle_t pub_handle;
        create_instance_handle(pub_handle);
        pubimpl->set_handle(pub_handle);

        //SAVE THE PUBLISHER INTO MAPS
        std::lock_guard<std::mutex> lock(mtx_pubs_);
        publishers_by_handle_[pub_handle] = pub;
        publishers_[pub] = pubimpl;

        // Enable publisher if appropriate
        if (enabled && qos_.entity_factory().autoenable_created_entities)
        {
            if (ReturnCode_t::RETCODE_OK != pub->enable())
            {
                delete_publisher(pub);
                return nullptr;
            }
        }

        return pub;
    }

    SubscriberMock* create_subscriber(
            const SubscriberQos& qos,
            SubscriberListener* listener,
            const StatusMask& mask)
    {
        //TODO CONSTRUIR LA IMPLEMENTACION DENTRO DEL OBJETO DEL USUARIO.
        SubscriberImplMock* subimpl = new SubscriberImplMock(this, qos, listener);
        SubscriberMock* sub = new SubscriberMock(subimpl, mask);
        subimpl->set_user_subscriber(sub);
        subimpl->set_rtps_participant(rtps_participant_);

        // Create InstanceHandle for the new subscriber
        fastrtps::rtps::InstanceHandle_t sub_handle;
        bool enabled = rtps_participant_ != nullptr;

        // Create InstanceHandle for the new subscriber
        create_instance_handle(sub_handle);
        subimpl->set_handle(sub_handle);

        //SAVE THE PUBLISHER INTO MAPS
        std::lock_guard<std::mutex> lock(mtx_subs_);
        subscribers_by_handle_[sub_handle] = sub;
        subscribers_[sub] = subimpl;

        // Enable subscriber if appropriate
        if (enabled && qos_.entity_factory().autoenable_created_entities)
        {
            if (ReturnCode_t::RETCODE_OK != sub->enable())
            {
                delete_subscriber(sub);
                return nullptr;
            }
        }

        return sub;
    }

};

// Mocked DomainParticipant with access to the DomainParticipantImpl
class DomainParticipantMock : public DomainParticipant
{
public:

    DomainParticipantMock(
            const StatusMask& mask = StatusMask::all())
        : DomainParticipant(mask)
    {
    }

    DomainParticipantMock(
            DomainId_t domain_id,
            const DomainParticipantQos& qos,
            DomainParticipantListener* listener,
            const StatusMask& mask = StatusMask::all())
        : DomainParticipant(domain_id, qos, listener, mask)
    {
    }

    DomainParticipantImplMock* get_implementation()
    {
        return dynamic_cast<DomainParticipantImplMock*>(impl_);
    }

    PublisherMock* create_publisher(
            const PublisherQos& qos,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        return dynamic_cast<DomainParticipantImplMock*>(impl_)->create_publisher(qos, listener, mask);
    }

    SubscriberMock* create_subscriber(
            const SubscriberQos& qos,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        return dynamic_cast<DomainParticipantImplMock*>(impl_)->create_subscriber(qos, listener, mask);
    }

};

// Mocked DomainParticipantFactory that creates DomainParticipantMock
class DomainParticipantFactoryMock : public DomainParticipantFactory
{
private:

    DomainParticipantFactoryMock()
    {
    }

public:

    DomainParticipantFactoryMock(
            const DomainParticipantFactoryMock&) = delete;

    void operator = (
            const DomainParticipantFactoryMock&) = delete;

    static DomainParticipantFactoryMock* get_instance()
    {
        static DomainParticipantFactoryMock instance;
        return &instance;
    }

    DomainParticipantMock* create_participant(
            DomainId_t did,
            const DomainParticipantQos& qos,
            DomainParticipantListener* listen,
            const StatusMask& mask = StatusMask::all())
    {
        const DomainParticipantQos& pqos = (&qos == &PARTICIPANT_QOS_DEFAULT) ? default_participant_qos_ : qos;

        DomainParticipantMock* dom_part = new DomainParticipantMock(mask);
        DomainParticipantImplMock* dom_part_impl = new DomainParticipantImplMock(dom_part, did, pqos, listen);

        {
            std::lock_guard<std::mutex> guard(mtx_participants_);
            using VectorIt = std::map<DomainId_t, std::vector<DomainParticipantImpl*> >::iterator;
            VectorIt vector_it = participants_.find(did);

            if (vector_it == participants_.end())
            {
                // Insert the vector
                std::vector<DomainParticipantImpl*> new_vector;
                auto pair_it = participants_.insert(std::make_pair(did, std::move(new_vector)));
                vector_it = pair_it.first;
            }

            vector_it->second.push_back(dom_part_impl);
        }

        if (factory_qos_.entity_factory().autoenable_created_entities)
        {
            if (ReturnCode_t::RETCODE_OK != dom_part->enable())
            {
                delete_participant(dom_part);
                return nullptr;
            }
        }

        return dom_part;
    }

    ReturnCode_t delete_participant(
            DomainParticipant* part)
    {
        return DomainParticipantFactory::delete_participant(part);
    }

};


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_ENTITY_MOCKS_HPP_ */


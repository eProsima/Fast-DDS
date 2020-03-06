/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef EPROSIMA_DDS_PUB_DATA_WRITER_IMPL_HPP_
#define EPROSIMA_DDS_PUB_DATA_WRITER_IMPL_HPP_

/***************************************************************************
*
* dds/pub/DataWriter<> WRAPPER implementation.
* Declaration can be found in dds/pub/DataWriter.hpp
*
***************************************************************************/

#include <dds/topic/Topic.hpp>

namespace dds {
namespace pub {

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>::DataWriter(
        const dds::pub::Publisher& pub,
        const dds::topic::Topic<T>& topic)
    : dds::core::Reference< DELEGATE<T> >(
        new DELEGATE<T>(pub,
        topic,
        pub.is_nil() ? dds::pub::qos::DataWriterQos() : pub.default_datawriter_qos(),
        NULL,
        dds::core::status::StatusMask::none()))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(pub);
    //    this->delegate()->init(this->impl_);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>::DataWriter(
        const dds::pub::Publisher& pub,
        const ::dds::topic::Topic<T>& topic,
        const dds::pub::qos::DataWriterQos& qos,
        dds::pub::DataWriterListener<T>* listener,
        const dds::core::status::StatusMask& mask)
    : dds::core::Reference< DELEGATE<T> >(
        new DELEGATE<T>(pub, topic, qos, listener, mask))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(pub);
    //    this->delegate()->init(this->impl_);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>::~DataWriter()
{
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const T& sample)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->write(sample);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->write(sample, timestamp);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const T& sample,
        const ::dds::core::InstanceHandle& instance)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->write(sample, instance);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const T& sample,
        const ::dds::core::InstanceHandle& instance,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->write(sample, instance, timestamp);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const dds::topic::TopicInstance<T>& i)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->write(i);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const dds::topic::TopicInstance<T>& i,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->write(i, timestamp);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename FWIterator>
void DataWriter<T, DELEGATE>::write(
        const FWIterator& begin,
        const FWIterator& end)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    //    FWIterator b = begin;
    //    while(b != end)
    //    {
    //        this->delegate()->write(*b);
    //        ++b;
    //    }
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename FWIterator>
void DataWriter<T, DELEGATE>::write(
        const FWIterator& begin,
        const FWIterator& end,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    //    FWIterator b = begin;
    //    while(b != end)
    //    {
    //        this->delegate()->write(*b, timestamp);
    //        ++b;
    //    }
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<
    typename SamplesFWIterator,
    typename HandlesFWIterator>
void DataWriter<T, DELEGATE>::write(
        const SamplesFWIterator& data_begin,
        const SamplesFWIterator& data_end,
        const HandlesFWIterator& handle_begin,
        const HandlesFWIterator& handle_end)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    //    SamplesFWIterator data = data_begin;
    //    HandlesFWIterator handle = handle_begin;

    //    while(data != data_end && handle != handle_end)
    //    {
    //        this->delegate()->write(*data, *handle);
    //        ++data;
    //        ++handle;
    //    }
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<
    typename SamplesFWIterator,
    typename HandlesFWIterator>
void DataWriter<T, DELEGATE>::write(
        const SamplesFWIterator& data_begin,
        const SamplesFWIterator& data_end,
        const HandlesFWIterator& handle_begin,
        const HandlesFWIterator& handle_end,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    //    SamplesFWIterator data = data_begin;
    //    HandlesFWIterator handle = handle_begin;

    //    while(data != data_end && handle != handle_end)
    //    {
    //        this->delegate()->write(*data, *handle, timestamp);
    //        ++data;
    //        ++handle;
    //    }
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        const ::dds::pub::qos::DataWriterQos& qos)
{
    //To implement
    //    this->delegate()->qos(qos);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        const T& data)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->write(data);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        const std::pair<T, dds::core::Time>& data)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->write(data.first, data.second);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        const std::pair<T, ::dds::core::InstanceHandle>& data)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->write(data.first, data.second);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        DataWriter& (*manipulator)(DataWriter&))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return manipulator(*this);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
const dds::core::InstanceHandle DataWriter<T, DELEGATE>::register_instance(
        const T& key)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    /* Invalid time will be used as current time. */
    //    return this->delegate()->register_instance(key, dds::core::Time::invalid());
}

template<
    typename T,
    template<typename Q> class DELEGATE>
const dds::core::InstanceHandle DataWriter<T, DELEGATE>::register_instance(
        const T& key,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->register_instance(key, timestamp);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(
        const ::dds::core::InstanceHandle& i)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    /* Invalid time will be used as current time. */
    //    this->delegate()->unregister_instance(i, dds::core::Time::invalid());
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(
        const ::dds::core::InstanceHandle& i,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->unregister_instance(i, timestamp);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(
        const T& key)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    /* Invalid time will be used as current time. */
    //    this->delegate()->unregister_instance(key, dds::core::Time::invalid());
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(
        const T& key,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->unregister_instance(key, timestamp);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(
        const ::dds::core::InstanceHandle& i)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    /* Invalid time will be used as current time. */
    //    this->delegate()->dispose_instance(i, dds::core::Time::invalid());
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(
        const ::dds::core::InstanceHandle& i,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->dispose_instance(i, timestamp);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(
        const T& key)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    /* Invalid time will be used as current time. */
    //    this->delegate()->dispose_instance(key, dds::core::Time::invalid());
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(
        const T& key,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->dispose_instance(key, timestamp);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
dds::topic::TopicInstance<T>& DataWriter<T, DELEGATE>::key_value(
        dds::topic::TopicInstance<T>& i,
        const ::dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->key_value(i, h);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
T& DataWriter<T, DELEGATE>::key_value(
        T& sample,
        const ::dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->key_value(sample, h);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
dds::core::InstanceHandle DataWriter<T, DELEGATE>::lookup_instance(
        const T& key)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->lookup_instance(key);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
const dds::topic::Topic<T>& DataWriter<T, DELEGATE>::topic() const
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->topic();
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::listener(
        DataWriterListener<T>* listener,
        const ::dds::core::status::StatusMask& mask)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->listener(listener, mask);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataWriterListener<T>* DataWriter<T, DELEGATE>::listener() const
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->listener();
}

} //namespace dds
} //namespace pub




/***************************************************************************
*
* dds/pub/detail/DataWriter<> DELEGATE implementation.
* Declaration can be found in dds/pub/detail/DataWriter.hpp
*
* Implementation and declaration have been separated because some circular
* dependencies, like with DataWriterListener and AnyDataWriter.
*
***************************************************************************/

/**
 * @cond
 * Do not add the delegate to the API documentation.
 */

#include <dds/pub/AnyDataWriter.hpp>
#include <dds/pub/DataWriterListener.hpp>
//TODO: Fix when AnyDataWriterDelegate is implemented
//#include <org/opensplice/pub/AnyDataWriterDelegate.hpp>

template<typename T>
dds::pub::detail::DataWriter<T>::DataWriter(
        const dds::pub::Publisher& pub,
        const ::dds::topic::Topic<T>& topic,
        const dds::pub::qos::DataWriterQos& qos,
        dds::pub::DataWriterListener<T>* listener,
        const dds::core::status::StatusMask& mask)
//    : ::org::opensplice::pub::AnyDataWriterDelegate(qos, topic), pub_(pub), topic_(topic)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(pub);

    //    if (dds::topic::is_topic_type<T>::value == 0) {
    //        ISOCPP_THROW_EXCEPTION(ISOCPP_PRECONDITION_NOT_MET_ERROR, "DataWriter cannot be created, topic information not found");
    //    }

    //    /* Create a implicit publisher with the topic participant when needed. */
    //    if (pub_.is_nil()) {
    //        pub_ = dds::pub::Publisher(topic->domain_participant());
    //    }

    //    /* Merge the topic QoS implicitly when needed. */
    //    if (topic.qos()->force_merge()) {
    //        qos_ = topic.qos();
    //    }

    //    org::opensplice::pub::qos::DataWriterQosDelegate dwQos = qos_.delegate();

    //    // get and validate the kernel qos
    //    dwQos.check();
    //    u_writerQos uQos = dwQos.u_qos();

    //    u_publisher uPublisher = (u_publisher)(pub_.delegate()->get_user_handle());
    //    u_topic uTopic = (u_topic)(topic.delegate()->get_user_handle());

    //    std::string name = "writer <" + topic.name() + ">";

    //    u_writer uWriter = u_writerNew(uPublisher, name.c_str(), uTopic, uQos);
    //    u_writerQosFree(uQos);

    //    if (!uWriter) {
    //        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Failed to create DataWriter");
    //    } else {
    //        topic_.delegate()->incrNrDependents();
    //    }

    //    this->setCopyIn(org::opensplice::topic::TopicTraits<T>::getCopyIn());
    //    this->setCopyOut(org::opensplice::topic::TopicTraits<T>::getCopyOut());

    //    this->userHandle = (u_object)uWriter;
    //    this->listener_set((void*)listener, mask);
    //    this->set_domain_id(this->pub_.delegate()->get_domain_id());
}

template<typename T>
dds::pub::detail::DataWriter<T>::~DataWriter()
{
    //To implement
    //    if (!this->closed) {
    //        try {
    //            this->close();
    //        } catch (...) {
    //            /* Empty: the exception throw should have already traced an error. */
    //        }
    //    }
}

template<typename T>
void dds::pub::detail::DataWriter<T>::init(
        ObjectDelegate::weak_ref_type weak_ref)
{
    //To implement
    //    /* Set weak_ref before passing ourselves to other isocpp objects. */
    //    this->set_weak_ref(weak_ref);
    //    /* Register writer at publisher. */
    //    this->pub_.delegate()->add_datawriter(*this);
    //    /* Use listener dispatcher from the publisher. */
    //    this->listener_dispatcher_set(this->pub_.delegate()->listener_dispatcher_get());
    //    /* This only starts listening when the status mask shows interest. */
    //    this->listener_enable();
    //    /* Enable when needed. */
    //    if (this->pub_.delegate()->is_enabled() && this->pub_.delegate()->is_auto_enable()) {
    //        this->enable();
    //    }
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const T& sample)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::write((u_writer)(this->userHandle),
    //                                  &sample,
    //                                  dds::core::InstanceHandle(dds::core::null),
    //                                  dds::core::Time::invalid());
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::write((u_writer)(this->userHandle),
    //                                  &sample,
    //                                  dds::core::InstanceHandle(dds::core::null),
    //                                  timestamp);
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const T& sample,
        const ::dds::core::InstanceHandle& instance)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::write((u_writer)(this->userHandle),
    //                                  &sample,
    //                                  instance,
    //                                  dds::core::Time::invalid());
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const T& sample,
        const ::dds::core::InstanceHandle& instance,
        const dds::core::Time& timestamp)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::write((u_writer)(this->userHandle),
    //                                  &sample,
    //                                  instance,
    //                                  timestamp);
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const dds::topic::TopicInstance<T>& i)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::write((u_writer)(this->userHandle),
    //                                  &i.sample(),
    //                                  i.handle(),
    //                                  dds::core::Time::invalid());
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const dds::topic::TopicInstance<T>& i,
        const dds::core::Time& timestamp)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::write((u_writer)(this->userHandle),
    //                                  &i.sample(),
    //                                  i.handle(),
    //                                  timestamp);
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const T& sample)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    //    this->check();
    //    AnyDataWriterDelegate::writedispose(
    //                                  (u_writer)(this->userHandle),
    //                                  &sample,
    //                                  dds::core::InstanceHandle(dds::core::null),
    //                                  dds::core::Time::invalid());
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    //    this->check();
    //    AnyDataWriterDelegate::writedispose(
    //                                  (u_writer)(this->userHandle),
    //                                  &sample,
    //                                  dds::core::InstanceHandle(dds::core::null),
    //                                  timestamp);
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const T& sample,
        const ::dds::core::InstanceHandle& instance)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    //    this->check();
    //    AnyDataWriterDelegate::writedispose(
    //                                  (u_writer)(this->userHandle),
    //                                  &sample,
    //                                  instance,
    //                                  dds::core::Time::invalid());
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const T& sample,
        const ::dds::core::InstanceHandle& instance,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    //    this->check();
    //    AnyDataWriterDelegate::writedispose(
    //                                  (u_writer)(this->userHandle),
    //                                  &sample,
    //                                  instance,
    //                                  timestamp);
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const dds::topic::TopicInstance<T>& i)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    //    this->check();
    //    AnyDataWriterDelegate::writedispose(
    //                                  (u_writer)(this->userHandle),
    //                                  &i.sample(),
    //                                  i.handle(),
    //                                  dds::core::Time::invalid());
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const dds::topic::TopicInstance<T>& i,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    //    this->check();
    //    AnyDataWriterDelegate::writedispose(
    //                                  (u_writer)(this->userHandle),
    //                                  &i.sample(),
    //                                  i.handle(),
    //                                  timestamp);
}

template<typename T>
template<typename FWIterator>
void dds::pub::detail::DataWriter<T>::writedispose(
        const FWIterator& begin,
        const FWIterator& end)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    //    FWIterator b = begin;
    //    while(b != end)
    //    {
    //        this->writedispose(*b);
    //        ++b;
    //    }
}

template<typename T>
template<typename FWIterator>
void dds::pub::detail::DataWriter<T>::writedispose(
        const FWIterator& begin,
        const FWIterator& end,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    //    FWIterator b = begin;
    //    while(b != end)
    //    {
    //        this->writedispose(*b, timestamp);
    //        ++b;
    //    }
}

template<typename T>
template<
    typename SamplesFWIterator,
    typename HandlesFWIterator>
void dds::pub::detail::DataWriter<T>::writedispose(
        const SamplesFWIterator& data_begin,
        const SamplesFWIterator& data_end,
        const HandlesFWIterator& handle_begin,
        const HandlesFWIterator& handle_end)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    //    SamplesFWIterator data = data_begin;
    //    HandlesFWIterator handle = handle_begin;

    //    while(data != data_end && handle != handle_end)
    //    {
    //        this->writedispose(*data, *handle);
    //        ++data;
    //        ++handle;
    //    }
}

template<typename T>
template<
    typename SamplesFWIterator,
    typename HandlesFWIterator>
void dds::pub::detail::DataWriter<T>::writedispose(
        const SamplesFWIterator& data_begin,
        const SamplesFWIterator& data_end,
        const HandlesFWIterator& handle_begin,
        const HandlesFWIterator& handle_end,
        const dds::core::Time& timestamp)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    //    SamplesFWIterator data = data_begin;
    //    HandlesFWIterator handle = handle_begin;

    //    while(data != data_end && handle != handle_end)
    //    {
    //        this->writedispose(*data, *handle, timestamp);
    //        ++data;
    //        ++handle;
    //    }
}

template<typename T>
const ::dds::core::InstanceHandle dds::pub::detail::DataWriter<T>::register_instance(
        const T& key,
        const dds::core::Time& timestamp)
{
    //To implement
    //    this->check();
    //    dds::core::InstanceHandle handle(AnyDataWriterDelegate::register_instance((u_writer)(this->userHandle), &key, timestamp));
    //    return handle;
}

template<typename T>
void dds::pub::detail::DataWriter<T>::unregister_instance(
        const ::dds::core::InstanceHandle& handle,
        const dds::core::Time& timestamp)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::unregister_instance((u_writer)(this->userHandle), handle, timestamp);
}

template<typename T>
void dds::pub::detail::DataWriter<T>::unregister_instance(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::unregister_instance((u_writer)(this->userHandle), &sample, timestamp);
}

template<typename T>
void dds::pub::detail::DataWriter<T>::dispose_instance(
        const ::dds::core::InstanceHandle& handle,
        const dds::core::Time& timestamp)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::dispose_instance((u_writer)(this->userHandle), handle, timestamp);
}

template<typename T>
void dds::pub::detail::DataWriter<T>::dispose_instance(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::dispose_instance((u_writer)(this->userHandle), &sample, timestamp);
}

template<typename T>
dds::topic::TopicInstance<T>&dds::pub::detail::DataWriter<T>::key_value(
        dds::topic::TopicInstance<T>& i,
        const ::dds::core::InstanceHandle& h)
{
    //To implement
    //    T sample;
    //    AnyDataWriterDelegate::get_key_value((u_writer)(this->userHandle), &sample, h);
    //    i.handle(h);
    //    i.sample(sample);

    //    return i;
}

template<typename T>
T & dds::pub::detail::DataWriter<T>::key_value(
        T& sample,
        const ::dds::core::InstanceHandle& h)
{
    //To implement
    //    this->check();
    //    AnyDataWriterDelegate::get_key_value((u_writer)(this->userHandle), &sample, h);
    //    return sample;
}

template<typename T>
dds::core::InstanceHandle dds::pub::detail::DataWriter<T>::lookup_instance(
        const T& key)
{
    //To implement
    //    this->check();
    //    dds::core::InstanceHandle handle(AnyDataWriterDelegate::lookup_instance((u_writer)(this->userHandle), &key));
    //    return handle;
}

template<typename T>
const dds::topic::Topic<T>&dds::pub::detail::DataWriter<T>::topic() const
{
    //To implement
    //    return this->topic_;
}

template<typename T>
const dds::pub::Publisher& dds::pub::detail::DataWriter<T>::publisher() const
{
    //To implement
    //    return this->pub_;
}

template<typename T>
void dds::pub::detail::DataWriter<T>::listener(
        DataWriterListener<T>* listener,
        const ::dds::core::status::StatusMask& mask)
{
    //To implement
    //    /* EntityDelegate takes care of thread safety. */
    //    this->listener_set((void*)listener, mask);
    //    this->listener_enable();
}

template<typename T>
dds::pub::DataWriterListener<T>* dds::pub::detail::DataWriter<T>::listener() const
{
    //To implement
    //    return reinterpret_cast<dds::pub::DataWriterListener<T>*>(this->listener_get());
}

template<typename T>
void dds::pub::detail::DataWriter<T>::close()
{
    //To implement
    //    this->listener(NULL, dds::core::status::StatusMask::none());
    //    this->listener_dispatcher_reset();
    //    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    //    topic_.delegate()->decrNrDependents();
    //    this->pub_.delegate()->remove_datawriter(*this);

    //    org::opensplice::pub::AnyDataWriterDelegate::close();

    //    scopedLock.unlock();
}

template<typename T>
dds::pub::DataWriter<T, dds::pub::detail::DataWriter> dds::pub::detail::DataWriter<T>::wrapper()
{
    //To implement
    //    typename DataWriter::ref_type ref =
    //            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DataWriter<T> >(this->get_strong_ref());
    //    dds::pub::DataWriter<T, dds::pub::detail::DataWriter> writer(ref);

    //    return writer;
}

template<typename T>
void dds::pub::detail::DataWriter<T>::listener_notify(
        ObjectDelegate::ref_type source,
        uint32_t triggerMask,
        void* eventData,
        void* l)
{
    //To implement
    //    /* The EntityDelegate takes care of the thread safety and always
    //    /* provides a listener and source. */
    //    dds::pub::DataWriterListener<T>* listener =
    //            reinterpret_cast<dds::pub::DataWriterListener<T>*>(l);
    //    assert(listener);

    //    /* Get DataWriter wrapper from given source EntityDelegate. */
    //    typename DataWriter::ref_type ref =
    //            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DataWriter<T> >(source);
    //    dds::pub::DataWriter<T, dds::pub::detail::DataWriter> writer(ref->wrapper());


    //    if (triggerMask & V_EVENT_LIVELINESS_LOST) {
    //        dds::core::status::LivelinessLostStatus status;
    //        status.delegate().v_status(v_writerStatus(eventData)->livelinessLost);
    //        listener->on_liveliness_lost(writer, status);
    //    }

    //    if (triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
    //        dds::core::status::OfferedDeadlineMissedStatus status;
    //        status.delegate().v_status(v_writerStatus(eventData)->deadlineMissed);
    //        listener->on_offered_deadline_missed(writer, status);
    //    }

    //    if (triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
    //        dds::core::status::OfferedIncompatibleQosStatus status;
    //        status.delegate().v_status(v_writerStatus(eventData)->incompatibleQos);
    //        listener->on_offered_incompatible_qos(writer, status);
    //    }

    //    if (triggerMask & V_EVENT_PUBLICATION_MATCHED) {
    //        dds::core::status::PublicationMatchedStatus status;
    //        status.delegate().v_status(v_writerStatus(eventData)->publicationMatch);
    //        listener->on_publication_matched(writer, status);
    //    }
}

/** @endcond */

#endif //EPROSIMA_DDS_PUB_DATA_WRITER_IMPL_HPP_

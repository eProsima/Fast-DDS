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
}

template<
        typename T,
        template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const T& sample,
        const ::dds::core::InstanceHandle& instance)
{
    //To implement
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
}

template<
        typename T,
        template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const dds::topic::TopicInstance<T>& i)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(
        const dds::topic::TopicInstance<T>& i,
        const dds::core::Time& timestamp)
{
    //To implement
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
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        const ::dds::pub::qos::DataWriterQos& qos)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        const T& data)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        const std::pair<T, dds::core::Time>& data)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        const std::pair<T, ::dds::core::InstanceHandle>& data)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(
        DataWriter & (*manipulator)(DataWriter&))
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
const dds::core::InstanceHandle DataWriter<T, DELEGATE>::register_instance(
        const T& key)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
const dds::core::InstanceHandle DataWriter<T, DELEGATE>::register_instance(
        const T& key,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(
        const ::dds::core::InstanceHandle& i)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(
        const ::dds::core::InstanceHandle& i,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(
        const T& key)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(const T& key,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(
        const ::dds::core::InstanceHandle& i)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(
        const ::dds::core::InstanceHandle& i,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(
        const T& key)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(
        const T& key,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
dds::topic::TopicInstance<T>& DataWriter<T, DELEGATE>::key_value(
        dds::topic::TopicInstance<T>& i,
        const ::dds::core::InstanceHandle& h)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
T& DataWriter<T, DELEGATE>::key_value(
        T& sample,
        const ::dds::core::InstanceHandle& h)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
dds::core::InstanceHandle DataWriter<T, DELEGATE>::lookup_instance(
        const T& key)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
const dds::topic::Topic<T>& DataWriter<T, DELEGATE>::topic() const
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::listener(
        DataWriterListener<T>* listener,
        const ::dds::core::status::StatusMask& mask)
{
    //To implement
}

template<
        typename T,
        template<typename Q> class DELEGATE>
DataWriterListener<T>* DataWriter<T, DELEGATE>::listener() const
{
    //To implement
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
}

template<typename T>
dds::pub::detail::DataWriter<T>::~DataWriter()
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::init(
        ObjectDelegate::weak_ref_type weak_ref)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const T& sample)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const T& sample,
        const ::dds::core::InstanceHandle& instance)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const T& sample,
        const ::dds::core::InstanceHandle& instance,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const dds::topic::TopicInstance<T>& i)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::write(
        const dds::topic::TopicInstance<T>& i,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const T& sample)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
        const T& sample,
        const ::dds::core::InstanceHandle& instance)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
           const T& sample,
           const ::dds::core::InstanceHandle& instance,
           const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(const dds::topic::TopicInstance<T>& i)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::writedispose(
           const dds::topic::TopicInstance<T>& i,
           const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
template<typename FWIterator>
void dds::pub::detail::DataWriter<T>::writedispose(
        const FWIterator& begin,
        const FWIterator& end)
{
    //To implement
}

template<typename T>
template<typename FWIterator>
void dds::pub::detail::DataWriter<T>::writedispose(
        const FWIterator& begin,
        const FWIterator& end,
        const dds::core::Time& timestamp)
{
    //To implement
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
}

template<typename T>
const ::dds::core::InstanceHandle dds::pub::detail::DataWriter<T>::register_instance(
        const T& key,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::unregister_instance(
        const ::dds::core::InstanceHandle& handle,
         const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::unregister_instance(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::dispose_instance(
        const ::dds::core::InstanceHandle& handle,
        const dds::core::Time& timestamp)
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::dispose_instance(
        const T& sample,
        const dds::core::Time& timestamp)
{
    //To implement
}


template<typename T>
dds::topic::TopicInstance<T>& dds::pub::detail::DataWriter<T>::key_value(
        dds::topic::TopicInstance<T>& i,
        const ::dds::core::InstanceHandle& h)
{
    //To implement
}

template<typename T>
T& dds::pub::detail::DataWriter<T>::key_value(
        T& sample,
        const ::dds::core::InstanceHandle& h)
{
    //To implement
}

template<typename T>
dds::core::InstanceHandle dds::pub::detail::DataWriter<T>::lookup_instance(
        const T& key)
{
    //To implement
}

template<typename T>
const dds::topic::Topic<T>& dds::pub::detail::DataWriter<T>::topic() const
{
    //To implement
}

template<typename T>
const dds::pub::Publisher& dds::pub::detail::DataWriter<T>::publisher() const
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::listener(
        DataWriterListener<T>* listener,
        const ::dds::core::status::StatusMask& mask)
{
    //To implement
}

template<typename T>
dds::pub::DataWriterListener<T>* dds::pub::detail::DataWriter<T>::listener() const
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::close()
{
    //To implement
}

template<typename T>
dds::pub::DataWriter<T, dds::pub::detail::DataWriter> dds::pub::detail::DataWriter<T>::wrapper()
{
    //To implement
}

template<typename T>
void dds::pub::detail::DataWriter<T>::listener_notify(
        ObjectDelegate::ref_type source,
        uint32_t triggerMask,
        void *eventData,
        void *l)
{
    //To implement
}

/** @endcond */

#endif //EPROSIMA_DDS_PUB_DATA_WRITER_IMPL_HPP_

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

#ifndef EPROSIMA_DDS_SUB_TDATAREADER_IMPL_HPP_
#define EPROSIMA_DDS_SUB_TDATAREADER_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/detail/DataReader.hpp>
#include <dds/sub/Query.hpp>
#include <dds/sub/detail/SamplesHolder.hpp>

/***************************************************************************
*
* dds/sub/DataReader<> WRAPPER implementation.
* Declaration can be found in dds/sub/TDataReader.hpp
*
***************************************************************************/

// Implementation

namespace dds {
namespace sub {

//--------------------------------------------------------------------------------
//  DATAREADER
//--------------------------------------------------------------------------------

template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::Selector::Selector(
        DataReader& dr)
    : impl_(dr.delegate())
{
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector&
DataReader<T, DELEGATE>::Selector::instance(
    const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.instance(h);
    //    return *this;
}


template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector& DataReader<T, DELEGATE>::Selector::next_instance(
    const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.next_instance(h);
    //    return *this;
}


template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector& DataReader<T, DELEGATE>::Selector::state(
    const dds::sub::status::DataState& s)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.filter_state(s);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector& DataReader<T, DELEGATE>::Selector::content(
    const dds::sub::Query& query)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.filter_content(query);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector& DataReader<T, DELEGATE>::Selector::max_samples(
    uint32_t n)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.max_samples(n);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
dds::sub::LoanedSamples<T> DataReader<T, DELEGATE>::Selector::read()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    return impl_.read();
}

template<
    typename T,
    template<typename Q> class DELEGATE>
dds::sub::LoanedSamples<T> DataReader<T, DELEGATE>::Selector::take()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    return impl_.take();
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename SamplesFWIterator>
uint32_t DataReader<T, DELEGATE>::Selector::read(
    SamplesFWIterator sfit,
    uint32_t max_samples)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    return impl_.read(sfit, max_samples);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename SamplesFWIterator>
uint32_t DataReader<T, DELEGATE>::Selector::take(
    SamplesFWIterator sfit,
    uint32_t max_samples)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    return impl_.take(sfit, max_samples);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename SamplesBIIterator>
uint32_t DataReader<T, DELEGATE>::Selector::read(
    SamplesBIIterator sbit)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    return impl_.read(sbit);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename SamplesBIIterator>
uint32_t DataReader<T, DELEGATE>::Selector::take(
    SamplesBIIterator sbit)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    return impl_.take(sbit);
}

//--------------------------------------------------------------------------------
//  DATAREADER::MANIPULATORSELECTOR
//--------------------------------------------------------------------------------
template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::ManipulatorSelector::ManipulatorSelector(
        DataReader& dr)
    : impl_(dr.delegate())
{
}

template<
    typename T,
    template<typename Q> class DELEGATE>
bool DataReader<T, DELEGATE>::ManipulatorSelector::read_mode()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    return impl_.read_mode();
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataReader<T, DELEGATE>::ManipulatorSelector::read_mode(
    bool b)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    impl_.read_mode(b);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::instance(
    const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.instance(h);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::next_instance(
    const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.next_instance(h);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::operator >>(
        dds::sub::LoanedSamples<T>& samples)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_ >> samples;
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::operator >>(
        ManipulatorSelector& (manipulator)(ManipulatorSelector &))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    manipulator(*this);
    //    return *this;
}

/** @cond */
template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename Functor>
typename DataReader<T, DELEGATE>::ManipulatorSelector DataReader<T, DELEGATE>::ManipulatorSelector::operator >>(
        Functor f)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    f(*this);
    //    return *this;
}

/** @endcond */

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::state(
    const dds::sub::status::DataState& s)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.filter_state(s);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::content(
    const dds::sub::Query& query)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.filter_content(query);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::max_samples(
    uint32_t n)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.max_samples(n);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(
        const dds::sub::Subscriber& sub,
        const dds::topic::Topic<T>& topic)
    : ::dds::core::Reference< DELEGATE<T> >(
        new DELEGATE<T>(sub, topic, sub.is_nil() ? dds::sub::qos::DataReaderQos() : sub->default_datareader_qos()))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);
    //    this->delegate()->init(this->impl_);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(
        const dds::sub::Subscriber& sub,
        const ::dds::topic::Topic<T>& topic,
        const dds::sub::qos::DataReaderQos& qos,
        dds::sub::DataReaderListener<T>* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(sub, topic, qos, listener, mask))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);
    //    this->delegate()->init(this->impl_);
}

#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT
template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(
        const dds::sub::Subscriber& sub,
        const dds::topic::ContentFilteredTopic<T>& topic)
    : ::dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(sub, topic, sub.default_datareader_qos()))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);
    //    this->delegate()->init(this->impl_);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(
        const dds::sub::Subscriber& sub,
        const ::dds::topic::ContentFilteredTopic<T>& topic,
        const dds::sub::qos::DataReaderQos& qos,
        dds::sub::DataReaderListener<T>* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(sub, topic, qos, listener, mask))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);
    //    this->delegate()->init(this->impl_);
}

#endif //OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT
template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(
        const dds::sub::Subscriber& sub,
        const dds::topic::MultiTopic<T>& topic)
    : ::dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(sub, topic))
{
    //To implement
    //    this->delegate()->init(this->impl_);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(
        const dds::sub::Subscriber& sub,
        const ::dds::topic::MultiTopic<T>& topic,
        const dds::sub::qos::DataReaderQos& qos,
        dds::sub::DataReaderListener<T>* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(sub, topic, qos, listener, mask))
{
    //To implement
    //    this->delegate()->init(this->impl_);
}

#endif //OMG_DDS_MULTI_TOPIC_SUPPORT

template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>::~DataReader()
{
}

template<
    typename T,
    template<typename Q> class DELEGATE>
dds::sub::status::DataState
DataReader<T, DELEGATE>::default_filter_state()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->default_filter_state();
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>& DataReader<T, DELEGATE>::default_filter_state(
        const dds::sub::status::DataState& status)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->default_filter_state(status);
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
DataReader<T, DELEGATE>& DataReader<T, DELEGATE>::operator >>(
        dds::sub::LoanedSamples<T>& ls)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    //    ls = this->read();
    //    return *this;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector DataReader<T, DELEGATE>::operator >>(
        ManipulatorSelector& (manipulator)(ManipulatorSelector &))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    //    ManipulatorSelector selector(*this);
    //    manipulator(selector);
    //    return selector;
}

/** @cond */
template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename Functor>
typename DataReader<T, DELEGATE>::ManipulatorSelector DataReader<T, DELEGATE>::operator >>(
        Functor f)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    //    ManipulatorSelector selector(*this);
    //    f(selector);
    //    return selector;
}

/** @endcond */

template<
    typename T,
    template<typename Q> class DELEGATE>
LoanedSamples<T> DataReader<T, DELEGATE>::read()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->read();
}

template<
    typename T,
    template<typename Q> class DELEGATE>
LoanedSamples<T> DataReader<T, DELEGATE>::take()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->take();
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename SamplesFWIterator>
uint32_t DataReader<T, DELEGATE>::read(
        SamplesFWIterator sfit,
        uint32_t max_samples)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->read(sfit, max_samples);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename SamplesFWIterator>
uint32_t DataReader<T, DELEGATE>::take(
        SamplesFWIterator sfit,
        uint32_t max_samples)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->take(sfit, max_samples);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename SamplesBIIterator>
uint32_t DataReader<T, DELEGATE>::read(
        SamplesBIIterator sbit)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->read(sbit);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
template<typename SamplesBIIterator>
uint32_t DataReader<T, DELEGATE>::take(
        SamplesBIIterator sbit)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->take(sbit);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector DataReader<T, DELEGATE>::select()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    //    Selector selector(*this);
    //    return selector;
}

template<
    typename T,
    template<typename Q> class DELEGATE>
dds::topic::TopicInstance<T> DataReader<T, DELEGATE>::key_value(
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->key_value(h);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
T& DataReader<T, DELEGATE>::key_value(
        T& sample,
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->key_value(sample, h);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
const dds::core::InstanceHandle DataReader<T, DELEGATE>::lookup_instance(
        const T& key) const
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->lookup_instance(key);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
void DataReader<T, DELEGATE>::listener(
        Listener* listener,
        const dds::core::status::StatusMask& event_mask)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->listener(listener, event_mask);
}

template<
    typename T,
    template<typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Listener*
DataReader<T, DELEGATE>::listener() const
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->listener();
}

} // namespace sub
} // namespace dds




/***************************************************************************
*
* dds/sub/detail/DataReader<> DELEGATE implementation.
* Declaration can be found in dds/sub/detail/DataReader.hpp
*
* Implementation and declaration have been separated because some circular
* dependencies, like with DataReaderListener and AnyDataReader.
*
***************************************************************************/

#include <dds/sub/AnyDataReader.hpp>
#include <dds/sub/DataReaderListener.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/topic/ContentFilteredTopic.hpp>


template<typename T>
dds::sub::detail::DataReader<T>::DataReader(
        const dds::sub::Subscriber& sub,
        const dds::topic::Topic<T>& topic,
        const dds::sub::qos::DataReaderQos& qos,
        dds::sub::DataReaderListener<T>* listener,
        const dds::core::status::StatusMask& mask)
    : ::org::opensplice::sub::AnyDataReaderDelegate(qos, topic)
    , sub_(sub)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(topic);

    //    /* Create a implicit subscriber with the topic participant when needed. */
    //    if (sub_.is_nil()) {
    //        sub_ = dds::sub::Subscriber(topic->domain_participant());
    //    }

    //    /* Merge the topic QoS implicitly when needed. */
    //    if (topic.qos()->force_merge()) {
    //        qos_ = topic.qos();
    //    }

    //    common_constructor(listener, mask);
}

template<typename T>
dds::sub::detail::DataReader<T>::DataReader(
        const dds::sub::Subscriber& sub,
        const dds::topic::ContentFilteredTopic<T, dds::topic::detail::ContentFilteredTopic>& topic,
        const dds::sub::qos::DataReaderQos& qos,
        dds::sub::DataReaderListener<T>* listener,
        const dds::core::status::StatusMask& mask)
    : ::org::opensplice::sub::AnyDataReaderDelegate(qos, topic)
    , sub_(sub)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(topic);

    //    /* Create a implicit subscriber with the topic participant when needed. */
    //    if (sub_.is_nil()) {
    //        sub_ = dds::sub::Subscriber(topic->domain_participant());
    //    }

    //    /* Merge the topic QoS implicitly when needed. */
    //    if (topic.topic().qos()->force_merge()) {
    //        qos_ = topic.topic().qos();
    //    }

    //    common_constructor(listener, mask);
}

template<typename T>
void dds::sub::detail::DataReader<T>::common_constructor(
        dds::sub::DataReaderListener<T>* listener,
        const dds::core::status::StatusMask& mask)
{
    //To implement
    //    if (dds::topic::is_topic_type<T>::value == 0) {
    //        ISOCPP_THROW_EXCEPTION(ISOCPP_PRECONDITION_NOT_MET_ERROR, "DataReader cannot be created, topic information not found");
    //    }

    //    org::opensplice::sub::qos::DataReaderQosDelegate drQos = qos_.delegate();

    //    // get and validate the kernel qos
    //    drQos.check();
    //    u_readerQos uQos = drQos.u_qos();

    //    u_subscriber uSubscriber = (u_subscriber)(sub_.delegate()->get_user_handle());

    //    std::string expression = this->AnyDataReaderDelegate::td_.delegate()->reader_expression();
    //    std::vector<c_value> params = this->AnyDataReaderDelegate::td_.delegate()->reader_parameters();

    //    std::string name = "reader <" + this->AnyDataReaderDelegate::td_.name() + ">";
    //    u_dataReader uReader = u_dataReaderNew(uSubscriber, name.c_str(), expression.c_str(),
    //            params.empty() ? NULL : &params[0], params.size(), uQos);
    //    u_readerQosFree(uQos);

    //    if (!uReader) {
    //        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Failed to create DataReader");
    //    } else {
    //        this->AnyDataReaderDelegate::td_.delegate()->incrNrDependents();
    //    }

    //    this->AnyDataReaderDelegate::setCopyOut(org::opensplice::topic::TopicTraits<T>::getCopyOut());
    //    this->AnyDataReaderDelegate::setCopyIn(org::opensplice::topic::TopicTraits<T>::getCopyIn());

    //    this->userHandle = u_object(uReader);
    //    this->listener_set((void*)listener, mask);
    //    this->set_domain_id(this->sub_.delegate()->get_domain_id());
}

template<typename T>
dds::sub::detail::DataReader<T>::~DataReader()
{
    //To implement
    //    if (!this->closed) {
    //        try {
    //            close();
    //        } catch (...) {

    //        }
    //    }
}

template<typename T>
void
dds::sub::detail::DataReader<T>::init(
        ObjectDelegate::weak_ref_type weak_ref)
{
    //To implement
    //    /* Set weak_ref before passing ourselves to other isocpp objects. */
    //    this->set_weak_ref(weak_ref);
    //    /* Register writer at publisher. */
    //    this->sub_.delegate()->add_datareader(*this);
    //    /* Use listener dispatcher from the publisher. */
    //    this->listener_dispatcher_set(this->sub_.delegate()->listener_dispatcher_get());
    //    /* This only starts listening when the status mask shows interest. */
    //    this->listener_enable();
    //    /* Enable when needed. */
    //    if (this->sub_.delegate()->is_enabled() && this->sub_.delegate()->is_auto_enable()) {
    //        this->enable();
    //    }
}

template<typename T>
dds::sub::status::DataState dds::sub::detail::DataReader<T>::default_filter_state()
{
    //To implement
    //    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    //    dds::sub::status::DataState state = this->status_filter_;

    //    scopedLock.unlock();

    //    return state;
}

template<typename T>
void dds::sub::detail::DataReader<T>::default_filter_state(
        const dds::sub::status::DataState& state)
{
    //To implement
    //    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    //    this->status_filter_ = state;

    //    scopedLock.unlock();
}

template<typename T>
dds::sub::LoanedSamples<T>
dds::sub::detail::DataReader<T>::read()
{
    //To implement
    //    dds::sub::LoanedSamples<T> samples;
    //    dds::sub::detail::LoanedSamplesHolder<T> holder(samples);

    //    this->AnyDataReaderDelegate::read((u_dataReader)(this->userHandle), this->status_filter_, holder, -1);

    //    return samples;
}

template<typename T>
dds::sub::LoanedSamples<T> dds::sub::detail::DataReader<T>::take()
{
    //To implement
    //    dds::sub::LoanedSamples<T> samples;
    //    dds::sub::detail::LoanedSamplesHolder<T> holder(samples);

    //    this->AnyDataReaderDelegate::take((u_dataReader)(this->userHandle), this->status_filter_, holder, -1);

    //    return samples;
}

template<typename T>
template<typename SamplesFWIterator>
uint32_t dds::sub::detail::DataReader<T>::read(
        SamplesFWIterator samples,
        uint32_t max_samples)
{
    //To implement
    //    dds::sub::detail::SamplesFWInteratorHolder<T, SamplesFWIterator> holder(samples);

    //    this->AnyDataReaderDelegate::read((u_dataReader)(this->userHandle), this->status_filter_, holder, max_samples);

    //    return holder.get_length();
}

template<typename T>
template<typename SamplesFWIterator>
uint32_t dds::sub::detail::DataReader<T>::take(
        SamplesFWIterator samples,
        uint32_t max_samples)
{
    //To implement
    //    dds::sub::detail::SamplesFWInteratorHolder<T, SamplesFWIterator> holder(samples);

    //    this->AnyDataReaderDelegate::take((u_dataReader)(this->userHandle), this->status_filter_, holder, max_samples);

    //    return holder.get_length();
}

template<typename T>
template<typename SamplesBIIterator>
uint32_t dds::sub::detail::DataReader<T>::read(
        SamplesBIIterator samples)
{
    //To implement
    //    dds::sub::detail::SamplesBIIteratorHolder<T, SamplesBIIterator> holder(samples);

    //    this->AnyDataReaderDelegate::read((u_dataReader)(this->userHandle), this->status_filter_, holder, -1);

    //    return holder.get_length();
}

template<typename T>
template<typename SamplesBIIterator>
uint32_t dds::sub::detail::DataReader<T>::take(
        SamplesBIIterator samples)
{
    //To implement
    //    dds::sub::detail::SamplesBIIteratorHolder<T, SamplesBIIterator> holder(samples);

    //    this->AnyDataReaderDelegate::take((u_dataReader)(this->userHandle), this->status_filter_, holder, -1);

    //    return holder.get_length();
}

template<typename T>
dds::topic::TopicInstance<T>
dds::sub::detail::DataReader<T>::key_value(
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    T key_holder;

    //    this->AnyDataReaderDelegate::get_key_value((u_dataReader)(this->userHandle), h, &key_holder);

    //    return dds::topic::TopicInstance<T>(h, key_holder);
}

template<typename T>
T &
dds::sub::detail::DataReader<T>::key_value(
        T& key,
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    this->AnyDataReaderDelegate::get_key_value((u_dataReader)(this->userHandle), h, &key);

    //    return key;
}

template<typename T>
const dds::core::InstanceHandle dds::sub::detail::DataReader<T>::lookup_instance(
        const T& key) const
{
    //To implement
    //    dds::core::InstanceHandle handle(this->AnyDataReaderDelegate::lookup_instance((u_dataReader)(this->userHandle), &key));

    //    return handle;
}

template<typename T>
const dds::sub::Subscriber&
dds::sub::detail::DataReader<T>::subscriber() const
{
    //To implement
    //    this->check();

    //    return sub_;
}

template<typename T>
void dds::sub::detail::DataReader<T>::close()
{
    //To implement
    //    this->listener(NULL, dds::core::status::StatusMask::none());
    //    this->listener_dispatcher_reset();

    //    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    //    this->sub_.delegate()->remove_datareader(*this);

    //    // Remove our dependency on the topicdescription, and drop our reference to it,
    //    // so that it can become garbage collected.
    //    // It is important that we also drop our reference to the topicdescription, since
    //    // subsequent dependencies between for example ContentFilteredTopic to Topic can
    //    // only be dropped by the destructor of the ContentFilteredTopic.
    //    this->AnyDataReaderDelegate::td_.delegate()->decrNrDependents();
    //    this->AnyDataReaderDelegate::td_ = dds::topic::TopicDescription(dds::core::null);

    //    org::opensplice::sub::AnyDataReaderDelegate::close();

    //    scopedLock.unlock();
}

template<typename T>
dds::sub::DataReaderListener<T>* dds::sub::detail::DataReader<T>::listener()
{
    //    return reinterpret_cast<dds::sub::DataReaderListener<T>*>(this->listener_get());
}

template<typename T>
void dds::sub::detail::DataReader<T>::listener(
        dds::sub::DataReaderListener<T>* l,
        const dds::core::status::StatusMask& event_mask)
{
    //To implement
    //    /* EntityDelegate takes care of thread safety. */
    //    this->listener_set((void*)l, event_mask);
    //    this->listener_enable();
}

template<typename T>
dds::sub::DataReader<T, dds::sub::detail::DataReader> dds::sub::detail::DataReader<T>::wrapper()
{
    //To implement
    //    typename DataReader::ref_type ref =
    //            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DataReader<T> >(this->get_strong_ref());
    //    dds::sub::DataReader<T, dds::sub::detail::DataReader> reader(ref);

    //    return reader;
}

template<typename T>
void dds::sub::detail::DataReader<T>::listener_notify(
        ObjectDelegate::ref_type source,
        uint32_t triggerMask,
        void* eventData,
        void* l)
{
    //To implement
    //    /* The EntityDelegate takes care of the thread safety and always
    //     * provides a listener and source. */
    //    dds::sub::DataReaderListener<T>* listener =
    //            reinterpret_cast<dds::sub::DataReaderListener<T>*>(l);
    //    assert(listener);

    //    /* Get DataWriter wrapper from given source EntityDelegate. */
    //    typename DataReader::ref_type ref =
    //            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DataReader<T> >(source);
    //    dds::sub::DataReader<T, dds::sub::detail::DataReader> reader(ref->wrapper());


    //    if (triggerMask & V_EVENT_DATA_AVAILABLE) {
    //        ref->reset_data_available();
    //        listener->on_data_available(reader);
    //    }

    //    if (triggerMask & V_EVENT_SAMPLE_REJECTED) {
    //        dds::core::status::SampleRejectedStatus status;
    //        status.delegate().v_status(v_readerStatus(eventData)->sampleRejected);
    //        listener->on_sample_rejected(reader, status);
    //    }

    //    if (triggerMask & V_EVENT_LIVELINESS_CHANGED) {
    //        dds::core::status::LivelinessChangedStatus status;
    //        status.delegate().v_status(v_readerStatus(eventData)->livelinessChanged);
    //        listener->on_liveliness_changed(reader, status);
    //    }

    //    if (triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
    //        dds::core::status::RequestedDeadlineMissedStatus status;
    //        status.delegate().v_status(v_readerStatus(eventData)->deadlineMissed);
    //        listener->on_requested_deadline_missed(reader, status);
    //    }

    //    if (triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
    //        dds::core::status::RequestedIncompatibleQosStatus status;
    //        status.delegate().v_status(v_readerStatus(eventData)->incompatibleQos);
    //        listener->on_requested_incompatible_qos(reader, status);
    //    }

    //    if (triggerMask & V_EVENT_SAMPLE_LOST) {
    //        dds::core::status::SampleLostStatus status;
    //        status.delegate().v_status(v_readerStatus(eventData)->sampleLost);
    //        listener->on_sample_lost(reader, status);
    //    }

    //    if (triggerMask & V_EVENT_SUBSCRIPTION_MATCHED) {
    //        dds::core::status::SubscriptionMatchedStatus status;
    //        status.delegate().v_status(v_readerStatus(eventData)->subscriptionMatch);
    //        listener->on_subscription_matched(reader, status);
    //    }
}

template<typename T>
dds::sub::detail::DataReader<T>::Selector::Selector(
        typename DataReader<T>::ref_type dr)
    : mode(SELECT_MODE_READ)
    , reader(dr)
    , state_filter_is_set_(false)
    , max_samples_((uint32_t)-1)
    , query_(dds::core::null)
{
}

template<typename T>
typename dds::sub::detail::DataReader<T>::Selector& dds::sub::detail::DataReader<T>::Selector::instance(
    const dds::core::InstanceHandle& h)
{
    //To implement
    //    this->handle = h;
    //    switch (this->mode) {
    //    case SELECT_MODE_READ:
    //    case SELECT_MODE_READ_INSTANCE:
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        this->mode = SELECT_MODE_READ_INSTANCE;
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        this->mode = SELECT_MODE_READ_INSTANCE_WITH_CONDITION;
    //        break;
    //    }

    //    return *this;
}

template<typename T>
typename dds::sub::detail::DataReader<T>::Selector& dds::sub::detail::DataReader<T>::Selector::next_instance(
    const dds::core::InstanceHandle& h)
{
    //To implement
    //    this->handle = h;
    //    switch (this->mode) {
    //    case SELECT_MODE_READ:
    //    case SELECT_MODE_READ_INSTANCE:
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        this->mode = SELECT_MODE_READ_NEXT_INSTANCE;
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        this->mode = SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION;
    //        break;
    //    }

    //    return *this;
}

template<typename T>
typename dds::sub::detail::DataReader<T>::Selector& dds::sub::detail::DataReader<T>::Selector::filter_state(
    const dds::sub::status::DataState& s)
{
    //To implement
    //    this->state_filter_ = s;
    //    this->state_filter_is_set_ = true;
    //    if ((this->mode == SELECT_MODE_READ_WITH_CONDITION) ||
    //        (this->mode == SELECT_MODE_READ_INSTANCE_WITH_CONDITION) ||
    //        (this->mode == SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION)) {
    //        if (!this->query_.delegate()->modify_state_filter(this->state_filter_)) {
    //            dds::sub::Query q(this->query_.data_reader(), this->query_.expression(), this->query_.delegate()->parameters());
    //            q.delegate()->state_filter(this->state_filter_);
    //            this->query_ = q;
    //        }
    //    }
    //    return *this;
}

template<typename T>
typename dds::sub::detail::DataReader<T>::Selector& dds::sub::detail::DataReader<T>::Selector::max_samples(
    uint32_t n)
{
    //To implement
    //    this->max_samples_ = n;
    //    return *this;
}

template<typename T>
typename dds::sub::detail::DataReader<T>::Selector& dds::sub::detail::DataReader<T>::Selector::filter_content(
    const dds::sub::Query& query)
{
    //To implement
    //    if (this->state_filter_is_set_) {
    //        if (!query.delegate()->modify_state_filter(this->state_filter_)) {
    //            dds::sub::Query q(query.data_reader(), query.expression(), query.delegate()->parameters());
    //            q.delegate()->state_filter(this->state_filter_);
    //            this->query_ = q;
    //        } else {
    //            this->query_ = query;
    //        }
    //    } else {
    //        this->query_ = query;
    //    }

    //    switch (mode) {
    //    case SELECT_MODE_READ:
    //        mode = SELECT_MODE_READ_WITH_CONDITION;
    //        break;
    //    case SELECT_MODE_READ_INSTANCE:
    //        mode = SELECT_MODE_READ_INSTANCE_WITH_CONDITION;
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        mode = SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION;
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        break;
    //    default:
    //        break;
    //    }

    //    return *this;
}

template<typename T>
dds::sub::LoanedSamples<T> dds::sub::detail::DataReader<T>::Selector::read()
{
    //To implement
    //    return this->reader->read(*this);
}

template<typename T>
dds::sub::LoanedSamples<T> dds::sub::detail::DataReader<T>::Selector::take()
{
    //To implement
    //    return this->reader->take(*this);
}

// --- Forward Iterators: --- //

template<typename T>
template<typename SamplesFWIterator>
uint32_t dds::sub::detail::DataReader<T>::Selector::read(
    SamplesFWIterator sfit,
    uint32_t max_samples)
{
    //To implement
    //    return this->reader->read(sfit, max_samples, *this);
}

template<typename T>
template<typename SamplesFWIterator>
uint32_t dds::sub::detail::DataReader<T>::Selector::take(
    SamplesFWIterator sfit,
    uint32_t max_samples)
{
    //To implement
    //    return this->reader->take(sfit, max_samples, *this);
}

// --- Back-Inserting Iterators: --- //

template<typename T>
template<typename SamplesBIIterator>
uint32_t dds::sub::detail::DataReader<T>::Selector::read(
    SamplesBIIterator sbit)
{
    //To implement
    //    return this->reader->read(sbit, *this);
}

template<typename T>
template<typename SamplesBIIterator>
uint32_t dds::sub::detail::DataReader<T>::Selector::take(
    SamplesBIIterator sbit)
{
    //To implement
    //    return this->reader->take(sbit, *this);
}

template<typename T>
typename dds::sub::detail::DataReader<T>::SelectMode dds::sub::detail::DataReader<T>::Selector::get_mode() const
{
    //To implement
    //    return this->mode;
}

template<typename T>
dds::sub::detail::DataReader<T>::ManipulatorSelector::ManipulatorSelector(
        typename DataReader<T>::ref_type dr)
    : Selector(dr)
    , read_mode_(true)
{
}

template<typename T>
bool dds::sub::detail::DataReader<T>::ManipulatorSelector::read_mode()
{
    //To implement
    //    return read_mode_;
}

template<typename T>
void dds::sub::detail::DataReader<T>::ManipulatorSelector::read_mode(
    bool b)
{
    //To implement
    //    read_mode_ = b;
}

template<typename T>
typename dds::sub::detail::DataReader<T>::ManipulatorSelector& dds::sub::detail::DataReader<T>::ManipulatorSelector::
operator >>(
        dds::sub::LoanedSamples<T>& samples)
{
    //To implement
    //    if(read_mode_)
    //    {
    //        samples = this->Selector::read();
    //    }
    //    else
    //    {
    //        samples = this->Selector::take();
    //    }
    //    return *this;
}

template<typename T>
dds::sub::LoanedSamples<T> dds::sub::detail::DataReader<T>::read(
        const Selector& selector)
{
    //To implement
    //    u_query uQuery;

    //    dds::sub::LoanedSamples<T> samples;
    //    dds::sub::detail::LoanedSamplesHolder<T> holder(samples);

    //    switch(selector.mode) {
    //    case SELECT_MODE_READ:
    //        this->AnyDataReaderDelegate::read((u_dataReader)(userHandle), selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE:
    //        this->AnyDataReaderDelegate::read_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        this->AnyDataReaderDelegate::read_next_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_w_condition(uQuery, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_instance_w_condition(uQuery, selector.handle, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_next_instance_w_condition(uQuery, selector.handle, holder, selector.max_samples_);
    //        break;
    //    }

    //    return samples;
}

template<typename T>
dds::sub::LoanedSamples<T> dds::sub::detail::DataReader<T>::take(
        const Selector& selector)
{
    //To implement
    //    u_query uQuery;

    //    dds::sub::LoanedSamples<T> samples;
    //    dds::sub::detail::LoanedSamplesHolder<T> holder(samples);

    //    switch(selector.mode) {
    //    case SELECT_MODE_READ:
    //        this->AnyDataReaderDelegate::take((u_dataReader)(userHandle), selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE:
    //        this->AnyDataReaderDelegate::take_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        this->AnyDataReaderDelegate::take_next_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_w_condition(uQuery, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_instance_w_condition(uQuery, selector.handle, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_next_instance_w_condition(uQuery, selector.handle, holder, selector.max_samples_);
    //        break;
    //    }

    //    return samples;
}

// --- Forward Iterators: --- //

template<typename T>
template<typename SamplesFWIterator>
uint32_t dds::sub::detail::DataReader<T>::read(
        SamplesFWIterator samples,
        uint32_t max_samples,
        const Selector& selector)
{
    //To implement
    //    u_query uQuery;

    //    dds::sub::detail::SamplesFWInteratorHolder<T, SamplesFWIterator> holder(samples);

    //    switch(selector.mode) {
    //    case SELECT_MODE_READ:
    //        this->AnyDataReaderDelegate::read((u_dataReader)(userHandle), selector.state_filter_, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE:
    //        this->AnyDataReaderDelegate::read_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        this->AnyDataReaderDelegate::read_next_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_w_condition(uQuery, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_instance_w_condition(uQuery, selector.handle, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_next_instance_w_condition(uQuery, selector.handle, holder, max_samples);
    //        break;
    //    }

    //    return holder.get_length();
}

template<typename T>
template<typename SamplesFWIterator>
uint32_t dds::sub::detail::DataReader<T>::take(
        SamplesFWIterator samples,
        uint32_t max_samples,
        const Selector& selector)
{
    //To implement
    //    u_query uQuery;

    //    dds::sub::detail::SamplesFWInteratorHolder<T, SamplesFWIterator> holder(samples);

    //    switch(selector.mode) {
    //    case SELECT_MODE_READ:
    //        this->AnyDataReaderDelegate::take((u_dataReader)(userHandle), selector.state_filter_, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE:
    //        this->AnyDataReaderDelegate::take_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        this->AnyDataReaderDelegate::take_next_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_w_condition(uQuery, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_instance_w_condition(uQuery, selector.handle, holder, max_samples);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_next_instance_w_condition(uQuery, selector.handle, holder, max_samples);
    //        break;
    //    }

    //    return holder.get_length();
}

// --- Back-Inserting Iterators: --- //

template<typename T>
template<typename SamplesBIIterator>
uint32_t dds::sub::detail::DataReader<T>::read(
        SamplesBIIterator samples,
        const Selector& selector)
{
    //To implement
    //    u_query uQuery;

    //    dds::sub::detail::SamplesBIIteratorHolder<T, SamplesBIIterator> holder(samples);

    //    switch(selector.mode) {
    //    case SELECT_MODE_READ:
    //        this->AnyDataReaderDelegate::read((u_dataReader)(userHandle), selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE:
    //        this->AnyDataReaderDelegate::read_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        this->AnyDataReaderDelegate::read_next_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_w_condition(uQuery, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_instance_w_condition(uQuery, selector.handle, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::read_next_instance_w_condition(uQuery, selector.handle, holder, selector.max_samples_);
    //        break;
    //    }

    //    return holder.get_length();
}

template<typename T>
template<typename SamplesBIIterator>
uint32_t dds::sub::detail::DataReader<T>::take(
        SamplesBIIterator samples,
        const Selector& selector)
{
    //To implement
    //    u_query uQuery;

    //    dds::sub::detail::SamplesBIIteratorHolder<T, SamplesBIIterator> holder(samples);

    //    switch(selector.mode) {
    //    case SELECT_MODE_READ:
    //        this->AnyDataReaderDelegate::take((u_dataReader)(userHandle), selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE:
    //        this->AnyDataReaderDelegate::take_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE:
    //        this->AnyDataReaderDelegate::take_next_instance((u_dataReader)(userHandle), selector.handle, selector.state_filter_, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_w_condition(uQuery, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_instance_w_condition(uQuery, selector.handle, holder, selector.max_samples_);
    //        break;
    //    case SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION:
    //        uQuery = selector.query_.delegate()->get_user_query();
    //        this->AnyDataReaderDelegate::take_next_instance_w_condition(uQuery, selector.handle, holder, selector.max_samples_);
    //        break;
    //    }

    //    return holder.get_length();
}

namespace dds {
namespace sub {

/**
 * Indicate to do a read when using the DataReader stream operator>>.
 *
 * Reading data can be done by the DataReader stream operator>>. The default of that
 * operator is to read samples (not to take them). However, it is possible
 * to manipulate if the samples are read or taken.
 *
 * By adding this manipulator in the stream operator>>, it is explicitly indicated that
 * the samples will be read and not taken.
 * @code{.cpp}
 * // Read all samples implicitly
 * reader >> samples;
 * // Read all samples explicitly
 * reader >> dds::sub::read >> samples;
 * @endcode
 *
 * Adding a manipulater operation in the stream operator>>, will create and use a
 * dds::sub::DataReader::ManipulatorSelector implicitly.
 *
 * @see @link dds::sub::DataReader::operator>>(dds::sub::LoanedSamples<T>& ls) DataReader stream operator>> @endlink
 */
template<typename SELECTOR>
SELECTOR& read(
        SELECTOR& selector)
{
    //To implement
    //    selector.read_mode(true);
    //    return selector;
}

/**
 * Indicate to do a take when using the DataReader stream operator>>.
 *
 * Reading data can be done by the DataReader stream operator>>. The default of that
 * operator is to read samples (not to take them). However, it is possible
 * to manipulate if the samples are read or taken.
 *
 * By adding this manipulator in the stream operator>>, it is explicitly indicated that
 * the samples will be taken and not read.
 * @code{.cpp}
 * // Read all samples implicitly
 * reader >> samples;
 * // Take all samples explicitly
 * reader >> dds::sub::take >> samples;
 * @endcode
 *
 * Adding a manipulater operation in the stream operator>>, will create and use a
 * dds::sub::DataReader::ManipulatorSelector implicitly.
 *
 * @see @link dds::sub::DataReader::operator>>(dds::sub::LoanedSamples<T>& ls) DataReader stream operator>> @endlink
 */
template<typename SELECTOR>
SELECTOR& take(
        SELECTOR& selector)
{
    //To implement
    //    selector.read_mode(false);
    //    return selector;
}

/**
 * Limit the number of samples, read by the DataReader stream operator>>.
 *
 * Reading data can be done by the DataReader stream operator>>. Normally, that would
 * read all data samples that is available within the reader. However, it is possible
 * to manipulate what samples are read.
 *
 * By adding this operation as a selection in the read or as a manipulator in the read
 * streaming operator, it is explicitly indicated that a maximum of n samples will be read.
 * @code{.cpp}
 * // Read a maximum of three samples
 * reader >> dds::sub::max_samples(3) >> samples;
 * @endcode
 *
 * Adding a manipulater operation in the stream operator>>, will create and use a
 * dds::sub::DataReader::ManipulatorSelector implicitly.
 *
 * @see @link dds::sub::DataReader::operator>>(dds::sub::LoanedSamples<T>& ls) DataReader stream operator>> @endlink
 */
inline dds::sub::functors::MaxSamplesManipulatorFunctor max_samples(
        uint32_t n)
{
    //To implement
    //    return dds::sub::functors::MaxSamplesManipulatorFunctor(n);
}

/**
 * Filter the samples by the query, read by the DataReader stream operator>>.
 *
 * Reading data can be done by the DataReader stream operator>>. Normally, that would
 * read all data samples that is available within the reader. However, it is possible
 * to manipulate what samples are read.
 *
 * By adding this operation as a selection in the read or as a manipulator in the read
 * streaming operator, it is explicitly indicated that samples will be filtered according
 * to the given dds::sub::Query.
 * @code{.cpp}
 * // Assume data type has an element called long_1
 * dds::sub::Query query(reader, "long_1 > 1 and long_1 < 7");
 *
 * // Read samples, filtered by content
 * reader >> dds::sub::dds::sub::content(query) >> samples;
 * @endcode
 *
 * Adding a manipulater operation in the stream operator>>, will create and use a
 * dds::sub::DataReader::ManipulatorSelector implicitly.
 *
 * @see @link dds::sub::DataReader::operator>>(dds::sub::LoanedSamples<T>& ls) DataReader stream operator>> @endlink
 * @see @ref dds::sub::Query "Query"
 */
inline dds::sub::functors::ContentFilterManipulatorFunctor content(
        const dds::sub::Query& query)
{
    //To implement
    //    return dds::sub::functors::ContentFilterManipulatorFunctor(query);
}

/**
 * Filter the samples based on their state, read by the DataReader stream operator>>.
 *
 * Reading data can be done by the DataReader stream operator>>. Normally, that would
 * read all data samples that is available within the reader. However, it is possible
 * to manipulate what samples are read.
 *
 * By adding this operation as a selection in the read or as a manipulator in the read
 * streaming operator, it is explicitly indicated that samples will be filtered according
 * to the given dds::sub::status::DataState.
 * @code{.cpp}
 * // DataState to filter only new data
 * dds::sub::status::DataState newData = dds::sub::status::DataState::new_data();
 *
 * // Read samples, filtered by state
 * reader >> dds::sub::state(newData) >> samples;
 * @endcode
 *
 * Adding a manipulater operation in the stream operator>>, will create and use a
 * dds::sub::DataReader::ManipulatorSelector implicitly.
 *
 * @see @link dds::sub::DataReader::select() DataReader select() @endlink
 * @see @link dds::sub::DataReader::operator>>(dds::sub::LoanedSamples<T>& ls) DataReader stream operator>> @endlink
 * @see @ref dds::sub::Query "Query"
 */
inline dds::sub::functors::StateFilterManipulatorFunctor state(
        const dds::sub::status::DataState& s)
{
    //To implement
    //    return dds::sub::functors::StateFilterManipulatorFunctor(s);
}

/**
 * Filter the samples of an instance, read by the DataReader stream operator>>.
 *
 * Reading data can be done by the DataReader stream operator>>. Normally, that would
 * read all data samples that is available within the reader. However, it is possible
 * to manipulate what samples are read.
 *
 * By adding this operation as a selection in the read or as a manipulator in the read
 * streaming operator, it is explicitly indicated that only samples of the given instance
 * are read.
 * @code{.cpp}
 * dds::core::InstanceHandle hdl = someValidInstanceHandle;
 *
 * // Read samples, filtered by instance
 * reader >> dds::sub::instance(hdl) >> samples;
 * @endcode
 *
 * Adding a manipulater operation in the stream operator>>, will create and use a
 * dds::sub::DataReader::ManipulatorSelector implicitly.
 *
 * @see @link dds::sub::DataReader::operator>>(dds::sub::LoanedSamples<T>& ls) DataReader stream operator>> @endlink
 * @see @ref dds::core::InstanceHandle "Instance Handle"
 */
inline dds::sub::functors::InstanceManipulatorFunctor instance(
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    return dds::sub::functors::InstanceManipulatorFunctor(h);
}

/**
 * Filter the samples of the next instance, read by the DataReader stream operator>>.
 *
 * Reading data can be done by the DataReader stream operator>>. Normally, that would
 * read all data samples that is available within the reader. However, it is possible
 * to manipulate what samples are read.
 *
 * By adding this operation as a selection in the read or as a manipulator in the read
 * streaming operator, it is explicitly indicated that only samples of the next instance
 * of the given instance are read. When the given instance is a nil handle, then the
 * first instance will be read.
 * @code{.cpp}
 * // Read all samples, instance by instance
 * {
 *     dds::sub::LoanedSamples<Foo::Bar> samples;
 *     // Get sample(s) of first instance
 *     dds::core::InstanceHandle hdl; //nil
 *     reader >> dds::sub::next_instance(hdl) >> samples;
 *     while (samples.length() > 0) {
 *         // Handle the sample(s) of this instance (just the first one in this case)
 *         const dds::sub::Sample<Foo::Bar>& sample = *(samples.begin());
 *         // Get sample(s) of the next instance
 *         hdl = sample.info().instance_handle();
 *         reader >> dds::sub::next_instance(hdl) >> samples;
 *     }
 * }
 * @endcode
 *
 * Adding a manipulater operation in the stream operator>>, will create and use a
 * dds::sub::DataReader::ManipulatorSelector implicitly.
 *
 * @see @link dds::sub::DataReader::operator>>(dds::sub::LoanedSamples<T>& ls) DataReader stream operator>> @endlink
 * @see @ref dds::core::InstanceHandle "Instance Handle"
 */
inline dds::sub::functors::NextInstanceManipulatorFunctor next_instance(
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    return dds::sub::functors::NextInstanceManipulatorFunctor(h);
}

} //namespace sub
} //namespace dds

#endif //EPROSIMA_DDS_SUB_TDATAREADER_IMPL_HPP_

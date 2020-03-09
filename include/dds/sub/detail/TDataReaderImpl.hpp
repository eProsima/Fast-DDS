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
/*
template<typename T>
DataReader<T>::Selector::Selector(
        DataReader& dr)
    : impl_(dr.delegate())
{
}

template<typename T>
typename DataReader<T>::Selector&
DataReader<T>::Selector::instance(
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.instance(h);
    //    return *this;
}


template<typename T>
typename DataReader<T>::Selector& DataReader<T>::Selector::next_instance(
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.next_instance(h);
    //    return *this;
}


template<typename T>
typename DataReader<T>::Selector& DataReader<T>::Selector::state(
        const dds::sub::status::DataState& s)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.filter_state(s);
    //    return *this;
}

template<typename T>
typename DataReader<T>::Selector& DataReader<T>::Selector::content(
        const dds::sub::Query& query)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.filter_content(query);
    //    return *this;
}

template<typename T>
typename DataReader<T>::Selector& DataReader<T>::Selector::max_samples(
        uint32_t n)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.max_samples(n);
    //    return *this;
}

template<typename T>
dds::sub::LoanedSamples<T> DataReader<T>::Selector::read()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    return impl_.read();
}

template<typename T>
dds::sub::LoanedSamples<T> DataReader<T>::Selector::take()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    return impl_.take();
}

template<typename T>
template<typename SamplesFWIterator>
uint32_t DataReader<T>::Selector::read(
        SamplesFWIterator sfit,
        uint32_t max_samples)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    return impl_.read(sfit, max_samples);
}

template<typename T>
template<typename SamplesFWIterator>
uint32_t DataReader<T>::Selector::take(
        SamplesFWIterator sfit,
        uint32_t max_samples)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    return impl_.take(sfit, max_samples);
}

template<typename T>
template<typename SamplesBIIterator>
uint32_t DataReader<T>::Selector::read(
        SamplesBIIterator sbit)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    return impl_.read(sbit);
}

template<typename T>
template<typename SamplesBIIterator>
uint32_t DataReader<T>::Selector::take(
        SamplesBIIterator sbit)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    return impl_.take(sbit);
}

//--------------------------------------------------------------------------------
//  DATAREADER::MANIPULATORSELECTOR
//--------------------------------------------------------------------------------
template<typename T>
DataReader<T>::ManipulatorSelector::ManipulatorSelector(
        DataReader& dr)
    : impl_(dr.delegate())
{
}

template<typename T>
bool DataReader<T>::ManipulatorSelector::read_mode()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    return impl_.read_mode();
}

template<typename T>
void DataReader<T>::ManipulatorSelector::read_mode(
        bool b)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());
    //    impl_.read_mode(b);
}

template<typename T>
typename DataReader<T>::ManipulatorSelector& DataReader<T>::ManipulatorSelector::instance(
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.instance(h);
    //    return *this;
}

template<typename T>
typename DataReader<T>::ManipulatorSelector& DataReader<T>::ManipulatorSelector::next_instance(
        const dds::core::InstanceHandle& h)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.next_instance(h);
    //    return *this;
}

template<typename T>
typename DataReader<T>::ManipulatorSelector& DataReader<T>::ManipulatorSelector::operator >>(
        dds::sub::LoanedSamples<T>& samples)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_ >> samples;
    //    return *this;
}

template<typename T>
typename DataReader<T>::ManipulatorSelector& DataReader<T>::ManipulatorSelector::operator >>(
        ManipulatorSelector & (manipulator)(ManipulatorSelector&))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    manipulator(*this);
    //    return *this;
}

// @cond
template<typename T>
template<typename Functor>
typename DataReader<T>::ManipulatorSelector DataReader<T>::ManipulatorSelector::operator >>(
        Functor f)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    f(*this);
    //    return *this;
}
// @endcond

template<typename T>
typename DataReader<T>::ManipulatorSelector& DataReader<T>::ManipulatorSelector::state(
        const dds::sub::status::DataState& s)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.filter_state(s);
    //    return *this;
}

template<typename T>
typename DataReader<T>::ManipulatorSelector& DataReader<T>::ManipulatorSelector::content(
        const dds::sub::Query& query)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.filter_content(query);
    //    return *this;
}

template<typename T>
typename DataReader<T>::ManipulatorSelector& DataReader<T>::ManipulatorSelector::max_samples(
        uint32_t n)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this->impl_.get_reader());

    //    impl_.max_samples(n);
    //    return *this;
}
*/
template<typename T>
DataReader<T>::DataReader(
    const dds::sub::Subscriber& sub,
    const dds::topic::Topic<T>& topic):
        ::dds::core::Reference< detail::DataReader >(
            new detail::DataReader(
                sub.delegate().get(),
                topic,
                sub.is_nil() ? dds::sub::qos::DataReaderQos() : sub.default_datareader_qos()))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);
    //    this->delegate()->init(this->impl_);
}

template<typename T>
DataReader<T>::DataReader(
    const dds::sub::Subscriber& sub,
    const ::dds::topic::Topic<T>& topic,
    const dds::sub::qos::DataReaderQos& qos,
    dds::sub::DataReaderListener<T>* listener,
    const dds::core::status::StatusMask& mask) :
        ::dds::core::Reference< detail::DataReader >(
            new detail::DataReader(
                sub.delegate().get(),
                *topic.delegate().get(),
                qos,
                listener,
                mask))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);
    //    this->delegate()->init(this->impl_);
}

#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT
template<typename T>
DataReader<T>::DataReader(
    const dds::sub::Subscriber& sub,
    const dds::topic::ContentFilteredTopic<T>& topic) :
        ::dds::core::Reference< detail::DataReader >(new detail::DataReader(sub, topic, sub.default_datareader_qos()))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);
    //    this->delegate()->init(this->impl_);
}

template<typename T>
DataReader<T>::DataReader(
    const dds::sub::Subscriber& sub,
    const ::dds::topic::ContentFilteredTopic<T>& topic,
    const dds::sub::qos::DataReaderQos& qos,
    dds::sub::DataReaderListener<T>* listener,
    const dds::core::status::StatusMask& mask) :
        ::dds::core::Reference< detail::DataReader >(new detail::DataReader(sub, topic, qos, listener, mask))
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);
    //    this->delegate()->init(this->impl_);
}

#endif //OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT
template<typename T>
DataReader<T>::DataReader(
    const dds::sub::Subscriber& sub,
    const dds::topic::MultiTopic<T>& topic) :
        ::dds::core::Reference< detail::DataReader >(new detail::DataReader(sub, topic))
{
    //To implement
    //    this->delegate()->init(this->impl_);
}

template<typename T>
DataReader<T>::DataReader(
    const dds::sub::Subscriber& sub,
    const ::dds::topic::MultiTopic<T>& topic,
    const dds::sub::qos::DataReaderQos& qos,
    dds::sub::DataReaderListener<T>* listener,
    const dds::core::status::StatusMask& mask) :
    ::dds::core::Reference< detail::DataReader >(new detail::DataReader(sub, topic, qos, listener, mask))
{
    //To implement
    //    this->delegate()->init(this->impl_);
}

#endif //OMG_DDS_MULTI_TOPIC_SUPPORT

template<typename T>
DataReader<T>::~DataReader()
{
}

template<typename T>
dds::sub::status::DataState DataReader<T>::default_filter_state()
{
    //To implement
//    return this->delegate()->default_filter_state();
    *this = dds::core::null;
    return *this;
}

template<typename T>
DataReader<T>& DataReader<T>::default_filter_state(
        const dds::sub::status::DataState& /*status*/)
{
    //To implement
//    this->delegate()->default_filter_state(status);
    return *this;
}

template<typename T>
DataReader<T>& DataReader<T>::operator >>(
        dds::sub::LoanedSamples<T>& ls)
{
    ls = this->read();
    return *this;
}

template<typename T>
typename DataReader<T>::ManipulatorSelector DataReader<T>::operator >>(
        ManipulatorSelector& (manipulator)(ManipulatorSelector&))
{
    ManipulatorSelector selector(*this);
    manipulator(selector);
    return selector;
}

/** @cond */
template<typename T>
template<typename Functor>
typename DataReader<T>::ManipulatorSelector DataReader<T>::operator >>(
        Functor f)
{
    ManipulatorSelector selector(*this);
    f(selector);
    return selector;
}

/** @endcond */

template<typename T>
LoanedSamples<T> DataReader<T>::read()
{
    //To implement
//    return this->delegate()->read();
    *this = dds::core::null;
    return *this;
}

template<typename T>
LoanedSamples<T> DataReader<T>::take()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->take();
    LoanedSamples<T> result;
    T data;
    SampleInfo info;

    //if (!!reader->take_next_sample(&hello_, info_))
    if (!!this->delegate()->take_next_sample(&data, info))
    {
        Sample<T> sample;
        sample.data(data);
        sample.info(info);
        result.delegate()->container().push_back(sample);
    }

    return result;
}


template<typename T>
template<typename SamplesFWIterator>
uint32_t DataReader<T>::read(
        SamplesFWIterator /*sfit*/,
        uint32_t /*max_samples*/)
{
    //To implement
//    return this->delegate()->read(sfit, max_samples);
    throw "Not implemented";
    return 0;
}

template<typename T>
template<typename SamplesFWIterator>
uint32_t DataReader<T>::take(
        SamplesFWIterator /*sfit*/,
        uint32_t /*max_samples*/)
{
    //To implement
//    return this->delegate()->take(sfit, max_samples);
    throw "Not implemented";
    return 0;
}

template<typename T>
template<typename SamplesBIIterator>
uint32_t DataReader<T>::read(
        SamplesBIIterator /*sbit*/)
{
    //To implement
//    return this->delegate()->read(sbit);
    throw "Not implemented";
    return 0;
}

template<typename T>
template<typename SamplesBIIterator>
uint32_t DataReader<T>::take(
        SamplesBIIterator /*sbit*/)
{
    //To implement
//    return this->delegate()->take(sbit);
    throw "Not implemented";
    return 0;
}

template<typename T>
typename DataReader<T>::Selector DataReader<T>::select()
{
    //To implement
    Selector selector(*this);
    return selector;
}

template<typename T>
dds::topic::TopicInstance<T> DataReader<T>::key_value(
        const dds::core::InstanceHandle& /*h*/)
{
    //To implement
//    return this->delegate()->key_value(h);
}

template<typename T>
T& DataReader<T>::key_value(
        T& /*sample*/,
        const dds::core::InstanceHandle& /*h*/)
{
    //To implement
//    return this->delegate()->key_value(sample, h);
}

template<typename T>
const dds::core::InstanceHandle DataReader<T>::lookup_instance(
        const T& /*key*/) const
{
    //To implement
//    return this->delegate()->lookup_instance(key);
    *this = dds::core::null;
    return *this;
}

template<typename T>
void DataReader<T>::listener(
    Listener* listener,
    const dds::core::status::StatusMask& /*event_mask*/)
{
    delegate()->set_listener(listener/*, event_mask*/);
}

template<typename T>
typename DataReader<T>::Listener*
DataReader<T>::listener() const
{
    return delegate()->get_listener();
}

} //namespace sub
} //namespace dds

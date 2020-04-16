/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <dds/sub/DataReader.hpp>
#include <dds/sub/DataReaderListener.hpp>
#include <dds/topic/Topic.hpp>

namespace dds {
namespace sub {

DataReader::DataReader(
        const Subscriber& sub,
        const dds::topic::Topic& topic)
    : ::dds::core::Reference<detail::DataReader>(
        new detail::DataReader(
            sub.delegate().get(),
            topic.delegate().get(),
            sub.default_datareader_qos(),
            nullptr,
            dds::core::status::StatusMask::all()))
    , subscriber_(nullptr)
{
}

DataReader::DataReader(
        const Subscriber& sub,
        const dds::topic::Topic& topic,
        const qos::DataReaderQos& qos,
        DataReaderListener* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference<detail::DataReader>(
        new detail::DataReader(
            sub.delegate().get(), topic.delegate().get(), qos, listener, mask))
    , subscriber_(nullptr)
{
}

DataReader::~DataReader()
{
}

//const qos::DataReaderQos& DataReader::qos() const
//{
//    return delegate()->get_qos();
//}

//void DataReader::qos(
//        const qos::DataReaderQos& pqos)
//{
//    delegate()->set_qos(pqos);
//}

//DataReader& DataReader::operator <<(
//        const qos::DataReaderQos& qos)
//{
//    this->qos(qos);
//    return *this;
//}

//DataReader& DataReader::operator >>(
//        qos::DataReaderQos& qos)
//{
//    qos = this->qos();
//    return *this;
//}

//void DataReader::listener(
//        Listener* plistener,
//        const dds::core::status::StatusMask& /*event_mask*/)
//{
//    delegate()->set_listener(plistener /*, event_mask*/);
//}

//typename DataReader::Listener* DataReader::listener() const
//{
//    return dynamic_cast<Listener*>(delegate()->get_listener());
//}

//const dds::sub::Subscriber& DataReader::subscriber() const
//{
//    eprosima::fastdds::dds::Subscriber s = delegate()->get_subscriber();
//    std::shared_ptr<eprosima::fastdds::dds::Subscriber> ptr(&s);
//    subscriber_->delegate().swap(ptr);

//    return *subscriber;
//}

} //namespace sub
} //namespace dds


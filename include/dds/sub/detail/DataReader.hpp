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

#ifndef EPROSIMA_DDS_SUB_DETAIL_DATA_READER_HPP_
#define EPROSIMA_DDS_SUB_DETAIL_DATA_READER_HPP_

#include <dds/topic/Topic.hpp>
#include <dds/topic/TopicInstance.hpp>

#include <dds/core/status/Status.hpp>
//TODO: Fix when DataStateImpl is implemented
//#include <dds/sub/status/detail/DataStateImpl.hpp>
#include <dds/sub/detail/Manipulators.hpp>
#include <dds/sub/LoanedSamples.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/Query.hpp>

//#include <org/opensplice/core/EntityDelegate.hpp>
//#include <org/opensplice/sub/AnyDataReaderDelegate.hpp>

//#include <org/opensplice/core/ScopedLock.hpp>
//#include <org/opensplice/ForwardDeclarations.hpp>


/**
 * @cond
 * Ignore this file in the API
 */

/***************************************************************************
*
* dds/sub/detail/DataReader<> DELEGATE declaration.
* Implementation can be found in dds/sub/detail/TDataReaderImpl.hpp
*
***************************************************************************/

namespace dds {
namespace sub {
namespace detail {

template<typename T>
//class dds::sub::detail::DataReader : public ::org::opensplice::sub::AnyDataReaderDelegate
//{
//public:

//    typedef typename ::dds::core::smart_ptr_traits< DataReader<T> >::ref_type ref_type;
//    typedef typename ::dds::core::smart_ptr_traits< DataReader<T> >::weak_ref_type weak_ref_type;

//    DataReader(const dds::sub::Subscriber& sub,
//               const dds::topic::Topic<T>& topic,
//               const dds::sub::qos::DataReaderQos& qos,
//               dds::sub::DataReaderListener<T>* listener = NULL,
//               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::none());

//    DataReader(const dds::sub::Subscriber& sub,
//               const dds::topic::ContentFilteredTopic<T, dds::topic::detail::ContentFilteredTopic>& topic,
//               const dds::sub::qos::DataReaderQos& qos,
//               dds::sub::DataReaderListener<T>* listener = NULL,
//               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::none());

//    void common_constructor(dds::sub::DataReaderListener<T>* listener,
//                            const dds::core::status::StatusMask& mask);

//    virtual ~DataReader();

//    void init(ObjectDelegate::weak_ref_type weak_ref);

//    dds::sub::status::DataState default_filter_state();
//    void default_filter_state(const dds::sub::status::DataState& state);

//    dds::sub::LoanedSamples<T> read();
//    dds::sub::LoanedSamples<T> take();

//    template<typename SamplesFWIterator>
//    uint32_t read(SamplesFWIterator samples, uint32_t max_samples);
//    template<typename SamplesFWIterator>
//    uint32_t take(SamplesFWIterator samples, uint32_t max_samples);

//    template<typename SamplesBIIterator>
//    uint32_t read(SamplesBIIterator samples);
//    template<typename SamplesBIIterator>
//    uint32_t take(SamplesBIIterator samples);

//    dds::topic::TopicInstance<T> key_value(const dds::core::InstanceHandle& h);
//    T& key_value(T& key, const dds::core::InstanceHandle& h);

//    const dds::core::InstanceHandle lookup_instance(const T& key) const;

//    virtual const dds::sub::Subscriber& subscriber() const;

//    void close();

//    dds::sub::DataReaderListener<T>* listener();
//    void listener(dds::sub::DataReaderListener<T>* l,
//                  const dds::core::status::StatusMask& event_mask);

//    dds::sub::DataReader<T, dds::sub::detail::DataReader> wrapper();

//    virtual void listener_notify(ObjectDelegate::ref_type source,
//                                 uint32_t       triggerMask,
//                                 void           *eventData,
//                                 void           *listener);

//private:
//    dds::sub::Subscriber sub_;
//    dds::sub::status::DataState status_filter_;


////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//private:
//    enum SelectMode {
//        SELECT_MODE_READ,
//        SELECT_MODE_READ_INSTANCE,
//        SELECT_MODE_READ_NEXT_INSTANCE,
//        SELECT_MODE_READ_WITH_CONDITION,
//        SELECT_MODE_READ_INSTANCE_WITH_CONDITION,
//        SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION
//    };


//public:

//    class Selector
//    {
//    public:
//        Selector(typename DataReader<T>::ref_type dr);

//        Selector& instance(const dds::core::InstanceHandle& h);
//        Selector& next_instance(const dds::core::InstanceHandle& h);
//        Selector& filter_state(const dds::sub::status::DataState& s);
//        Selector& max_samples(uint32_t n);
//        Selector& filter_content(const dds::sub::Query& query);

//        dds::sub::LoanedSamples<T> read();
//        dds::sub::LoanedSamples<T> take();

//        // --- Forward Iterators: --- //
//        template<typename SamplesFWIterator>
//        uint32_t read(SamplesFWIterator sfit, uint32_t max_samples);
//        template<typename SamplesFWIterator>
//        uint32_t take(SamplesFWIterator sfit, uint32_t max_samples);

//        // --- Back-Inserting Iterators: --- //
//        template<typename SamplesBIIterator>
//        uint32_t read(SamplesBIIterator sbit);
//        template<typename SamplesBIIterator>
//        uint32_t take(SamplesBIIterator sbit);

//        SelectMode get_mode() const;

//        DataReader<T> * get_reader() const {
//            return this->reader.get();
//        }

//    private:
//        friend class DataReader;
//        SelectMode mode;
//        typename DataReader<T>::ref_type reader;
//        dds::sub::status::DataState state_filter_;
//        bool state_filter_is_set_;
//        dds::core::InstanceHandle handle;
//        uint32_t max_samples_;
//        dds::sub::Query query_;
//    };


//    class ManipulatorSelector: public Selector
//    {
//    public:
//        //ManipulatorSelector(DataReader<T>* dr);
//        ManipulatorSelector(typename DataReader<T>::ref_type dr);

//        bool read_mode();
//        void read_mode(bool b);

//        ManipulatorSelector&
//        operator >>(dds::sub::LoanedSamples<T>& samples);

//    private:
//        bool read_mode_;
//    };


//private:
//    // ==============================================================
//    // == Selector Read/Take API

//    dds::sub::LoanedSamples<T> read(const Selector& selector);

//    dds::sub::LoanedSamples<T> take(const Selector& selector);

//    // --- Forward Iterators: --- //
//    template<typename SamplesFWIterator>
//    uint32_t read(SamplesFWIterator samples,
//                  uint32_t max_samples, const Selector& selector);

//    template<typename SamplesFWIterator>
//    uint32_t take(SamplesFWIterator samples,
//                  uint32_t max_samples, const Selector& selector);

//    // --- Back-Inserting Iterators: --- //
//    template<typename SamplesBIIterator>
//    uint32_t read(SamplesBIIterator samples, const Selector& selector);

//    template<typename SamplesBIIterator>
//    uint32_t take(SamplesBIIterator samples, const Selector& selector);

//};
class DataReader
{
};

} //namespace detail
} //namespace sub
} //namespace dds

/** @endcond */

#endif //EPROSIMA_TDDS_SUB_DETAIL_DATA_READER_HPP_


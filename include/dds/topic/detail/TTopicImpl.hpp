/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef EPROSIMA_DDS_TOPIC_TTOPIC_HPP_
#define EPROSIMA_DDS_TOPIC_TTOPIC_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/Topic.hpp>
//#include "org/opensplice/topic/TopicTraits.hpp"
//#include "org/opensplice/topic/TopicListener.hpp"

// Implementation

namespace dds {
namespace topic {


/***************************************************************************
 *
 * dds/topic/Topic<> WRAPPER implementation.
 * Declaration can be found in dds/topic/TTopic.hpp
 *
 ***************************************************************************/


//template<typename T, template<typename Q> class DELEGATE>
//Topic<T, DELEGATE>::Topic(const dds::domain::DomainParticipant& dp,
//                          const std::string& topic_name) :
//      dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(
//              dp,
//              topic_name,
//              "",
//              dp.is_nil() ? dds::topic::qos::TopicQos() : dp.default_topic_qos(),
//              NULL,
//              dds::core::status::StatusMask::none()))
//{
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//
//    this->delegate()->init(this->impl_);
//}
//
//template<typename T, template<typename Q> class DELEGATE>
//Topic<T, DELEGATE>::Topic(const dds::domain::DomainParticipant& dp,
//                          const std::string& topic_name,
//                          const std::string& type_name) :
//      dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(
//              dp,
//              topic_name,
//              type_name,
//              dp.is_nil() ? dds::topic::qos::TopicQos() : dp.default_topic_qos(),
//              NULL,
//              dds::core::status::StatusMask::none())),
//      dds::topic::TAnyTopic< DELEGATE<T> >(dds::core::Reference< DELEGATE<T>  >::delegate())
//{
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//
//    this->delegate()->init(this->impl_);
//}
//
//template<typename T, template<typename Q> class DELEGATE>
//Topic<T, DELEGATE>::Topic(const dds::domain::DomainParticipant& dp,
//                          const std::string& topic_name,
//                          const dds::topic::qos::TopicQos& qos,
//                          dds::topic::TopicListener<T>* listener,
//                          const dds::core::status::StatusMask& mask) :
//      dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(
//              dp,
//              topic_name,
//              "",
//              qos,
//              listener,
//              mask)),
//      dds::topic::TAnyTopic< DELEGATE<T> >(dds::core::Reference< DELEGATE<T>  >::delegate())
//{
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//
//    this->delegate()->init(this->impl_);
//}
//
//template<typename T, template<typename Q> class DELEGATE>
//Topic<T, DELEGATE>::Topic(const dds::domain::DomainParticipant& dp,
//                          const std::string& topic_name,
//                          const std::string& type_name,
//                          const dds::topic::qos::TopicQos& qos,
//                          dds::topic::TopicListener<T>* listener,
//                          const dds::core::status::StatusMask& mask) :
//      dds::core::Reference< DELEGATE<T> >(new DELEGATE<T>(
//              dp,
//              topic_name,
//              type_name,
//              qos,
//              listener,
//              mask)),
//      dds::topic::TAnyTopic< DELEGATE<T> >(dds::core::Reference< DELEGATE<T>  >::delegate())
//{
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//
//    this->delegate()->init(this->impl_);
//}
//
//template<typename T, template<typename Q> class DELEGATE>
//Topic<T, DELEGATE>::~Topic() { }
//
//template<typename T, template<typename Q> class DELEGATE>
//void Topic<T, DELEGATE>::listener(Listener* listener,
//                                  const dds::core::status::StatusMask& event_mask)
//{
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//
//    this->delegate()->listener(listener, event_mask);
//}
//
//template<typename T, template<typename Q> class DELEGATE>
//typename Topic<T, DELEGATE>::Listener* Topic<T, DELEGATE>::listener() const
//{
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//
//    return this->delegate()->listener();
//}
//
//
//}
//}
//
///***************************************************************************
// *
// * dds/topic/detail/Topic<> DELEGATE implementation.
// * Declaration can be found in dds/topic/detail/Topic.hpp
// *
// * Implementation and declaration have been separated because some circular
// * dependencies, like with TopicListener and AnyTopic.
// *
// ***************************************************************************/
//
//#include <dds/topic/detail/Topic.hpp>
//#include <dds/topic/AnyTopic.hpp>
//#include <dds/topic/TopicListener.hpp>
////#include <org/opensplice/core/ScopedLock.hpp>
//
//template<typename T>
//dds::topic::detail::Topic<T>::Topic(const dds::domain::DomainParticipant& dp,
//      const std::string& name,
//      const std::string& type_name,
//      const dds::topic::qos::TopicQos& qos,
//      dds::topic::TopicListener<T>* listener,
//      const dds::core::status::StatusMask& mask)
//    : org::opensplice::topic::TopicDescriptionDelegate(dp, name, type_name),
//      org::opensplice::topic::AnyTopicDelegate(qos, dp, name, type_name)
//{
//    ISOCPP_REPORT_STACK_NC_BEGIN();
//
//    dds::domain::DomainParticipant participant = dds::core::null;
//
//    /* The dp argument can be nil. Use the participant we know isn't nil because
//     * the TopicDescriptionDelegate would have created it when needed. */
//    participant = this->domain_participant();
//
//    // Set the correct (IDL) type_name in the TopicDescription.
//    org::opensplice::topic::TopicDescriptionDelegate::myTypeName = org::opensplice::topic::TopicTraits<T>::getTypeName();
//
//    // get and validate the kernel qos
//    org::opensplice::topic::qos::TopicQosDelegate tQos = qos.delegate();
//    tQos.check();
//    u_topicQos uTopicQos = tQos.u_qos();
//    u_participant uParticipant = participant->registerType(
//            org::opensplice::topic::TopicTraits<T>::getTypeName(),
//            org::opensplice::topic::TopicTraits<T>::getDescriptor(),
//            org::opensplice::topic::TopicTraits<T>::getDataRepresentationId(),
//            org::opensplice::topic::TopicTraits<T>::getTypeHash(),
//            org::opensplice::topic::TopicTraits<T>::getMetaData(),
//            org::opensplice::topic::TopicTraits<T>::getExtentions());
//
//    u_topic uTopic = u_topicNew(
//            uParticipant,
//            name.c_str(),
//            org::opensplice::topic::TopicDescriptionDelegate::myTypeName.c_str(),
//            org::opensplice::topic::TopicTraits<T>::getKeyList(),
//            uTopicQos);
//
//    u_topicQosFree(uTopicQos);
//
//    if (!uTopic) {
//        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Failed to create Topic");
//    }
//
//    this->userHandle = (u_object)uTopic;
//    this->listener_set((void*)listener, mask);
//}
//
//template<typename T>
//dds::topic::detail::Topic<T>::Topic(const dds::domain::DomainParticipant& dp,
//      const std::string& name,
//      const std::string& type_name,
//      const dds::topic::qos::TopicQos& qos,
//      u_topic uTopic)
//    : org::opensplice::topic::TopicDescriptionDelegate(dp, name, type_name),
//      org::opensplice::topic::AnyTopicDelegate(qos, dp, name, type_name)
//{
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//    this->userHandle = (u_object)uTopic;
//    this->listener_set((void*)NULL, dds::core::status::StatusMask::none());
//}
//
//
//template<typename T>
//dds::topic::detail::Topic<T>::~Topic()
//{
//    if (!closed) {
//        try {
//            close();
//        } catch (...) {
//
//        }
//    }
//}
//
//template<typename T>
//void
//dds::topic::detail::Topic<T>::close()
//{
//    this->listener(NULL, dds::core::status::StatusMask::none());
//    this->listener_dispatcher_reset();
//
//    org::opensplice::core::ScopedObjectLock scopedLock(*this);
//
//    if (this->hasDependents()) {
//        ISOCPP_THROW_EXCEPTION(ISOCPP_PRECONDITION_NOT_MET_ERROR, "Topic still has unclosed dependencies (e.g. Readers/Writers/ContentFilteredTopics)");
//    }
//
//    this->myParticipant.delegate()->remove_topic(*this);
//
//    org::opensplice::core::EntityDelegate::close();
//}
//
//template<typename T>
//void
//dds::topic::detail::Topic<T>::init(ObjectDelegate::weak_ref_type weak_ref)
//{
//    /* Set weak_ref before passing ourselves to other isocpp objects. */
//    this->set_weak_ref(weak_ref);
//    /* Register topic at participant. */
//    this->myParticipant.delegate()->add_topic(*this);
//    /* Use listener dispatcher from the publisher. */
//    this->listener_dispatcher_set(this->myParticipant.delegate()->listener_dispatcher_get());
//    /* This only starts listening when the status mask shows interest. */
//    this->listener_enable();
//    /* Enable when needed. */
//    if (this->myParticipant.delegate()->is_auto_enable()) {
//        this->enable();
//    }
//}
//
//template<typename T>
//void
//dds::topic::detail::Topic<T>::listener(TopicListener<T>* listener,
//                                       const dds::core::status::StatusMask& mask)
//{
//    /* EntityDelegate takes care of thread safety. */
//    this->listener_set((void*)listener, mask);
//    this->listener_enable();
//}
//
//template<typename T>
//dds::topic::TopicListener<T>*
//dds::topic::detail::Topic<T>::listener()
//{
//    return reinterpret_cast<dds::topic::TopicListener<T>*>(this->listener_get());
//}
//
//template<typename T>
//dds::topic::Topic<T, dds::topic::detail::Topic>
//dds::topic::detail::Topic<T>::wrapper()
//{
//
//    typename Topic::ref_type ref =
//            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<Topic<T> >(this->get_strong_ref());
//    dds::topic::Topic<T, dds::topic::detail::Topic> topic(ref);
//
//    return topic;
//}
//
//template<typename T>
//void
//dds::topic::detail::Topic<T>::listener_notify(
//        ObjectDelegate::ref_type source,
//        uint32_t                 triggerMask,
//        void                    *eventData,
//        void                    *l)
//{
//    /* The EntityDelegate takes care of the thread safety and always
//     * provides a listener and source. */
//    dds::topic::TopicListener<T>* listener =
//            reinterpret_cast<dds::topic::TopicListener<T>*>(l);
//    assert(listener);
//
//    /* Get Topic wrapper from given source EntityDelegate. */
//    typename Topic::ref_type ref =
//            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<Topic<T> >(source);
//    dds::topic::Topic<T, dds::topic::detail::Topic> topic(ref->wrapper());
//
//    if (triggerMask & V_EVENT_INCONSISTENT_TOPIC) {
//        dds::core::status::InconsistentTopicStatus status;
//        status.delegate().v_status(v_topicStatus(eventData)->inconsistentTopic);
//        listener->on_inconsistent_topic(topic, status);
//    }
//
//    if (triggerMask & V_EVENT_ALL_DATA_DISPOSED ) {
//        org::opensplice::topic::TopicListener<T>* extListener =
//                  dynamic_cast<org::opensplice::topic::TopicListener<T>*>(listener);
//        if (extListener) {
//            org::opensplice::core::status::AllDataDisposedTopicStatus status;
//            status.delegate().v_status(v_topicStatus(eventData)->allDataDisposed);
//            extListener->on_all_data_disposed(topic, status);
//        }
//    }
//}
//
//template<typename T>
//dds::topic::Topic<T, dds::topic::detail::Topic>
//dds::topic::detail::Topic<T>::discover_topic(
//        const dds::domain::DomainParticipant& dp,
//        const std::string& name,
//        const dds::core::Duration& timeout)
//{
//    u_topic uTopic = dp.delegate()->lookup_topic(name, timeout);
//
//    if (uTopic == NULL) {
//        return dds::core::null;
//    }
//
//    os_char *uTypename = u_topicTypeName(uTopic);
//    std::string type_name = uTypename;
//    os_free(uTypename);
//
//    u_topicQos uQos;
//    u_result uResult = u_topicGetQos(uTopic, &uQos);
//    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Failed to get user layer topic qos");
//
//    qos::TopicQos qos;
//    qos.delegate().u_qos(uQos);
//    u_topicQosFree(uQos);
//
//    typename dds::topic::Topic<T, dds::topic::detail::Topic>::DELEGATE_REF_T ref(new Topic<T>(dp, name, type_name, qos, uTopic));
//    ref->init(ref);
//
//    return dds::topic::Topic<T>(ref);
//}
//
//template<typename T>
//void
//dds::topic::detail::Topic<T>::discover_topics(
//        const dds::domain::DomainParticipant& dp,
//        std::vector<dds::topic::Topic<T, dds::topic::detail::Topic> >& topics,
//        uint32_t max_size)
//{
//    std::vector<u_topic> uTopics;
//
//    dp.delegate()->lookup_topics(topic_type_name<T>::value(), uTopics, max_size);
//
//    topics.clear();
//    topics.reserve(uTopics.size());
//
//    for (std::vector<u_topic>::const_iterator it = uTopics.begin(); it != uTopics.end(); ++it) {
//        u_topic uTopic = *it;
//        os_char *topic_name = u_topicName(uTopic);
//        os_char *type_name = u_topicTypeName(uTopic);
//
//        u_topicQos uQos;
//        u_result uResult = u_topicGetQos(uTopic, &uQos);
//        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Failed to get user layer topic qos");
//
//        dds::topic::qos::TopicQos qos;
//        qos.delegate().u_qos(uQos);
//        u_topicQosFree(uQos);
//
//        typename dds::topic::Topic<T, dds::topic::detail::Topic>::DELEGATE_REF_T ref(new Topic<T>(dp, topic_name, type_name, qos, uTopic));
//        ref->init(ref);
//
//        os_free(topic_name);
//        os_free(type_name);
//
//        topics.push_back(dds::topic::Topic<T>(ref));
//    }
//}

}
}

// End of implementation

#endif /* EPROSIMA_DDS_TOPIC_TTOPIC_HPP_ */

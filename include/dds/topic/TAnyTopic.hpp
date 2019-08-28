/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
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
 */

#ifndef OMG_DDS_TOPIC_TANYTOPIC_HPP_
#define OMG_DDS_TOPIC_TANYTOPIC_HPP_

#include <dds/core/conformance.hpp>
#include <dds/core/types.hpp>
#include <dds/core/ref_traits.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/qos/TopicQos.hpp>
#include <dds/topic/TopicDescription.hpp>


namespace dds
{
namespace topic
{
template <typename DELEGATE>
class TAnyTopic;
}
}


/**
 * @brief
 * Typeless base class for the typed Topic.
 *
 * Topics are created type specific (fi Topic<Foo::Bar> topic). However, there
 * are a few places in the API (and possibly application) where the type can not be known
 * while still some Topic has to be passed around, stored or even typeless functionality
 * called.<br>
 * The main examples in the API that needs typeless Topic is: DomainParticipantListener.
 *
 * @see dds::topic::Topic
 */
template <typename DELEGATE>
class dds::topic::TAnyTopic :
    public ::dds::core::TEntity< DELEGATE >,
    public ::dds::topic::TTopicDescription<DELEGATE>
{
    OMG_DDS_REF_TYPE_PROTECTED_DC(TAnyTopic, ::dds::topic::TTopicDescription, DELEGATE)
    OMG_DDS_IMPLICIT_REF_BASE(TAnyTopic)

    /** @cond */
    virtual ~TAnyTopic();
    /** @endcond */

public:

    //==========================================================================
    //== QoS Management

    /**
     * Gets the TopicQos setting for this instance.
     *
     * @return the qos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    dds::topic::qos::TopicQos qos() const;

    /**
     * This operation replaces the existing set of QosPolicy settings for a Topic.
     *
     * The parameter qos contains the object with the QosPolicy settings which is
     * checked for self-consistency and mutability.
     *
     * When the application tries to change a
     * QosPolicy setting for an enabled Topic, which can only be set before the
     * Topic is enabled, the operation will fail and a
     * ImmutablePolicyError is thrown. In other words, the application must
     * provide the presently set QosPolicy settings in case of the immutable QosPolicy
     * settings. Only the mutable QosPolicy settings can be changed.
     *
     * When the qos contains conflicting QosPolicy settings (not self-consistent),
     * the operation will fail and an InconsistentPolicyError is thrown.
     *
     * @param qos the qos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws dds::core::ImmutablePolicyError
     *                  The parameter qos contains an immutable QosPolicy setting with a
     *                  different value than set during enabling of the DataReader.
     * @throws dds::core::InconsistentPolicyError
     *                  The parameter qos contains conflicting QosPolicy settings,
     */
    void qos(const dds::topic::qos::TopicQos& qos);

    /** @copydoc dds::topic::TAnyTopic::qos(const dds::topic::qos::TopicQos& qos) */
    TAnyTopic& operator << (const dds::topic::qos::TopicQos& qos);

    /** @copydoc dds::topic::TAnyTopic::qos() */
    const TAnyTopic& operator >> (dds::topic::qos::TopicQos& qos) const;

    /**
     * This operation obtains the InconsistentTopicStatus object of the Topic.
     *
     * The InconsistentTopicStatus can also be monitored using a
     * TopicListener or by using the associated StatusCondition.
     *
     * @return the SampleRejectedStatus
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    ::dds::core::status::InconsistentTopicStatus
    inconsistent_topic_status() const;

#ifdef DOXYGEN_FOR_ISOCPP2
    /*
     * The above macro is never (and must never) be defined in normal compilation.
     *
     * The following code is for documenting proprietary API only.
     */

    /**
     * This operation allows the application to dispose of all of the instances for a
     * particular topic without the network overhead of using a separate dispose call for
     * each instance.
     *
     * @note This is a proprietary OpenSplice extension.
     *
     * Its effect is equivalent to invoking a separate dispose operation for
     * each individual instance on the DataWriter that owns it.
     * (See dds::pub::DataWriter::dispose_instance)
     *
     * <i>The dispose_all_data is an asynchronous C&M operation that is not part of a
     * coherent update; it operates on the DataReaders history cache and not on the
     * incomplete transactions. The dispose_all_data is effectuated as soon as a
     * transaction becomes complete and is inserted into the DataReaders history cache; at
     * that point messages will be inserted according to the destination_order qos
     * policy. For BY_SOURCE_TIMESTAMP all messages older than the
     * dispose_all_data will be disposed and all newer will be alive; for
     * BY_RECEPTION_TIMESTAMP all messages will be alive if the transaction is
     * completed after receiving the dispose_all_data command.</i>
     *
     * This operation only sets the instance state of the instances concerned to
     * NOT_ALIVE_DISPOSED. It does not unregister the instances, and so does not
     * automatically clean up the memory that is claimed by the instances in both the
     * DataReaders and DataWriters.
     *
     * <i>Blocking</i><br>
     * The blocking (or nonblocking) behaviour of this call is undefined.
     *
     * <i>Concurrency</i><br>
     * If there are subsequent calls to this function before the action has been completed
     * (completion of the disposes on all nodes, not simply return from the function), then
     * the behaviour is undefined.
     *
     * <i>Other notes</i><br>
     * The effect of this call on disposed_generation_count, generation_rank
     * and absolute_generation_rank is undefined.
     *
     * <i>Call</i><br>
     * This is a proprietary operation and can be called by using the operator->.
     * @code{.cpp}
     * dds::topic::Topic<Foo::Bar> topic(participant, "TopicName");
     * topic->dispose_all_data();
     * @endcode
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    void dispose_all_data();
#endif /* DOXYGEN_FOR_ISOCPP2 */

};


#endif /* OMG_DDS_TOPIC_TANYTOPIC_HPP_ */

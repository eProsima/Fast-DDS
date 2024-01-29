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

/*
 * TopicImpl.hpp
 *
 */

#ifndef _FASTDDS_TOPICIMPL_HPP_
#define _FASTDDS_TOPICIMPL_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipantImpl;
class DomainParticipant;
class TopicListener;
class Topic;
class TopicProxyFactory;

class TopicImpl
{
public:

    TopicImpl(
            TopicProxyFactory* factory,
            DomainParticipantImpl* p,
            TypeSupport type_support,
            const TopicQos& qos,
            TopicListener* listen);

    /**
     * Extends the check_qos() call, including the check for
     * resource limits policy.
     * @param qos Pointer to the qos to be checked.
     * @param type Pointer to the associated TypeSupport object.
     * @return True if correct.
     */
    static ReturnCode_t check_qos_including_resource_limits(
            const TopicQos& qos,
            const TypeSupport& type);

    /**
     * Checks the consistency of the qos configuration.
     * @param qos Pointer to the qos to be checked.
     * @return True if correct.
     */
    static ReturnCode_t check_qos(
            const TopicQos& qos);

    /**
     * Checks resource limits policy: Instance allocation consistency
     * @param qos Pointer to the qos to be checked.
     * @return True if correct.
     */
    static ReturnCode_t check_allocation_consistency(
            const TopicQos& qos);

    static bool can_qos_be_updated(
            const TopicQos& to,
            const TopicQos& from);

    static void set_qos(
            TopicQos& to,
            const TopicQos& from,
            bool first_time);

    virtual ~TopicImpl();

    const TopicQos& get_qos() const;

    ReturnCode_t set_qos(
            const TopicQos& qos);

    const TopicListener* get_listener() const;

    void set_listener(
            TopicListener* listener);

    void set_listener(
            TopicListener* listener,
            const StatusMask& status);

    DomainParticipant* get_participant() const;

    const TypeSupport& get_type() const;

    /**
     * Returns the most appropriate listener to handle the callback for the given status,
     * or nullptr if there is no appropriate listener.
     */
    TopicListener* get_listener_for(
            const StatusMask& status,
            const Topic* topic);

protected:

    TopicProxyFactory* factory_;
    DomainParticipantImpl* participant_;
    TypeSupport type_support_;
    TopicQos qos_;
    TopicListener* listener_;

};

} // dds
} // fastdds
} // eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_TOPICIMPL_HPP_ */

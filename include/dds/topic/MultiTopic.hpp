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

#ifndef OMG_DDS_TOPIC_MULTI_TOPIC_HPP_
#define OMG_DDS_TOPIC_MULTI_TOPIC_HPP_

#include <dds/core/detail/conformance.hpp>
#include <dds/topic/detail/MultiTopic.hpp>

namespace dds {
namespace topic {

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT

/**
 * <b><i>This operation is not yet implemented. It is scheduled for a future release.</i></b>
 *
 * @see @ref DCPS_Modules_TopicDefinition "Topic Definition"
 */
template<
    typename T,
    template<typename Q> class DELEGATE>
class MultiTopic : public TTopicDescription< DELEGATE<T> >
{
public:
    OMG_DDS_REF_TYPE_PROTECTED_DC_T(
            MultiTopic,
            dds::topic::TTopicDescription,
            T,
            DELEGATE)

    OMG_DDS_IMPLICIT_REF_BASE(
            MultiTopic)

    template<typename FWDIterator>
    MultiTopic(
            const dds::domain::DomainParticipant& dp,
            const std::string& name,
            const std::string expression,
            const FWDIterator& params_begin,
            const FWDIterator& params_end);

    virtual ~MultiTopic();

    const std::string expression() const;

    template<typename FWDIterator>
    void expression_parameters(
            const FWDIterator& params_begin,
            const FWDIterator& params_end);

    dds::core::StringSeq void expression_parameters() const;

};

template<
    typename T,
    template<typename Q> class DELEGATE = dds::topic::detail::MultiTopic>
class MultiTopic;

#endif  //OMG_DDS_MULTI_TOPIC_SUPPORT

} //namespace topic
} //namespace dds

#endif //OMG_DDS_TOPIC_MULTI_TOPIC_HPP_

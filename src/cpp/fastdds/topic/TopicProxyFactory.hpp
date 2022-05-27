// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * TopicProxyFactory.hpp
 */

#ifndef _FASTDDS_TOPICPROXYFACTORY_HPP_
#define _FASTDDS_TOPICPROXYFACTORY_HPP_

#include <fastrtps/types/TypesBase.h>

#include <fastdds/topic/TopicProxy.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A factory of TopicProxy objects for a specific topic.
 */
class TopicProxyFactory
{
public:

    /**
     * Create a new proxy object for the topic managed by the factory.
     *
     * @return Pointer to the created TopicProxy
     */
    TopicProxy* create_topic();

    /**
     * Delete a proxy object for the topic managed by the factory.
     *
     * @param proxy Pointer to the TopicProxy object to be deleted.
     *
     * @return PRECONDITION_NOT_MET if the @c proxy was not created by this factory, or has already being deleted.
     * @return PRECONDITION_NOT_MET if the @c proxy is still referenced.
     * @return OK if the @c proxy is correctly deleted.
     */
    ReturnCode_t delete_topic(
            TopicProxy* proxy);

    /**
     * Return whether this factory can be deleted.
     * Will disallow deletion if it still owns some proxy objects.
     *
     * @return true if the factory owns no proxy objects
     */
    bool can_be_deleted();

};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif /* _FASTDDS_TOPICPROXYFACTORY_HPP_ */

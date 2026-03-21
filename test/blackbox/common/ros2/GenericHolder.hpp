// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_BLACKBOX_COMMON_ROS2__GENERICHOLDER_HPP
#define FASTDDS_TEST_BLACKBOX_COMMON_ROS2__GENERICHOLDER_HPP

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>

#include <gtest/gtest.h>

// *INDENT-OFF* Uncrustify makes a mess with this kind of macros
/**
 * @brief Generic holder class for a DDS entity.
 * This macro generates a holder class for a DDS entity along with it's factory so that the entity is automatically
 * released when the holder is destroyed.
 * The generated class has a custom-named getter method to access the entity.
 * The macro allows to specify the factory class, the entity class, the factory's method to release the entity and the
 * getter method name.
 *
 * @param _Factory  The class of the factory that created the entity (e.g. Publisher).
 * @param _Entity   The class of the entity (e.g., DataWriter).
 * @param _Release  The method of the factory to release the entity (e.g., delete_datawriter).
 * @param _Getter   The name of the getter method to access the entity (for instance, writer).
 *
 * Examples:
 * GENERIC_HOLDER_CLASS(Publisher, DataWriter, delete_datawriter, writer) generates DataWriterHolder.
 * GENERIC_HOLDER_CLASS(Subscriber, DataReader, delete_datareader, reader) generates DataReaderHolder.
 * GENERIC_HOLDER_CLASS(DomainParticipant, Publisher, delete_publisher, publisher) generates PublisherHolder.
 * GENERIC_HOLDER_CLASS(DomainParticipant, Subscriber, delete_subscriber, subscriber) generates SubscriberHolder.
 * GENERIC_HOLDER_CLASS(DomainParticipant, Topic, delete_topic, topic) generates TopicHolder.
 * GENERIC_HOLDER_CLASS(DomainParticipantFactory, DomainParticipant, delete_participant, participant) generates DomainParticipantHolder.
 */
#define GENERIC_HOLDER_CLASS(_Factory, _Entity, _Release, _Getter) \
class _Entity##Holder                                              \
{                                                                  \
public:                                                            \
    _Entity##Holder(                                               \
            _Factory* factory,                                     \
            _Entity* entity)                                       \
        : factory_(factory)                                        \
        , entity_(entity)                                          \
    {                                                              \
    }                                                              \
                                                                   \
    ~_Entity##Holder()                                             \
    {                                                              \
        if (nullptr != factory_ && nullptr != entity_)             \
        {                                                          \
            EXPECT_EQ(RETCODE_OK, factory_->_Release(entity_));    \
        }                                                          \
    }                                                              \
                                                                   \
    _Entity* _Getter()                                             \
    {                                                              \
        return entity_;                                            \
    }                                                              \
                                                                   \
private:                                                           \
                                                                   \
    _Factory* factory_ = nullptr;                                  \
    _Entity* entity_ = nullptr;                                    \
                                                                   \
};
// *INDENT-ON*

#endif  // FASTDDS_TEST_BLACKBOX_COMMON_ROS2__GENERICHOLDER_HPP

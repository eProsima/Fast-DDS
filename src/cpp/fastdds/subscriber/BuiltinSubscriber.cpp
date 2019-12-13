// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file BuiltinSubscriber.cpp
 *
 */

#include <fastdds/dds/subscriber/BuiltinSubscriber.hpp>
#include <fastdds/dds/topic/BuiltinTopicKey.hpp>

#include <mutex>

namespace eprosima {
namespace fastdds {
namespace dds {

static bool g_instance_initialized = false;
static std::mutex g_mtx;
static BuiltinSubscriber* g_instance = nullptr;

BuiltinSubscriber* BuiltinSubscriber::get_instance()
{
    if (!g_instance_initialized)
    {
        std::lock_guard<std::mutex> lock(g_mtx);
        if (g_instance == nullptr)
        {
            g_instance = new BuiltinSubscriber();
            g_instance_initialized = true;
        }
    }
    return g_instance;
}

bool BuiltinSubscriber::delete_instance()
{
    std::lock_guard<std::mutex> lock(g_mtx);
    if (g_instance_initialized && g_instance != nullptr)
    {
        delete g_instance;
        g_instance = nullptr;
        g_instance_initialized = false;
        return true;
    }
    return false;
}

ParticipantBuiltinTopicData* BuiltinSubscriber::get_participant_data(
        const fastrtps::rtps::InstanceHandle_t& participant_handle)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    auto it = participant_data_.find(participant_handle);
    if (it != participant_data_.end())
    {
        return &it->second;
    }
    return nullptr;
}

void BuiltinSubscriber::add_participant_data(
        const fastrtps::rtps::InstanceHandle_t& participant_handle,
        const DomainParticipantQos& pqos)
{
    ParticipantBuiltinTopicData data;
    data.key(BuiltinTopicKey(fastrtps::rtps::iHandle2GUID(participant_handle)));
    data.user_data(pqos.user_data);

    std::lock_guard<std::mutex> lock(g_mtx);
    participant_data_[participant_handle] = data;
}

void BuiltinSubscriber::add_participant_data(
        const fastrtps::rtps::InstanceHandle_t& participant_handle,
        const UserDataQosPolicy& user_data)
{
    ParticipantBuiltinTopicData data;
    data.key(BuiltinTopicKey(fastrtps::rtps::iHandle2GUID(participant_handle)));
    data.user_data(user_data);

    std::lock_guard<std::mutex> lock(g_mtx);
    participant_data_[participant_handle] = data;
}

void BuiltinSubscriber::delete_participant_data(
        const fastrtps::rtps::InstanceHandle_t& participant_handle)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    auto it = participant_data_.find(participant_handle);
    if (it != participant_data_.end())
    {
        participant_data_.erase(it);
    }
}

TopicBuiltinTopicData* BuiltinSubscriber::get_topic_data(
        const fastrtps::rtps::InstanceHandle_t& topic_handle)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    auto it = topic_data_.find(topic_handle);
    if (it != topic_data_.end())
    {
        return &it->second;
    }
    return nullptr;
}

void BuiltinSubscriber::add_topic_data(
        const fastrtps::rtps::InstanceHandle_t& topic_handle,
        std::string topic_name,
        std::string type_name,
        const TopicQos& tqos)
{
    TopicBuiltinTopicData data;
    data.key(BuiltinTopicKey(fastrtps::rtps::iHandle2GUID(topic_handle)));
    data.name(topic_name);
    data.type_name(type_name);
    data.fill_with_topic_qos(tqos);

    std::lock_guard<std::mutex> lock(g_mtx);
    topic_data_[topic_handle] = data;
}

void BuiltinSubscriber::delete_topic_data(
        const fastrtps::rtps::InstanceHandle_t& topic_handle)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    auto it = topic_data_.find(topic_handle);
    if (it != topic_data_.end())
    {
        topic_data_.erase(it);
    }
}

PublicationBuiltinTopicData* BuiltinSubscriber::get_publication_data(
        const fastrtps::rtps::InstanceHandle_t& writer_handle)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    auto it = publication_data_.find(writer_handle);
    if (it != publication_data_.end())
    {
        return &it->second;
    }
    return nullptr;
}

void BuiltinSubscriber::add_publication_data(
        const fastrtps::rtps::InstanceHandle_t& writer_handle,
        const fastrtps::rtps::InstanceHandle_t& participant_handle,
        std::string topic_name,
        std::string type_name,
        const PublisherQos& pqos,
        const DataWriterQos& dwqos)

{
    PublicationBuiltinTopicData data;
    data.key(BuiltinTopicKey(fastrtps::rtps::iHandle2GUID(writer_handle)));
    data.participant_key(BuiltinTopicKey(fastrtps::rtps::iHandle2GUID(participant_handle)));
    data.name(topic_name);
    data.type_name(type_name);
    data.fill_with_publication_qos(pqos, dwqos);

    std::lock_guard<std::mutex> lock(g_mtx);
    publication_data_[writer_handle] = data;
}

void BuiltinSubscriber::delete_publication_data(
        const fastrtps::rtps::InstanceHandle_t& writer_handle)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    auto it = publication_data_.find(writer_handle);
    if (it != publication_data_.end())
    {
        publication_data_.erase(it);
    }
}


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Publisher.cpp
 *
 */

#include <fastrtps/publisher/Publisher.h>
#include "PublisherImpl.h"
#include "../participant/ParticipantImpl.h"

#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

Publisher::Publisher(PublisherImpl* pimpl) : mp_impl(pimpl)
{
    // TODO Auto-generated constructor stub
}

Publisher::~Publisher() {
    // TODO Auto-generated destructor stub
}

const PublisherAttributes& Publisher::getAttributes() const
{
    return mp_impl->getAttributes();
}

bool Publisher::updateAttributes(const PublisherAttributes& att)
{
    return mp_impl->updateAttributes(att);
}

DataWriter* Publisher::create_writer(
        const TopicAttributes& topic_att,
        const WriterQos& qos,
        DataWriterListener* listener)
{
    return mp_impl->create_writer(topic_att, qos, listener);
}

bool Publisher::update_writer(
            DataWriter* Writer,
            const TopicAttributes& topicAtt,
            const WriterQos& wqos)
{
    return mp_impl->update_writer(Writer, topicAtt, wqos);
}

PublisherListener* Publisher::listener() const
{
    return mp_impl->listener();
}

void Publisher::listener(PublisherListener* listener)
{
    mp_impl->listener(listener);
}

bool Publisher::delete_writer(DataWriter* writer)
{
    return mp_impl->delete_writer(writer);
}

DataWriter* Publisher::lookup_writer(const std::string& topic_name) const
{
    return mp_impl->lookup_writer(topic_name);
}

Participant* Publisher::participant() const
{
    return mp_impl->participant()->participant();
}

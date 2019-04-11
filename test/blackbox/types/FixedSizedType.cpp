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
 * @file FixedSizedTopic.cpp
 *
 */

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "FixedSizedType.h"

using namespace eprosima::fastrtps::rtps;

FixedSizedType::FixedSizedType() {
    setName("FixedSizedType");
    m_typeSize = (uint32_t)FixedSized::getMaxCdrSerializedSize() + 4 /*encapsulation*/;
    m_isGetKeyDefined = false;

}

FixedSizedType::~FixedSizedType() {
    // TODO Auto-generated destructor stub
}

bool FixedSizedType::serialize(void* data, SerializedPayload_t* payload)
{
    FixedSized* fs = (FixedSized*) data;	
    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    // Serialize encapsulation
    ser.serialize_encapsulation();
    //serialize the object:
    fs->serialize(ser);
    payload->length = (uint32_t)ser.getSerializedDataLength();
    return true;
}

bool FixedSizedType::deserialize(SerializedPayload_t* payload, void* data)
{
    FixedSized* fs = (FixedSized*) data;
    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length);
    // Object that serializes the data.
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.
    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    //serialize the object:
    fs->deserialize(deser);
    return true;
}

std::function<uint32_t()> FixedSizedType::getSerializedSizeProvider(void *data)
{
    return [data]() -> uint32_t { 
        return (uint32_t)type::getCdrSerializedSize(*static_cast<FixedSized*>(data)) + 4 /*encapsulation*/;
    };
}

void* FixedSizedType::createData()
{
    return (void*)new FixedSized();
}
void FixedSizedType::deleteData(void* data)
{
    delete((FixedSized*)data);
}

bool FixedSizedType::getKey(void* /*data*/, InstanceHandle_t* /*ihandle*/, bool /*force_md5*/)
{
    return false;
}

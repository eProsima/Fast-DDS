// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file GenericTopicDataType.cpp
 *
 */

#include <string>

#include <xtypes/idl/idl.hpp>
#include <xtypes/idl/parser.hpp>

#include "GenericTopicDataType.h"

GenericTopicDataType::GenericTopicDataType(
    const std::string& filename,
    const std::string& type_name)
{
    context_ = eprosima::xtypes::idl::parse_file(filename);
    type_struct_ = &(context_.module().structure(type_name));

    std::cout << "Type: '" << type_name << "' loaded with type struct: " << *type_struct_ << std::endl;

    // Set values for data Type
    setName(type_name.c_str());
    // Max size of the type. It includes 4 bytes from uint and 251 for string (+ \0)
    m_typeSize = 256u;
    m_isGetKeyDefined = false;
}

GenericTopicDataType::~GenericTopicDataType()
{
}

bool GenericTopicDataType::serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload)
{
    // Get values from data
    eprosima::xtypes::DynamicData* data_ = static_cast<eprosima::xtypes::DynamicData*>(data);
    uint32_t index = (*data_)["index"].value<uint32_t>();
    std::string msg = (*data_)["message"].value<std::string>();

    // Set values in payload
    memcpy(payload->data, &index, sizeof(uint32_t));
    memcpy(payload->data + sizeof(uint32_t), msg.c_str(), msg.size() + 1);

    // Set payload length for this message
    payload->length = sizeof(uint32_t) + msg.size() + 1; // Get the serialized length of int + string

    return true;
}

bool GenericTopicDataType::deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data)
{
    // Read index
    uint32_t index;
    memcpy(&index, payload->data, sizeof(uint32_t));

    // The rest of the message is the string
    std::string msg(
        reinterpret_cast<const char*>(payload->data + sizeof(uint32_t)), payload->length - sizeof(uint32_t));

    // Load these values into the dynamic data
    eprosima::xtypes::DynamicData* data_ = static_cast<eprosima::xtypes::DynamicData*>(data);
    (*data_)["index"] = index;
    (*data_)["message"] = msg;

    return true;
}

std::function<uint32_t()> GenericTopicDataType::getSerializedSizeProvider(void* data)
{
    return [data]() -> uint32_t
    {
        eprosima::xtypes::DynamicData* data_ = static_cast<eprosima::xtypes::DynamicData*>(data);

        return sizeof(uint32_t) + (*data_)["message"].value<std::string>().size();
    };
}

bool GenericTopicDataType::getKey(
        void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
        bool force_md5)
{
    static_cast<void>(data);
    static_cast<void>(ihandle);
    static_cast<void>(force_md5);
    logWarning(GENERIC_TOPICDATATYPE, "Keys are not implemented");
    return false;
}

void* GenericTopicDataType::createData()
{
    eprosima::xtypes::DynamicData* new_data = new eprosima::xtypes::DynamicData(*type_struct_);
    return static_cast<void*>(new_data);
}

void GenericTopicDataType::deleteData(void * data)
{
    delete(static_cast<eprosima::xtypes::DynamicData*>(data));
}

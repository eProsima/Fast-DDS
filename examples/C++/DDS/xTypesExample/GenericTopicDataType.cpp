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

GenericTopicDataType::GenericTopicDataType(const std::string& type_name)
    : type_struct_(nullptr)
{
    eprosima::xtypes::idl::Context context = eprosima::xtypes::idl::parse_file("xtypesExample.idl");
    type_struct_ = &context.module().structure(type_name);

    setName(type_name.c_str());
    m_typeSize = sizeof(uint32_t);
    m_isGetKeyDefined = false;
}

GenericTopicDataType::~GenericTopicDataType()
{
}

bool GenericTopicDataType::serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload)
{
    logError(GENERIC_TOPICDATATYPE, "serialize is not implemented");
    return false;
}

bool GenericTopicDataType::deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data)
{
    logError(GENERIC_TOPICDATATYPE, "deserialize is not implemented");
    return false;
}

std::function<uint32_t()> GenericTopicDataType::getSerializedSizeProvider(void* data)
{
    logError(GENERIC_TOPICDATATYPE, "getSerializedSizeProvider is not implemented");
    return []() -> uint32_t
    {
        // TODO
        return sizeof(uint32_t);
    };
}

bool GenericTopicDataType::getKey(
        void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
        bool force_md5)
{
    logWarning(GENERIC_TOPICDATATYPE, "Keys are not implemented");
    return false;
}

void* GenericTopicDataType::createData()
{
    return reinterpret_cast<void*>(new eprosima::xtypes::DynamicData(*type_struct_));
}

void GenericTopicDataType::deleteData(void * data)
{
    delete(reinterpret_cast<eprosima::xtypes::DynamicData*>(data));
}

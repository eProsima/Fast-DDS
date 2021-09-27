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

/*!
 * @file GenericTopicDataType.h
 */


#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_XTYPESEXAMPLE_GENERICTYPESUPPORTNOKEYS_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_XTYPESEXAMPLE_GENERICTYPESUPPORTNOKEYS_H_

#include <xtypes/idl/idl.hpp>
#include <xtypes/StructType.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

/*!
 * @brief This class represents the TopicDataType of a generic file load by xtypes from an IDL or implemented by code.
 */
class GenericTopicDataType : public eprosima::fastdds::dds::TopicDataType
{
public:

    GenericTopicDataType(const std::string& filename, const std::string& type_name);
    virtual ~GenericTopicDataType();
    virtual std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
    virtual bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle,
        bool force_md5 = false) override;
    virtual void* createData() override;
    virtual void deleteData(void * data) override;

    // WARNING
    // These funciona must be implemented depending on the type that will be written
    // TODO: write a generic function for xtypes objects
    virtual bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) override;
    virtual bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) override;

private:
    eprosima::xtypes::idl::Context context_;
    eprosima::xtypes::StructType* type_struct_;
};

#endif // _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_XTYPESEXAMPLE_GENERICTYPESUPPORTNOKEYS_H_
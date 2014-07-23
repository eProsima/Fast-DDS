/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimartps/dds/DDSTopicDataType.h"

using namespace eprosima;
using namespace dds;

/**
 * @brief The ShapeTopicDataType class, implements the serializing and deserializing methods.
 */
class ShapeTopicDataType : public DDSTopicDataType
{
public:
    ShapeTopicDataType();
    virtual ~ShapeTopicDataType();
    bool deserialize(SerializedPayload_t *payload, void *data);
    bool getKey(void *data, InstanceHandle_t *ihandle);
    bool serialize(void *data, SerializedPayload_t *payload);
};

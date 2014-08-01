/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimartps/dds/DDSTopicDataType.h"

#include "eprosimashapesdemo/shapesdemo/Shape.h"

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

typedef std::pair<SD_COLOR,InstanceHandle_t> colorPair;

class ColorInstanceHandle
{
public:
    colorPair PurpleIH;
    colorPair BlueIH;
    colorPair RedIH;
    colorPair GreenIH;
    colorPair YellowIH;
    colorPair CyanIH;
    colorPair MagentaIH;
    colorPair OrangeIH;
    ShapeTopicDataType m_topic;
    ColorInstanceHandle()
    {
        ShapeType shape;
        shape.define(SD_PURPLE);
        m_topic.getKey((void*)&shape,&PurpleIH.second);
        shape.define(SD_BLUE);
        m_topic.getKey((void*)&shape,&BlueIH.second);
        shape.define(SD_RED);
        m_topic.getKey((void*)&shape,&RedIH.second);
        shape.define(SD_GREEN);
        m_topic.getKey((void*)&shape,&GreenIH.second);
        shape.define(SD_YELLOW);
        m_topic.getKey((void*)&shape,&YellowIH.second);
        shape.define(SD_CYAN);
        m_topic.getKey((void*)&shape,&CyanIH.second);
        shape.define(SD_MAGENTA);
        m_topic.getKey((void*)&shape,&MagentaIH.second);
        shape.define(SD_ORANGE);
        m_topic.getKey((void*)&shape,&OrangeIH.second);
//        cout << PurpleIH.second << endl;
//        cout << BlueIH.second << endl;
//        cout << RedIH.second<<endl;
    }
    ~ColorInstanceHandle()
    {

    }
};

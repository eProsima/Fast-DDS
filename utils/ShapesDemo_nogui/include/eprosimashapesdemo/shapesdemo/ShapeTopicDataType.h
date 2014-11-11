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
    colorPair GrayIH;
    colorPair BlackIH;
    ShapeTopicDataType m_topic;
    ColorInstanceHandle()
    {
        Shape shape;
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
        shape.define(SD_GRAY);
        m_topic.getKey((void*)&shape,&GrayIH.second);
        shape.define(SD_BLACK);
        m_topic.getKey((void*)&shape,&BlackIH.second);
        //        cout << PurpleIH.second << endl;
        //        cout << BlueIH.second << endl;
        //        cout << RedIH.second<<endl;
    }
    ~ColorInstanceHandle()
    {

    }
};


const ColorInstanceHandle c_ShapesHandles;


inline SD_COLOR getColorFromInstanceHandle(InstanceHandle_t& iHandle)
{
    if(iHandle == c_ShapesHandles.BlueIH.second)
    {
        return SD_BLUE;
    }
    if(iHandle == c_ShapesHandles.PurpleIH.second)
    {
        return SD_PURPLE;
    }
    if(iHandle == c_ShapesHandles.RedIH.second)
    {
        return SD_RED;
    }
    if(iHandle == c_ShapesHandles.MagentaIH.second)
    {
        return SD_MAGENTA;
    }
    if(iHandle == c_ShapesHandles.OrangeIH.second)
    {
        return SD_ORANGE;
    }
    if(iHandle == c_ShapesHandles.YellowIH.second)
    {
        return SD_YELLOW;
    }
    if(iHandle == c_ShapesHandles.GreenIH.second)
    {
        return SD_GREEN;
    }
    if(iHandle == c_ShapesHandles.CyanIH.second)
    {
        return SD_CYAN;
    }
    if(iHandle == c_ShapesHandles.GrayIH.second)
    {
        return SD_GRAY;
    }
    return SD_BLACK;
}

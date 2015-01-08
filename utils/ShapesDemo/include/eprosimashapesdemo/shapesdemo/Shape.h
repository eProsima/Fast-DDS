/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Shape.h
 *
 */

#ifndef SHAPE_H_
#define SHAPE_H_
#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/common/Guid.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#include <cstdint>
#include <sstream>
#include <QString>

#include "eprosimashapesdemo/shapesdemo/ShapeDefinitions.h"

/**
 * @brief The Shape class, defines a shape
 */
class Shape {
public:
    Shape():m_x(0),m_y(0),m_size(0),m_strength(0),
        m_hasOwner(true),m_dirX(0),m_dirY(0),m_changeDir(true){}
    virtual ~Shape(){}
    void define(SD_COLOR color=SD_BLUE,
                uint32_t x=30,uint32_t y =30,uint32_t size=30)
    {
        m_color = color;
        m_x = x;
        m_y =y;
        m_size = size;
        m_strength = 0;
    }
    TYPESHAPE m_type;
    SD_COLOR m_color;
    uint32_t m_x;
    uint32_t m_y;
    uint32_t m_size;
    Time_t m_time;
    GUID_t m_writerGuid;
    uint32_t m_strength;
    bool m_hasOwner;
    float m_dirX;
    float m_dirY;
    bool m_changeDir;


};



#endif /* SHAPE_H_ */

/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimashapesdemo/shapesdemo/ShapeType.h"


ShapeType::ShapeType()
{
    define();
}
ShapeType::~ShapeType()
{

}

void ShapeType::define(TYPE type,SD_COLOR color,
                       uint32_t x,uint32_t y ,uint32_t size)
{
    m_type = type;
    setColor(color);
    m_x = x;
    m_y =y;
    m_size = size;
}

void ShapeType::setColor(SD_COLOR c)
{
    m_color = c;
    switch(c)
    {
    case SD_PURPLE: m_str_color = "PURPLE";break;
    case SD_BLUE: m_str_color = "BLUE";break;
    case SD_RED: m_str_color = "RED";break;
    case SD_GREEN: m_str_color = "GREEN";break;
    case SD_YELLOW: m_str_color = "YELLOW";break;
    case SD_CYAN: m_str_color = "CYAN";break;
    case SD_MAGENTA: m_str_color = "MAGENTA";break;
    case SD_ORANGE: m_str_color = "ORANGE";break;
    }
}
void ShapeType::setColor(std::string str)
{
    if(str == "PURPLE") m_color = SD_PURPLE;
    if(str == "BLUE") m_color = SD_BLUE;
    if(str == "RED") m_color = SD_RED;
    if(str == "GREEN") m_color = SD_GREEN;
    if(str == "YELLOW") m_color = SD_YELLOW;
    if(str == "CYAN") m_color = SD_CYAN;
    if(str == "MAGENTA") m_color = SD_MAGENTA;
    if(str == "ORANGE") m_color = SD_ORANGE;
}

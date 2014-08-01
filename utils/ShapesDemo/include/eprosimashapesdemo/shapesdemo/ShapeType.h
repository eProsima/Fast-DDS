/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef SHAPETYPE_H
#define SHAPETYPE_H


#include <cstdint>
#include <sstream>
#include "eprosimartps/common/types/Time_t.h"
#include "eprosimartps/common/types/Guid.h"

using namespace eprosima::rtps;


/**
 * @brief The ShapeType class, defined a shape.
 */
class ShapeType{
public:
    ShapeType();
    ~ShapeType();
    /**
     * @brief Define the shape.
     * @param color color
     * @param x x position.
     * @param y y position.
     * @param size size.
     */
    void define(SD_COLOR color=SD_BLUE,
                uint32_t x=30,uint32_t y =30,uint32_t size=30);
    void setColor(SD_COLOR c);
    void setColor(const char* strin);
    void setColor(const char c);
    std::string getColorStr()
    {
        return std::string(m_char_color);
    }
    SD_COLOR getColor()
    {
        return m_color;
    }

private:
    char m_char_color[8]; //KEY
   // std::string m_str_color; //KEY

};
#include <list>
#include <vector>
#endif

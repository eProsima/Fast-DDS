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

enum SD_COLOR
{
    SD_PURPLE,
    SD_BLUE,
    SD_RED,
    SD_GREEN,
    SD_YELLOW,
    SD_CYAN,
    SD_MAGENTA,
    SD_ORANGE
};

enum TYPESHAPE{
    SQUARE,
    CIRCLE,
    TRIANGLE
};

class ShapeType{
public:
    ShapeType();
    ~ShapeType();
    void define(SD_COLOR color=SD_BLUE,
                uint32_t x=30,uint32_t y =30,uint32_t size=30);
    void setColor(SD_COLOR c);
    void setColor(std::string str);
    std::string getColorStr()
    {
        return m_str_color;
    }
    SD_COLOR getColor()
    {
        return m_color;
    }
    uint32_t m_x;
    uint32_t m_y;
    uint32_t m_size;
private:
    std::string m_str_color; //KEY
    SD_COLOR m_color;
};

#endif

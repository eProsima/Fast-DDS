/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimashapesdemo/shapesdemo/ShapeType.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <string.h>

#if defined(_WIN32)
#define MYCOPYSTR strcpy_s
#else
#define MYCOPYSTR strcpy
#endif


ShapeType::ShapeType()
{
    define();
}
ShapeType::~ShapeType()
{

}

void ShapeType::define(SD_COLOR color,
                       uint32_t x,uint32_t y ,uint32_t size)
{
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
    case SD_PURPLE:     MYCOPYSTR(m_char_color,"PURPLE");break;
    case SD_BLUE:       MYCOPYSTR(m_char_color,"BLUE");break;
    case SD_RED:        MYCOPYSTR(m_char_color,"RED");break;
    case SD_GREEN:      MYCOPYSTR(m_char_color,"GREEN");break;
    case SD_YELLOW:     MYCOPYSTR(m_char_color,"YELLOW");break;
    case SD_CYAN:       MYCOPYSTR(m_char_color,"CYAN");break;
    case SD_MAGENTA:    MYCOPYSTR(m_char_color,"MAGENTA");break;
    case SD_ORANGE:     MYCOPYSTR(m_char_color,"ORANGE");break;
    }
}
void ShapeType::setColor(const char* strin)
{
    if(strcmp(strin,"PURPLE")==0)
    {
        setColor(SD_PURPLE);
    }
    if(strcmp(strin,"BLUE")==0)
    {
        setColor(SD_BLUE);
    }
    if(strcmp(strin,"RED")==0)
    {
        setColor(SD_RED);
    }
    if(strcmp(strin,"GREEN")==0)
    {
        setColor(SD_GREEN);
    }
    if(strcmp(strin,"YELLOW")==0)
    {
        setColor(SD_YELLOW);
    }
    if(strcmp(strin,"CYAN")==0)
    {
        setColor(SD_CYAN);
    }
    if(strcmp(strin,"MAGENTA")==0)
    {
        setColor(SD_MAGENTA);
    }
    if(strcmp(strin,"ORANGE")==0)
    {
        setColor(SD_ORANGE);
    }
}

void ShapeType::setColor(const char c)
{
    switch (c) {
    case 'P': setColor(SD_PURPLE);   break;
    case 'B': setColor(SD_BLUE);   break;
    case 'R': setColor(SD_RED);   break;
    case 'G': setColor(SD_GREEN);   break;
    case 'Y': setColor(SD_YELLOW);   break;
    case 'C': setColor(SD_CYAN);   break;
    case 'M': setColor(SD_MAGENTA);   break;
    case 'O': setColor(SD_ORANGE);   break;
    }
}

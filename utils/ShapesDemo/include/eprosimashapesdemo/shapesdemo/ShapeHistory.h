/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#ifndef SHAPEHISTORY_H
#define SHAPEHISTORY_H

#include "eprosimashapesdemo/shapesdemo/Shape.h"
#include <vector>
#include <list>
#include <cstdint>

/**
 * @brief The ShapeFilter class, represents a filter.
 */
class ShapeFilter
{
public:
    ShapeFilter(): m_maxX(MAX_DRAW_AREA_X),m_minX(0),m_maxY(MAX_DRAW_AREA_Y),m_minY(0),
        m_useContentFilter(false),m_useTimeFilter(false)
    {

    }
   ~ShapeFilter(){}
    uint32_t m_maxX;
    uint32_t m_minX;
    uint32_t m_maxY;
    uint32_t m_minY;
    Duration_t m_minimumSeparation;
    bool m_useContentFilter;
    bool m_useTimeFilter;

};



class ShapeHistory
{
public:
    ShapeHistory():m_history_depth(0),m_isExclusiveOwnership(false){}
    virtual ~ShapeHistory(){}
    std::vector<std::list<Shape>> m_history;
    bool addToHistory(Shape& sh);
private:

    bool findInstance(Shape& sh,std::vector<std::list<Shape>>::iterator* it);
    void addNewInstance(Shape& sh);
    bool passContentFilter(Shape& sh);
    bool passTimeFilter(Shape& sh_in, Shape& sh_last);
    void addShapeToList(Shape&sh,std::list<Shape>& list);
    void addShape(Shape& sh,std::list<Shape>& list);
    void addShapeExclusive(Shape& sh,std::list<Shape>& list);
public:
    void dispose(SD_COLOR& color);
    void unregister(SD_COLOR& color);
    void adjustContentFilter(ShapeFilter& filter);
    void removedOwner(GUID_t& guid);

    uint32_t m_history_depth;
    ShapeFilter m_filter;
    bool m_isExclusiveOwnership;
};

#endif // SHAPEHISTORY_H

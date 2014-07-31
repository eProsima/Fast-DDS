/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Shape.h
 *
 */

#ifndef SHAPE_H_
#define SHAPE_H_
#include <list>
#include <vector>
#include "eprosimashapesdemo/shapesdemo/ShapeType.h"

#include <QString>

/**
 * @brief The Shape class, defines a shape and its History (in case is a Subscriber.).
 */
class Shape {
public:
    Shape();
	virtual ~Shape();
    /**
     * @brief getShapeQStr, get the type fo shape as a string.
     * @return QString.
     */
    QString getShapeQStr()
    {
        if(m_type == SQUARE)
            return "Square";
        if(m_type == CIRCLE)
            return "Circle";
        if(m_type == TRIANGLE)
            return "Triangle";
        return "ERROR";
    }

    TYPESHAPE m_type;
	ShapeType m_mainShape;
    std::vector<std::list<ShapeType> > m_shapeHistory;
    //std::list<ShapeType> m_history;

    float m_dirX;
    float m_dirY;
    bool m_changeDir;


};



#endif /* SHAPE_H_ */

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
#include <queue>
#include "eprosimashapesdemo/shapesdemo/ShapeType.h"



class Shape {
public:
    Shape();
	virtual ~Shape();

	ShapeType m_mainShape;
    std::queue<ShapeType> m_history;

    float m_dirX;
    float m_dirY;
    bool m_changeDir;


};



#endif /* SHAPE_H_ */

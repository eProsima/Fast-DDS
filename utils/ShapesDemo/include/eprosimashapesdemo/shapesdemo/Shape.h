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
#include <vector>
#include "eprosimashapesdemo/shapesdemo/ShapeType.h"

namespace eprosima {
namespace rtps {

class Shape {
public:
	Shape();
	virtual ~Shape();

private:
	ShapeType m_mainShape;
	std::vector<ShapeType> m_history;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SHAPE_H_ */

/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapesDemo.h
 *
 */

#ifndef SHAPESDEMO_H_
#define SHAPESDEMO_H_
#include "eprosimartps/rtps_all.h"


#include "eprosimashapesdemo/shapesdemo/ShapeType.h"

class ShapesDemo
{
public:
	ShapesDemo();
	~ShapesDemo();
	void init(uint32_t domainId);
	void stop();

private:
	std::vector<Publisher*> m_publishers;
	std::vector<Subscriber*> m_subscribers;
	Participant* mp_participant;

	std::vector<ShapeType*> m_shapes;
    bool m_isInitialized;
};



#endif /* SHAPESDEMO_H_ */

/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Time_t.h
 *
 *  Created on: May 22, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef TIME_T_H_
#define TIME_T_H_

#include <cmath>

namespace eprosima{
namespace rtps{
//!Structure Time_t, used to describe times.
typedef struct Time_t{
	int32_t seconds;
	uint32_t nanoseconds;
	int64_t to64time(){
		return (int64_t)seconds+((int64_t)(nanoseconds/pow(2.0,32)));
	}
	Time_t()
	{
		seconds = 0;
		nanoseconds = 0;
	}
}Time_t;



#define TIME_ZERO(t){t.seconds=0;t.nanoseconds=0;}
#define TIME_INVALID(t){t.seconds=-1;t.nanoseconds=0xffffffff;}
#define TIME_INFINITE(t){t.seconds=0x7fffffff;t.nanoseconds=0xffffffff;}

typedef Time_t Duration_t;
}
}

#endif /* TIME_T_H_ */

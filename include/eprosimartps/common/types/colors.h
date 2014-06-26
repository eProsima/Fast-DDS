/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file colors.h
 */

#ifndef COLORS_H_
#define COLORS_H_


#if defined(_WIN32)
	#define RTPS_DEF ""
	#define RTPS_RED ""
	#define RTPS_B_RED ""
	#define RTPS_GREEN ""
	#define RTPS_B_GREEN ""
	#define RTPS_YELLOW ""
	#define RTPS_B_YELLOW ""
	#define RTPS_BLUE ""
	#define RTPS_B_BLUE ""
	#define RTPS_MAGENTA ""
	#define RTPS_B_MAGENTA ""
	#define RTPS_CYAN ""
	#define RTPS_B_CYAN ""
	#define RTPS_WHITE ""
	#define RTPS_B_WHITE ""
	#define RTPS_BRIGHT ""
#else
	#define RTPS_DEF "\033[m"
	#define RTPS_RED "\033[31m"
	#define RTPS_B_RED "\033[31;1m"
	#define RTPS_GREEN "\033[32m"
	#define RTPS_B_GREEN "\033[32;1m"
	#define RTPS_YELLOW "\033[33m"
	#define RTPS_B_YELLOW "\033[33;1m"
	#define RTPS_BLUE "\033[34m"
	#define RTPS_B_BLUE "\033[34;1m"
	#define RTPS_MAGENTA "\033[35m"
	#define RTPS_B_MAGENTA "\033[35;1m"
	#define RTPS_CYAN "\033[36m"
	#define RTPS_B_CYAN "\033[36;1m"
	#define RTPS_WHITE "\033[37m"
	#define RTPS_B_WHITE "\033[37;1m"
	#define RTPS_BRIGHT "\033[1m"
#endif


#endif /* COLORS_H_ */

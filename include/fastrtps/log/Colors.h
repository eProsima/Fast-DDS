/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Colors is licensed to you under the terms described in the
 * CSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Colors.h
 */

#ifndef C_COLORS_H_
#define C_COLORS_H_


#if defined(_WIN32)
	#define C_DEF ""
	#define C_RED ""
	#define C_B_RED ""
	#define C_GREEN ""
	#define C_B_GREEN ""
	#define C_YELLOW ""
	#define C_B_YELLOW ""
	#define C_BLUE ""
	#define C_B_BLUE ""
	#define C_MAGENTA ""
	#define C_B_MAGENTA ""
	#define C_CYAN ""
	#define C_B_CYAN ""
	#define C_WHITE ""
	#define C_B_WHITE ""
	#define C_BRIGHT ""
#else
	#define C_DEF "\033[m"
	#define C_RED "\033[31m"
	#define C_B_RED "\033[31;1m"
	#define C_GREEN "\033[32m"
	#define C_B_GREEN "\033[32;1m"
	#define C_YELLOW "\033[33m"
	#define C_B_YELLOW "\033[33;1m"
	#define C_BLUE "\033[34m"
	#define C_B_BLUE "\033[34;1m"
	#define C_MAGENTA "\033[35m"
	#define C_B_MAGENTA "\033[35;1m"
	#define C_CYAN "\033[36m"
	#define C_B_CYAN "\033[36;1m"
	#define C_WHITE "\033[37m"
	#define C_B_WHITE "\033[37;1m"
	#define C_BRIGHT "\033[1m"
#endif


#endif /* COLORS_H_ */

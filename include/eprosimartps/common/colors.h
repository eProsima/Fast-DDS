/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * colors.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#ifndef COLORS_H_
#define COLORS_H_


#if defined(_WIN32)
	#define DEF ""
	#define RED ""
	#define B_RED ""
	#define GREEN ""
	#define B_GREEN ""
	#define YELLOW ""
	#define B_YELLOW ""
	#define BLUE ""
	#define B_BLUE ""
	#define MAGENTA ""
	#define B_MAGENTA ""
	#define CYAN ""
	#define WHITE ""
	#define B_WHITE ""
	#define BRIGHT ""
#else
	#define DEF "\033[m"
	#define RED "\033[31m"
	#define B_RED "\033[31;1m"
	#define GREEN "\033[32m"
	#define B_GREEN "\033[32;1m"
	#define YELLOW "\033[33m"
	#define B_YELLOW "\033[33;1m"
	#define BLUE "\033[34m"
	#define B_BLUE "\033[34;1m"
	#define MAGENTA "\033[35m"
	#define B_MAGENTA "\033[35;1m"
	#define CYAN "\033[36m"
	#define WHITE "\033[37m"
	#define B_WHITE "\033[37;1m"
	#define BRIGHT "\033[1m"
#endif


#endif /* COLORS_H_ */

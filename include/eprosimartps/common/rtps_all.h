/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * rtps_all.h
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#ifndef RTPS_ALL_H_
#define RTPS_ALL_H_

#include <stdint.h>
#include <iomanip>

/*!
 * @brief This enumeration represents endianness types.
 */
typedef enum
{
	//! @brief Big endianness.
	BIGEND = 0x1,
	//! @brief Little endianness.
	LITTLEEND = 0x0
} Endianness_t;

// //! @brief Default endiness in the system.
//static Endianness_t DEFAULT_ENDIAN;

using namespace std;

#include "colors.h"
#include "rtps_common.h"
#include "rtps_messages.h"
#include "rtps_error_codes.h"
#include "../CDRMessage.h"



#endif /* RTPS_ALL_H_ */

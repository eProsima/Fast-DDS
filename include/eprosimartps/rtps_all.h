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

#include <cstdlib>
#include <cstring>
#include <math.h>
#include <vector>
#include <iostream>
 #include <bitset>
 #include <string>
#include <sstream>
#include <stdint.h>
#include <iomanip>

#ifndef RTPS_ALL_H_
#define RTPS_ALL_H_



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

using namespace std;

#include "common/colors.h"
#include "common/rtps_common.h"
#include "common/rtps_messages.h"
#include "common/rtps_error_codes.h"
#include "common/CacheChange.h"

//#include "CDRMessage.h"
//#include "Endpoint.h"
//
//#include "MessageReceiver.h"
//#include "ReaderLocator.h"
//#include "RTPSWriter.h"
//#include "HistoryCache.h"
//#include "StatelessWriter.h"





#endif /* RTPS_ALL_H_ */

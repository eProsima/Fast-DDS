/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * error_codes.h
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_

namespace eprosima{
namespace rtps{

#define ERR_CDRMESSAGE_BUFFER_FULL 101

#define ERR_MESSAGE_TOO_SHORT 201
#define ERR_MESSAGE_NOT_RTPS 202
#define ERR_MESSAGE_VERSION_UNSUPPORTED 203
#define ERR_SUBMSGHEADER_TOO_SHORT 204
#define ERR_SUBMSG_LENGTH_INVALID 205

}
}




#endif /* ERROR_CODES_H_ */

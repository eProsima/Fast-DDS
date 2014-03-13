/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file rtps_error_codes.h
 *	 Error codes.
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_

namespace eprosima{
namespace rtps{

/**
 * Error codes used.
 * @ingroup UTILITIESMODULE
 */
typedef enum ErrorCodes_t{
 ERR_CDRMESSAGE_BUFFER_FULL = 101,
 ERR_MESSAGE_TOO_SHORT = 201,
 ERR_MESSAGE_NOT_RTPS = 202,
 ERR_MESSAGE_VERSION_UNSUPPORTED = 203,
 ERR_SUBMSGHEADER_TOO_SHORT = 204,
 ERR_SUBMSG_LENGTH_INVALID = 205,
 ERR_MESSAGE_INCORRECT_HEADER =  206,
 ERR_PARTICIPANT_INCORRECT_ENDPOINT_TYPE= 301
}ErrorCodes_t;


/**
 * @name Error strings
 * @ingroup UTILITIESMODULE
 * @{
 */
#define ERR_CDRMESSAGE_BUFFER_FULL_STR "CDR message buffer full. Not enough space."
#define ERR_MESSAGE_TOO_SHORT_STR "Message too short to be RTPS."
#define ERR_MESSAGE_VERSION_UNSUPPORTED_STR "Message version not supported."
#define ERR_SUBMSGHEADER_TOO_SHORT_STR "Submessage Header too short."
#define ERR_SUBMSG_LENGTH_INVALID_STR "Submessage length invalid."
#define ERR_MESSAGE_INCORRECT_HEADER_STR "Incorrect Message Header."
#define ERR_PARTICIPANT_INCORRECT_ENDPOINT_TYPE_STR "Incorrect Participant Endpoint Type."
#define ERR_MESSAGE_NOT_RTPS_STR "Message NOT RTPS."
///@}

}
}




#endif /* ERROR_CODES_H_ */

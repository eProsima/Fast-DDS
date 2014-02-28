/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * rpts_elements.h
 *
 *  Created on: Feb 28, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RPTS_ELEMENTS_H_
#define RPTS_ELEMENTS_H_

#define RTPSMESSAGE_MAX_SIZE 2048  //max size of ftps message in bytes
#define RTPSMESSAGE_HEADER_SIZE 20  //header size in bytes
#define RTPSMESSAGE_SUBMESSAGEHEADER_SIZE 4

#define RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG 16 //may change in future versions

#define DEFAULT_HISTORY_SIZE 10


#define BIT0 0x1
#define BIT1 0x2
#define BIT2 0x4
#define BIT3 0x8
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define BIT(i) ((i==0) ? BIT0 : (i==1) ? BIT1 :(i==2)?BIT2:(i==3)?BIT3:(i==4)?BIT4:(i==5)?BIT5:(i==6)?BIT6:(i==7)?BIT7:0x0)






typedef unsigned char octet;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef octet SubmessageFlag;

#include "rtps_elem_guid.h"
#include "rtps_elem_seqnum.h"
#include "rtps_elem_locator.h"
#include "rtps_elem_parameter.h"



#endif /* RPTS_ELEMENTS_H_ */

/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file rtps_elem_locator.h
 *	Locator_t definition.
 *  Created on: Feb 28, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RTPS_ELEM_LOCATOR_H_
#define RTPS_ELEM_LOCATOR_H_

namespace eprosima{
namespace rtps{



#define LOCATOR_INVALID(loc)  {loc.kind=LOCATOR_KIND_INVALID;loc.port= LOCATOR_PORT_INVALID;LOCATOR_ADDRESS_INVALID(loc.address);}
#define LOCATOR_KIND_INVALID -1

#define LOCATOR_ADDRESS_INVALID(a) {for(uint8_t i=0;i<16;i++) a[i]=0x0;}
#define LOCATOR_PORT_INVALID 0
#define LOCATOR_KIND_RESERVED 0
#define LOCATOR_KIND_UDPv4 1
#define LOCATOR_KIND_UDPv6 2



//!@brief Structure Locator_t, uniquely identifies a address+port combination.
typedef struct Locator_t{
	int32_t kind;
	uint32_t port;
	octet address[16];
	Locator_t(){
		kind = 1;
		port = 0;
		LOCATOR_ADDRESS_INVALID(address);
	}
	bool operator==(Locator_t loc){
		if(kind!=loc.kind)
			return false;
		if(port !=loc.port)
			return false;
		for(uint8_t i =0;i<16;i++){
			if(address[i] !=loc.address[i])
				return false;
		}
		return true;
	}
	bool set_IP4_address(octet o1,octet o2,octet o3,octet o4){
		LOCATOR_ADDRESS_INVALID(address);
		address[12] = o1;
		address[13] = o2;
		address[14] = o3;
		address[15] = o4;
		return true;
	}
	std::string to_IP4_string(){
		std::stringstream ss;
		ss << (int)address[12] << "." << (int)address[13] << "." << (int)address[14]<< "." << (int)address[15];
		return ss.str();
	}

} Locator_t;



}
}




#endif /* RTPS_ELEM_LOCATOR_H_ */

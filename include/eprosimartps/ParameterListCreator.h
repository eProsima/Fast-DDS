/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of eProsimaRTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ParameterListCreator.h
 *
 *  Created on: Mar 7, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "rtps_all.h"
#include "ParameterList_t.h"


#ifndef PARAMETERLISTCREATOR_H_
#define PARAMETERLISTCREATOR_H_

using namespace eprosima::rtps;

namespace eprosima {
namespace dds {

class RTPS_DllAPI ParameterListCreator {
public:
	ParameterListCreator();
	virtual ~ParameterListCreator();
	static bool updateMsg(ParameterList_t* plist);
	static bool readParamListfromCDRmessage(ParameterList_t* plist,CDRMessage_t* msg,uint32_t* size);
	static bool addParameterLocator(ParameterList_t* plist,ParameterId_t pid,rtps::Locator_t loc);
	static bool addParameterString(ParameterList_t* plist,ParameterId_t pid,std::string in_str);
	static bool addParameterPort(ParameterList_t* plist,ParameterId_t pid,uint32_t port);
};





} /* namespace dds */
} /* namespace eprosima */

#endif /* PARAMETERLISTCREATOR_H_ */

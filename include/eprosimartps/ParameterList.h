/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterList.h
 *
 *  Created on: Mar 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARAMETERLIST_H_
#define PARAMETERLIST_H_

#include "rtps_all.h"
#include <vector>

#include "eprosimartps/ParameterTypes.h"

namespace eprosima {
namespace dds {




class ParameterList_t {
public:
	ParameterList_t():has_changed_Qos(true),has_changed_inlineQos(true){};
	virtual ~ParameterList_t(){};
	std::vector<Parameter_t*> QosParams;
	std::vector<Parameter_t*> inlineQosParams;
	CDRMessage_t QosMsg;
	CDRMessage_t inlineQosMsg;
	bool has_changed_Qos;
	bool has_changed_inlineQos;
	void get_QosMsg(CDRMessage_t** msg_out);
	void get_inlineQosMsg(CDRMessage_t** msg_out);
};


class ParameterList{
public:
	static bool updateQosMsg(ParameterList_t* plist);
	static bool updateInlineQosMsg(ParameterList_t* plist);
	static bool updateMsg(std::vector<Parameter_t*>* vec,CDRMessage_t* msg);
	static bool addParameterString(ParameterList_t* plist,ParameterId_t pid,std::string& in_str);
	static bool addParameterLocator(ParameterList_t* plist,ParameterId_t pid,Locator_t* loc);
	static bool addParameterPort(ParameterList_t* plist,ParameterId_t pid,uint32_t port);

	static bool readParameterList(CDRMessage_t* msg,ParameterList_t* plist,uint32_t* size);
	static bool readInlineQos(CDRMessage_t*msg,ParameterList* plist,InstanceHandle_t* iHandle,octet*status,uint32_t*size);
};


} /* namespace dds */
} /* namespace eprosima */

#endif /* PARAMETERLIST_H_ */

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

#include "eprosimartps/rtps_all.h"
#include <vector>

#include "eprosimartps/dds/ParameterTypes.h"

namespace eprosima {
namespace dds {




class ParameterList_t {
public:
	ParameterList_t():	QosMsg(RTPSMESSAGE_MAX_SIZE),
	inlineQosMsg(RTPSMESSAGE_MAX_SIZE),
	has_changed_Qos(true),has_changed_inlineQos(true){};
	virtual ~ParameterList_t()
	{
		std::vector<Parameter_t*>::iterator it;
		for(it=QosParams.begin();it!=QosParams.end();++it)
		{
			delete(*it);
		}
		for(it=inlineQosParams.begin();it!=inlineQosParams.end();++it)
		{
			delete(*it);
		}
	};
	std::vector<Parameter_t*> QosParams;
	std::vector<Parameter_t*> inlineQosParams;
	CDRMessage_t QosMsg;
	CDRMessage_t inlineQosMsg;
	bool has_changed_Qos;
	bool has_changed_inlineQos;
	void get_QosMsg(CDRMessage_t** msg_out,Endianness_t endian);
	void get_inlineQosMsg(CDRMessage_t** msg_out,Endianness_t endian);
};


namespace ParameterList
{

	inline bool updateQosMsg(ParameterList_t* plist,Endianness_t endian);
	inline bool updateInlineQosMsg(ParameterList_t* plist,Endianness_t endian);
	inline bool updateMsg(std::vector<Parameter_t*>* vec,CDRMessage_t* msg,Endianness_t endian);
	inline bool addParameterString(ParameterList_t* plist,ParameterId_t pid,std::string& in_str);
	inline bool addParameterLocator(ParameterList_t* plist,ParameterId_t pid,Locator_t* loc);
	inline bool addParameterPort(ParameterList_t* plist,ParameterId_t pid,uint32_t port);

	inline bool readParameterList(CDRMessage_t* msg,ParameterList_t* plist,uint32_t* size,ChangeKind_t* kind,InstanceHandle_t* iHandle);

	inline Endianness_t get_Qos_endian(ParameterList_t* plist){return plist->QosMsg.msg_endian;};
	inline Endianness_t get_inlineQos_endian(ParameterList_t* plist){return plist->inlineQosMsg.msg_endian;};

}



} /* namespace dds */
} /* namespace eprosima */

#include "ParameterList.hpp"

#endif /* PARAMETERLIST_H_ */

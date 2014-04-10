/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParamListt.h
 *
 *  Created on: Apr 9, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARAMLISTT_H_
#define PARAMLISTT_H_
#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/ParameterTypes.h"


namespace eprosima {
namespace dds {

class ParameterList_t {
public:
	ParameterList_t():m_cdrmsg(RTPSMESSAGE_MAX_SIZE),m_hasChanged(true){};
	virtual ~ParameterList_t()
	{

	}
	void deleteParams()
	{
		for(std::vector<Parameter_t*>::iterator it = m_parameters.begin();
				it!=m_parameters.end();++it)
			delete(*it);
	}
	void resetList(){
		m_parameters.clear();
		CDRMessage::initCDRMsg(&m_cdrmsg);
		m_hasChanged = true;
	}
	std::vector<Parameter_t*> m_parameters;
	CDRMessage_t m_cdrmsg;
	bool m_hasChanged;
};

class ParameterList
{
public:
	static bool updateCDRMsg(ParameterList_t* plist,Endianness_t endian);
	static uint32_t readParameterListfromCDRMsg(CDRMessage_t*msg,ParameterList_t*plist,InstanceHandle_t* handle,ChangeKind_t* chkind);

};



} /* namespace dds */
} /* namespace eprosima */

#endif /* PARAMLISTT_H_ */

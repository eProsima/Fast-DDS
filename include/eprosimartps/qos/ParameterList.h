/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterList.h
 *	ParameterList
 *  Created on: Apr 9, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARAMLISTT_H_
#define PARAMLISTT_H_

#include "eprosimartps/qos/ParameterTypes.h"
#include "eprosimartps/common/RTPS_messages.h"
#include "eprosimartps/common/types/CDRMessage_t.h"
#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/common/CacheChange.h"
namespace eprosima {



namespace dds {

/**
 * ParameterList_t class, used to store multiple parameters as a vector of pointers to the base class.
 */
class ParameterList_t {
public:
	ParameterList_t():m_cdrmsg(RTPSMESSAGE_MAX_SIZE),m_hasChanged(true){};
	virtual ~ParameterList_t()
	{

	}
	/**
	 * Delete all parameters in the list.
	 */
	void deleteParams()
	{
		for(std::vector<Parameter_t*>::iterator it = m_parameters.begin();
				it!=m_parameters.end();++it)
			delete(*it);
	}
	/**
	 * Reset the list of parameters (without deleting them, since they can be in use in another list).
	 */
	void resetList(){
		m_parameters.clear();
		CDRMessage::initCDRMsg(&m_cdrmsg);
		m_hasChanged = true;
	}
	//! Vector of the pointers to the parameters.
	std::vector<Parameter_t*> m_parameters;
	//! Associated CDRMessage_t message.
	CDRMessage_t m_cdrmsg;
	//! Bool variable to indicate wheter a new parameter has been added.
	bool m_hasChanged;
};

/**
 * ParameterList class has static methods to update or read a ParameterList_t
 */
class ParameterList
{
public:
	/**
	 * Update the CDRMessage of a parameterList.
	 * @param plist Pointer to the parameterList.
	 * @param endian Endianness that the resulting CDRMEssage_t should have.
	 * @return True if correct.
	 */
	static bool updateCDRMsg(ParameterList_t* plist,Endianness_t endian);
	/**
	 * Read a parameterList from a CDRMessage
	 * @param[in] msg Pointer to the message (the pos should be correct, otherwise the behaviour is unexpected).
	 * @param[out] plist Pointer to the parameter list.
	 * @param[out] handle Pointer to the handle.
	 * @param[out] chkind Pointer to the change Kind.
	 * @return Number of bytes of the parameter list.
	 */
	static uint32_t readParameterListfromCDRMsg(CDRMessage_t*msg,ParameterList_t*plist,InstanceHandle_t* handle,ChangeKind_t* chkind);

};



} /* namespace dds */
} /* namespace eprosima */

#endif /* PARAMLISTT_H_ */

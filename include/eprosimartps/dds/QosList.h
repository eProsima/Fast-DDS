/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file QosList.h
 *
 *  Created on: Apr 9, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef QOSLIST_H_
#define QOSLIST_H_
#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/ParameterList.h"

namespace eprosima {
namespace dds {

class QosList_t {
public:
	QosList_t();
	virtual ~QosList_t();
	ParameterList_t allQos;
	ParameterList_t inlineQos;
};

class QosList
{
static bool addQos(QosList_t* qos,ParameterId_t pid ,std::string& string_in);
static bool addQos(QosList_t* qos,ParameterId_t pid ,Locator_t& loc);
static bool addQos(QosList_t* qos,ParameterId_t pid ,uint32_t uintlong);
static bool addQos(QosList_t* qos,ParameterId_t pid ,GUID_t& guid);
static bool addQos(QosList_t* qos,ParameterId_t pid ,ProtocolVersion_t& protocol);
static bool addQos(QosList_t* qos,ParameterId_t pid ,VendorId_t& vendor);
static bool addQos(QosList_t* qos,ParameterId_t pid ,octet o1,octet o2,octet o3,octet o4);
//static bool addQos(QosList_t* qos,ParameterId_t pid ,Count_t& count);
static bool addQos(QosList_t* qos,ParameterId_t pid ,EntityId_t& entity);
static bool addQos(QosList_t* qos,ParameterId_t pid ,Time_t& entity);
//static bool addQos(QosList_t* qos,ParameterId_t pid ,BuiltinEndpointSet_t& set);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* QOSLIST_H_ */

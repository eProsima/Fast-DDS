/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file QosList.h
 */

#ifndef QOSLIST_H_
#define QOSLIST_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "fastrtps/rtps/common/all_common.h"
#include "fastrtps/qos/ParameterList.h"


namespace eprosima {
namespace fastrtps {

/**
 * QosList_t class contains two ParameterList_t for the Qos.
 * @ingroup PARAMETER_MODULE
 */
class QosList_t {
public:
	QosList_t();
	virtual ~QosList_t();
	//! All the Qos as a parameter List.
	ParameterList_t allQos;
	//! Only the Qos that can be send as inline.
	ParameterList_t inlineQos;
};

/**
 * QosList class, that contains static methods to add Qos to a QosList_t structure.
 * @ingroup PARAMETER_MODULE
 */
class QosList
{
public:
	/**
	 * @name AddQos methods.
	 * @param qos Pointer to the QOsList_t list.
	 * @param pid PID of the parameter to add to the QosList_t.
	 * @param param Parameter to add.
	 * @return True if correct.
	 */
	///@{
	static bool addQos(QosList_t* qos,ParameterId_t pid ,std::string& string_in);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,Locator_t& loc);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,uint32_t uintlong);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,const GUID_t& guid);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,ProtocolVersion_t& protocol);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,VendorId_t& vendor);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,octet o1,octet o2,octet o3,octet o4);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,const EntityId_t& entity);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,Time_t& entity);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,bool in_bool);
	static bool addQos(QosList_t* qos,ParameterId_t pid ,std::string& str1,std::string& str2);
	static bool addQos(QosList_t* qos, ParameterId_t pid,	std::vector<octet>& ocVec);
	static bool addQos(QosList_t* qos,ParameterId_t pid, const ParameterPropertyList_t& list);
	//static bool addQos(QosList_t* qos,ParameterId_t pid,BuiltinEndpointSet_t endpointset);
	///@}
};

} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* QOSLIST_H_ */

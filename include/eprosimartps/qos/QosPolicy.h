/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file QosPolicy.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef QOSPOLICY_H_
#define QOSPOLICY_H_

using namespace eprosima::rtps;

namespace eprosima {
namespace dds {

class QosPolicy {
public:
	QosPolicy():pid(PID_SENTINEL),length(0){};
	virtual ~QosPolicy(){};
	ParameterId_t pid;
	uint16_t length;
	QosPolicy(ParameterId_t pidin,uint16_t lengthin):pid(pidin),length(lengthin){};
	virtual bool addToCDRMessage(CDRMessage_t* msg)=0;
	virtual bool readFromCDRMessage(CDRMessage_t*msg,uint16_t length)=0;
	virtual bool addToQosList(QosList_t*)=0;
};

class LocatorQosPolicy:public QosPolicy{

};



} /* namespace dds */
} /* namespace eprosima */

#endif /* QOSPOLICY_H_ */

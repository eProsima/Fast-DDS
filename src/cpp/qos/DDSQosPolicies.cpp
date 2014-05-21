/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterTypes.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/qos/DDSQosPolicies.h"

#include "eprosimartps/CDRMessage.h"

namespace eprosima {
namespace dds {



bool DurabilityQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctet(msg,kind);msg->pos+=3;msg->length+=3;
	return valid;
}

bool DeadlineQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addInt32(msg,period.seconds);
	valid &= CDRMessage::addUInt32(msg,period.nanoseconds);
	return valid;
}


bool LatencyBudgetQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addInt32(msg,duration.seconds);
	valid &= CDRMessage::addUInt32(msg,duration.nanoseconds);
	return valid;
}

bool LivelinessQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctet(msg,kind);msg->pos+=3;msg->length+=3;
	valid &= CDRMessage::addInt32(msg,lease_duration.seconds);
	valid &= CDRMessage::addUInt32(msg,lease_duration.nanoseconds);
	return valid;
}

bool OwnershipQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctet(msg,kind);msg->pos+=3;msg->length+=3;
	return valid;
}

bool ReliabilityQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctet(msg,kind);msg->pos+=3;msg->length+=3;
	valid &= CDRMessage::addInt32(msg,max_blocking_time.seconds);
	valid &= CDRMessage::addUInt32(msg,max_blocking_time.nanoseconds);
	return valid;
}

bool DestinationOrderQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctet(msg,kind);msg->pos+=3;msg->length+=3;
	return valid;
}

bool TimeBasedFilterQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addInt32(msg,minimum_separation.seconds);
	valid &= CDRMessage::addUInt32(msg,minimum_separation.nanoseconds);
	return valid;
}

bool PresentationQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctet(msg,access_scope);
	valid &= CDRMessage::addOctet(msg,(octet)coherent_access);
	valid &= CDRMessage::addOctet(msg,(octet)ordered_access);
	msg->pos++;msg->length++;
	return valid;
}

bool PartitionQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctetVector(msg,&name);
	return valid;
}

bool TopicDataQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctetVector(msg,&value);
	return valid;
}

bool GroupDataQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctetVector(msg,&value);
	return valid;
}

bool HistoryQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addOctet(msg,kind);msg->pos+=3;msg->length+=3;
	valid &= CDRMessage::addInt32(msg,depth);
	return valid;
}

bool DurabilityServiceQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addInt32(msg,service_cleanup_delay.seconds);
	valid &= CDRMessage::addUInt32(msg,service_cleanup_delay.nanoseconds);
	valid &= CDRMessage::addOctet(msg,history_kind);msg->pos+=3;msg->length+=3;
	valid &= CDRMessage::addUInt32(msg,history_depth);
	valid &= CDRMessage::addUInt32(msg,max_samples);
	valid &= CDRMessage::addUInt32(msg,max_instances);
	valid &= CDRMessage::addUInt32(msg,max_samples_per_instance);
	return valid;
}

bool LifespanQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addInt32(msg,duration.seconds);
	valid &= CDRMessage::addUInt32(msg,duration.nanoseconds);
	return valid;
}

bool OwnershipStrengthQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addUInt32(msg,value);
	return valid;
}

bool ResourceLimitsQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);

	valid &= CDRMessage::addUInt32(msg,max_samples);
	valid &= CDRMessage::addUInt32(msg,max_instances);
	valid &= CDRMessage::addUInt32(msg,max_samples_per_instance);
	return valid;
}

bool TransportPriorityQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
	bool valid = CDRMessage::addUInt16(msg, this->Pid);
	valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
	valid &= CDRMessage::addUInt32(msg,value);
	return valid;
}




} /* namespace dds */
} /* namespace eprosima */



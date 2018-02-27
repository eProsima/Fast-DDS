// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file ParameterTypes.cpp
 *
 */

#include <fastrtps/qos/QosPolicies.h>

#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

bool DurabilityQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    return valid;
}

bool DeadlineQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,period.seconds);
    valid &= CDRMessage::addUInt32(msg,period.fraction);
    return valid;
}


bool LatencyBudgetQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,duration.seconds);
    valid &= CDRMessage::addUInt32(msg,duration.fraction);
    return valid;
}

bool LivelinessQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addInt32(msg,lease_duration.seconds);
    valid &= CDRMessage::addUInt32(msg,lease_duration.fraction);
    return valid;
}

bool OwnershipQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    return valid;
}

bool ReliabilityQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addInt32(msg,max_blocking_time.seconds);
    valid &= CDRMessage::addUInt32(msg,max_blocking_time.fraction);
    return valid;
}

bool DestinationOrderQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    return valid;
}

bool TimeBasedFilterQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,minimum_separation.seconds);
    valid &= CDRMessage::addUInt32(msg,minimum_separation.fraction);
    return valid;
}

bool PresentationQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PRESENTATION_LENGTH);//this->length);
    valid &= CDRMessage::addOctet(msg,access_scope);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);

    valid &= CDRMessage::addOctet(msg,(octet)coherent_access);
    valid &= CDRMessage::addOctet(msg,(octet)ordered_access);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);

    return valid;
}

bool PartitionQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    //Obtain Length:
    this->length = 0;
    this->length += 4;
    uint16_t rest;
    for(std::vector<std::string>::iterator it = names.begin();it!=names.end();++it)
    {
        this->length +=4;
        this->length += (uint16_t)it->size()+1;
        rest = ((uint16_t)it->size() +1 ) % 4;
        this->length += rest != 0 ? 4 - rest : 0;
    }
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg,(uint32_t)this->names.size());
    for(std::vector<std::string>::iterator it = names.begin();it!=names.end();++it)
        valid &= CDRMessage::addString(msg,*it);
    //valid &= CDRMessage::addOctetVector(msg,&name);
    return valid;
}

bool UserDataQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    uint32_t align = (4 - (msg->pos + 6 + dataVec.size())  % 4) & 3; //align
    this->length = (uint16_t)(4 + this->dataVec.size() + align);
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg, (uint32_t)this->dataVec.size());
    valid &= CDRMessage::addData(msg,this->dataVec.data(),(uint32_t)this->dataVec.size());
    for(uint32_t count = 0; count < align; ++count)
    {
        valid &= CDRMessage::addOctet(msg, 0);
    }
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
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addInt32(msg,depth);
    return valid;
}

bool DurabilityServiceQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,service_cleanup_delay.seconds);
    valid &= CDRMessage::addUInt32(msg,service_cleanup_delay.fraction);
    valid &= CDRMessage::addOctet(msg,history_kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addInt32(msg,history_depth);
    valid &= CDRMessage::addInt32(msg,max_samples);
    valid &= CDRMessage::addInt32(msg,max_instances);
    valid &= CDRMessage::addInt32(msg,max_samples_per_instance);
    return valid;
}

bool LifespanQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,duration.seconds);
    valid &= CDRMessage::addUInt32(msg,duration.fraction);
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

    valid &= CDRMessage::addInt32(msg,max_samples);
    valid &= CDRMessage::addInt32(msg,max_instances);
    valid &= CDRMessage::addInt32(msg,max_samples_per_instance);
    return valid;
}

bool TransportPriorityQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addUInt32(msg,value);
    return valid;
}



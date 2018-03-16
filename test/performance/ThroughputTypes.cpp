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
 * @file ThroughputTypes.cpp
 *
 */

#include "ThroughputTypes.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

//Funciones de serializacion y deserializacion para el ejemplo
bool ThroughputDataType::serialize(void*data,SerializedPayload_t* payload)
{
    ThroughputType* lt = (ThroughputType*)data;
    *(uint32_t*)payload->data = lt->seqnum;
    *(uint32_t*)(payload->data+4) = (uint32_t)lt->data.size();
    //	std::copy(lt->data.begin(),lt->data.end(),payload->data+8);
    memcpy(payload->data + 8, lt->data.data(), lt->data.size());
    payload->length = 8+(uint16_t)lt->data.size();
    return true;
}

bool ThroughputDataType::deserialize(SerializedPayload_t* payload,void * data)
{
    if(payload->length > 0)
    {
        ThroughputType* lt = (ThroughputType*)data;
        lt->seqnum = *(uint32_t*)payload->data;
        uint32_t siz = *(uint32_t*)(payload->data+4);
        //std::copy(payload->data+8,payload->data+8+siz,lt->data.begin());
        std::copy(payload->data + 8, payload->data + 8 + siz, lt->data.begin());
        //		lt->data.clear();
        //		lt->data.insert(lt->data.end(),payload->data+8,payload->data+8+siz);
    }
    return true;
}

std::function<uint32_t()> ThroughputDataType::getSerializedSizeProvider(void* data)
{
    return [data]() -> uint32_t
    {
        ThroughputType *tdata = static_cast<ThroughputType*>(data);
        uint32_t size = 0;

        size = (uint32_t)(sizeof(uint32_t) + sizeof(uint32_t) + tdata->data.size());

        return size;
    };
}

void* ThroughputDataType::createData()
{
    return (void*)new ThroughputType((uint16_t)this->m_typeSize);
}
void ThroughputDataType::deleteData(void* data)
{
    delete((ThroughputType*)data);
}



bool ThroughputCommandDataType::serialize(void*data,SerializedPayload_t* p)
{
    ThroughputCommandType* t = (ThroughputCommandType*)data;
    p->length = 0;
    *(uint32_t*)p->data = (uint32_t)t->m_command;p->length+=4;
    *(uint32_t*)&(p->data[p->length]) = (uint32_t)t->m_size;p->length+=4;
    *(uint32_t*)&(p->data[p->length]) = (uint32_t)t->m_demand;p->length+=4;
    *(uint32_t*)&(p->data[p->length]) = (uint32_t)t->m_lostsamples;p->length+=4;
    *(uint64_t*)&(p->data[p->length]) = (uint32_t)t->m_lastrecsample;p->length+=8;
    *(uint64_t*)&(p->data[p->length]) = (uint32_t)t->m_totaltime;p->length+=8;
    return true;
}
bool ThroughputCommandDataType::deserialize(SerializedPayload_t* p,void * data)
{
    ThroughputCommandType* t = (ThroughputCommandType*)data;
    p->pos = 0;
    t->m_command = (e_Command)*(uint32_t*)p->data;p->pos+=4;
    t->m_size = *(uint32_t*)&p->data[p->pos];p->pos+=4;
    t->m_demand = *(uint32_t*)&p->data[p->pos];p->pos+=4;
    t->m_lostsamples = *(uint32_t*)&p->data[p->pos];p->pos+=4;
    t->m_lastrecsample = *(uint64_t*)&p->data[p->pos];p->pos+=8;
    t->m_totaltime = *(uint64_t*)&p->data[p->pos];p->pos+=8;
    return true;
}

std::function<uint32_t()> ThroughputCommandDataType::getSerializedSizeProvider(void*)
{
    return []() -> uint32_t
    {
        uint32_t size = 0;

        size = (uint32_t)(sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)  + sizeof(uint32_t) +
                sizeof(uint64_t) + sizeof(uint64_t));

        return size;
    };
}

void* ThroughputCommandDataType::createData()
{
    return (void*)new ThroughputCommandType();
}
void ThroughputCommandDataType::deleteData(void* data)
{
    delete((ThroughputCommandType*)data);
}

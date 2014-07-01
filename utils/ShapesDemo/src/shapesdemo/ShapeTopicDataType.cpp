/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimashapesdemo/shapesdemo/ShapeType.h"
#include "eprosimashapesdemo/shapesdemo/ShapeTopicDataType.h"
#include "eprosimashapesdemo/utils/md5.h"

ShapeTopicDataType::ShapeTopicDataType()
{
    this->m_isGetKeyDefined = true;
    this->m_topicDataTypeName = "ShapeType";
    this->m_typeSize = 4+4+4+128;
}

ShapeTopicDataType::~ShapeTopicDataType()
{

}

bool ShapeTopicDataType::serialize(void *data, SerializedPayload_t *payload)
{
    ShapeType* sh = (ShapeType*)data;
    uint32_t strsize = sh->getColorStr().size();
    std::string auxstr = sh->getColorStr();

    uint32_t* auxptr;
    auxptr = (uint32_t*)payload->data;
    *auxptr = strsize; //COPY STR SIZE:
    for(uint32_t i =0;i<strsize;++i)
        payload->data[4+i] = auxstr.at(i);
    int rest = strsize%4;
    if(rest > 0)
    {
        for(int i = 0;i<rest;++i)
           payload->data[4+strsize+i] = 0;
    }
    auxptr = (uint32_t*)&payload->data[4+strsize+rest];
    *auxptr = sh->m_x;
    auxptr = (uint32_t*)&payload->data[4+strsize+rest+4];
    *auxptr = sh->m_y;
    auxptr = (uint32_t*)&payload->data[4+strsize+rest+4+4];
    *auxptr = sh->m_size;

    payload->length = 4+strsize+rest+4+4+4;
   // cout << "Serialize length: " << payload->length<<endl;
    return true;
 }

bool ShapeTopicDataType::deserialize(SerializedPayload_t *payload, void *data)
{
   // cout << "Deserializing"<<endl;
    ShapeType* sh = (ShapeType*)data;
    uint32_t strsize = *(uint32_t*)payload->data;
    std::string auxstr;
    auxstr.resize(strsize);
    for(uint32_t i=0;i<strsize;++i)
        auxstr.at(i) = payload->data[4+i];
    sh->setColor(auxstr);
    int rest = payload->length-4-4-4-4-strsize;
    sh->m_x = *(uint32_t*)&payload->data[4+strsize+rest];
    sh->m_y = *(uint32_t*)&payload->data[4+strsize+rest+4];
    sh->m_size = *(uint32_t*)&payload->data[4+strsize+rest+4+4];
    return false;
}

bool ShapeTopicDataType::getKey(void *data, InstanceHandle_t *ihandle)
{
    ShapeType* sh = (ShapeType*)data;
    std::stringstream ss;
    ss << (unsigned char)sh->getColorStr().size() << sh->getColorStr();
    std::string auxstr = md5(ss.str());
    for(uint8_t i = 0;i<16;++i)
    {
     ihandle->value[i] = auxstr.at(i);
    }
    return true;
}

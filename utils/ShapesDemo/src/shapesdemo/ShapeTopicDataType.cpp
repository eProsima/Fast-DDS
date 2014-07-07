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
    this->m_typeSize = 152;//4+4+4+128+4; 152 is to equal RTI.
}

ShapeTopicDataType::~ShapeTopicDataType()
{

}

bool ShapeTopicDataType::serialize(void *data, SerializedPayload_t *payload)
{
    ShapeType* sh = (ShapeType*)data;
    uint32_t strsize = sh->getColorStr().size();
    std::string auxstr = sh->getColorStr();
    cout << "sTRING:"  << auxstr << endl;
    cout << "Strsize: "<< strsize<<endl;
    uint32_t* auxptr;
    auxptr = (uint32_t*)payload->data;
    *auxptr = strsize+1; //COPY STR SIZE:
    for(uint32_t i =0;i<strsize;++i)
        payload->data[4+i] = auxstr.at(i);
    payload->data[4+strsize] = 0;
    int rest = (strsize+1)%4;
    cout << "Rest mod: "<<rest;
    if(rest > 0)
    {
        rest = 4-rest;
        for(int i = 0;i<rest;++i)
           payload->data[4+strsize+1+i] = 0;
    }
    auxptr = (uint32_t*)&payload->data[4+strsize+1+rest];
    *auxptr = sh->m_x;
    auxptr = (uint32_t*)&payload->data[4+strsize+1+rest+4];
    *auxptr = sh->m_y;
    auxptr = (uint32_t*)&payload->data[4+strsize+1+rest+4+4];
    *auxptr = sh->m_size;

    payload->length = 4+strsize+1+rest+4+4+4;
    payload->encapsulation = EPROSIMA_ENDIAN == BIG_ENDIAN ? CDR_BE:CDR_LE;
   // cout << "Serialize length: " << payload->length<<endl;
    return true;
 }

bool ShapeTopicDataType::deserialize(SerializedPayload_t *payload, void *data)
{
  //  cout << "Deserializing"<<endl;
    ShapeType* sh = (ShapeType*)data;
    uint32_t strsize = *(uint32_t*)payload->data;
    //cout << strsize<<endl;
    sh->setColor(payload->data[4]);
    int rest = strsize%4;
    if(rest>0)
        rest = 4-rest;
   // cout << "rest "<< rest << endl;
    sh->m_x = *(uint32_t*)&payload->data[4+strsize+rest];
    sh->m_y = *(uint32_t*)&payload->data[4+strsize+rest+4];
    sh->m_size = *(uint32_t*)&payload->data[4+strsize+rest+4+4];
    cout <<  sh->m_x << " "<<  sh->m_y << " "<<  sh->m_size << endl;
    return false;
}

bool ShapeTopicDataType::getKey(void *data, InstanceHandle_t *ihandle)
{
    ShapeType* sh = (ShapeType*)data;
    std::string colorstr = sh->getColorStr();
    char cdrcolor[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    cdrcolor[0] = 0x00;
    cdrcolor[1] = 0x0;
    cdrcolor[2] = 0x0;
    cdrcolor[3] = colorstr.size()+1;
    for(uint8_t i =0;i<colorstr.size();++i)
        cdrcolor[4+i] = colorstr.at(i);
    MD5 m =MD5();
    m.update(cdrcolor,colorstr.size()+1+4);
    m.finalize();
    for(uint8_t i = 0;i<16;++i)
    {
     ihandle->value[i] = m.digest[i];
    }



    return true;
}

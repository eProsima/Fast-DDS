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
 * @file EDPStaticXML.cpp
 *
 */

#include <string>
#include <cstdlib>

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPStaticXML.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>

#include <tinyxml2.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {


EDPStaticXML::EDPStaticXML() {
    // TODO Auto-generated constructor stub

}

EDPStaticXML::~EDPStaticXML()
{
    // TODO Auto-generated destructor stub
    for(std::vector<StaticRTPSParticipantInfo*>::iterator pit = m_RTPSParticipants.begin();
            pit!=m_RTPSParticipants.end();++pit)
    {
        for(std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
                rit!=(*pit)->m_readers.end();++rit)
        {
            delete(*rit);
        }
        for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
                wit!=(*pit)->m_writers.end();++wit)
        {
            delete(*wit);
        }

        delete(*pit);
    }
}

bool EDPStaticXML::loadXMLFile(std::string& filename)
{
    logInfo(RTPS_EDP,"File: "<<filename);

        tinyxml2::XMLDocument doc;
        doc.LoadFile(filename.c_str());

        tinyxml2::XMLNode* root = doc.FirstChildElement("staticdiscovery");
        tinyxml2::XMLElement* xml_RTPSParticipant = root->FirstChildElement();

        while(xml_RTPSParticipant != nullptr)
        {
            std::string key(xml_RTPSParticipant->Name());

            if(key == "participant")
            {
                StaticRTPSParticipantInfo* pdata= new StaticRTPSParticipantInfo();
                loadXMLParticipantEndpoint(xml_RTPSParticipant, pdata);
                m_RTPSParticipants.push_back(pdata);
            }

            xml_RTPSParticipant = xml_RTPSParticipant->NextSiblingElement();

        }

    logInfo(RTPS_EDP, "Finished parsing, "<< m_RTPSParticipants.size()<< " participants found.");
    return true;
}

void EDPStaticXML::loadXMLParticipantEndpoint(tinyxml2::XMLElement* xml_endpoint, StaticRTPSParticipantInfo* pdata)
{
        tinyxml2::XMLNode* xml_RTPSParticipant_child = xml_endpoint;
        tinyxml2::XMLElement* element = xml_RTPSParticipant_child->FirstChildElement();

        while(element != nullptr)
        {
            std::string key(element->Name());

            if(key == "name") {
                pdata->m_RTPSParticipantName = element->GetText();
            } else if(key == "reader") {
                if(!loadXMLReaderEndpoint(element, pdata))
                {
                    logError(RTPS_EDP,"Reader Endpoint has error, ignoring");
                }
            } else if(key == "writer") {
                if(!loadXMLWriterEndpoint(element, pdata))
                {
                    logError(RTPS_EDP,"Writer Endpoint has error, ignoring");
                }
            } else {
                logError(RTPS_EDP,"Unknown XMK tag: " << key);
            }

            element = element->NextSiblingElement();
        }
}

bool EDPStaticXML::loadXMLReaderEndpoint(tinyxml2::XMLElement* xml_endpoint, StaticRTPSParticipantInfo* pdata)
{
    ReaderProxyData* rdata = new ReaderProxyData();

    tinyxml2::XMLNode* xml_endpoint_child = xml_endpoint;
    tinyxml2::XMLElement* element = xml_endpoint_child->FirstChildElement();

    while(element != nullptr)
    {
        std::string key(element->Name());

        //cout << "READER ENDPOINT: " << key << endl;
        if(key == "userId")
        {
            //cout << "USER ID FOUND";
            int16_t id = static_cast<int16_t>(std::strtol(element->GetText(), nullptr, 10));
            if(id<=0 || m_endpointIds.insert(id).second == false)
            {
                logError(RTPS_EDP,"Repeated or negative ID in XML file");
                delete(rdata);
                return false;
            }
            rdata->userDefinedId(id);
        }
        else if(key == "entityId")
        {
            int32_t id = std::strtol(element->GetText(), nullptr, 10);
            if(id<=0 || m_entityIds.insert(id).second == false)
            {
                logError(RTPS_EDP,"Repeated or negative entityId in XML file");
                delete(rdata);
                return false;
            }
            octet* c = (octet*)&id;
            rdata->guid().entityId.value[2] = c[0];
            rdata->guid().entityId.value[1] = c[1];
            rdata->guid().entityId.value[0] = c[2];
        }
        else if(key == "expectsInlineQos")
        {
            std::string auxString(element->GetText());
            if(auxString == "true")
            {
                rdata->m_expectsInlineQos = true;
                //cout << "READER WITH EXPECTS INLINE QOS " << endl;
            }
            else if (auxString == "false")
            {
                rdata->m_expectsInlineQos = false;
                //cout << "READER NOT NOT NOT EXPECTS INLINE QOS " << endl;
            }
            else
            {
                logError(RTPS_EDP,"Bad XML file, endpoint of expectsInlineQos: " << auxString << " is not valid");
                delete(rdata);return false;
            }
        }
        else if(key == "topicName")
        {
            rdata->topicName() = element->GetText();
        }
        else if(key == "topicDataType")
        {
            rdata->typeName() = element->GetText();
        }
        else if(key == "topicKind")
        {
            std::string auxString(element->GetText());
            if(auxString == "NO_KEY")
            {
                rdata->topicKind() = NO_KEY;
                rdata->guid().entityId.value[3] = 0x04;
            }
            else if (auxString == "WITH_KEY")
            {
                rdata->topicKind() = WITH_KEY;
                rdata->guid().entityId.value[3] = 0x07;
            }
            else
            {
                logError(RTPS_EDP,"Bad XML file, topic of kind: " << auxString << " is not valid");
                delete(rdata);return false;
            }
        }
        else if(key == "reliabilityQos")
        {
            std::string auxString(element->GetText());
            if(auxString == "RELIABLE_RELIABILITY_QOS")
                rdata->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            else if (auxString == "BEST_EFFORT_RELIABILITY_QOS")
                rdata->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
            else
            {
                logError(RTPS_EDP,"Bad XML file, endpoint of stateKind: " << auxString << " is not valid");
                delete(rdata);return false;
            }
        }
        else if(key == "unicastLocator")
        {
            Locator_t loc;
            loc.kind = 1;
            const char *address = element->Attribute("address");
            std::string auxString(address ? address : "");
            loc.set_IP4_address(auxString);
            int port = 0;
            element->QueryIntAttribute("port", &port);
            loc.port = port;
            rdata->unicastLocatorList().push_back(loc);
        }
        else if(key == "multicastLocator")
        {
            Locator_t loc;
            loc.kind = 1;
            const char *address = element->Attribute("address");
            std::string auxString(address ? address : "");
            loc.set_IP4_address(auxString);
            int port = 0;
            element->QueryIntAttribute("port", &port);
            loc.port = port;
            rdata->multicastLocatorList().push_back(loc);
        }
        else if(key == "topic")
        {
            const char *topicName = element->Attribute("name");
            const char *typeName = element->Attribute("dataType");
            const char *kind = element->Attribute("kind");

            rdata->topicName() = topicName ? std::string(topicName) : std::string("");
            rdata->typeName() = typeName ? std::string(typeName) : std::string("");
            std::string auxString(kind ? kind : "");
            if(auxString == "NO_KEY")
            {
                rdata->topicKind() = NO_KEY;
                rdata->guid().entityId.value[3] = 0x04;
            }
            else if (auxString == "WITH_KEY")
            {
                rdata->topicKind() = WITH_KEY;
                rdata->guid().entityId.value[3] = 0x07;
            }
            else
            {
                logError(RTPS_EDP,"Bad XML file, topic of kind: " << auxString << " is not valid");
                delete(rdata);return false;
            }
            if(rdata->topicName() == "EPROSIMA_UNKNOWN_STRING" || rdata->typeName() == "EPROSIMA_UNKNOWN_STRING")
            {
                logError(RTPS_EDP,"Bad XML file, topic: " << rdata->topicName() << " or typeName: " << rdata->typeName() << " undefined");
                delete(rdata);
                return false;
            }
        }
        else if(key == "durabilityQos")
        {
            std::string auxstring(element->GetText());
            if(auxstring == "TRANSIENT_LOCAL_DURABILITY_QOS")
                rdata->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            else if(auxstring == "VOLATILE_DURABILITY_QOS")
                rdata->m_qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
            else
            {
                logError(RTPS_EDP,"Bad XML file, durability of kind: " << auxstring << " is not valid");
                delete(rdata);return false;
            }
        }
        else if(key == "ownershipQos")
        {
            const char *ownership = element->Attribute("kind");
            std::string auxstring(ownership ? ownership : "OWNERHSIP kind NOT PRESENT");
            if(auxstring == "SHARED_OWNERSHIP_QOS")
                rdata->m_qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;
            else if(auxstring == "EXCLUSIVE_OWNERSHIP_QOS")
                rdata->m_qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;
            else
            {
                logError(RTPS_EDP,"Bad XML file, ownership of kind: " << auxstring << " is not valid");
                delete(rdata);return false;
            }
        }
        else if(key == "partitionQos")
        {
            rdata->m_qos.m_partition.push_back(element->GetText());
        }
        else if(key == "livelinessQos")
        {
            const char *kind = element->Attribute("kind");
            std::string auxstring(kind ? kind : "LIVELINESS kind NOT PRESENT");
            if(auxstring == "AUTOMATIC_LIVELINESS_QOS")
                rdata->m_qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
            else if(auxstring == "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS")
                rdata->m_qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
            else if(auxstring == "MANUAL_BY_TOPIC_LIVELINESS_QOS")
                rdata->m_qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;
            else
            {
                logError(RTPS_EDP,"Bad XML file, liveliness of kind: " << auxstring << " is not valid");
                delete(rdata);return false;
            }
            const char *leaseDuration_ms = element->Attribute("leaseDuration_ms");
            auxstring = std::string(leaseDuration_ms ? leaseDuration_ms : "INF");
            if(auxstring == "INF")
                rdata->m_qos.m_liveliness.lease_duration = c_TimeInfinite;
            else
            {
                uint32_t milliseclease = std::strtoul(auxstring.c_str(), nullptr, 10);
                rdata->m_qos.m_liveliness.lease_duration = TimeConv::MilliSeconds2Time_t((double)milliseclease);
                if(milliseclease == 0){
                    logWarning(RTPS_EDP,"BAD XML:livelinessQos leaseDuration is 0");
                }
            }
        }
        else
        {
            logWarning(RTPS_EDP,"Unkown Endpoint-XML tag, ignoring "<< key)
        }

        element = element->NextSiblingElement();
        }
    if(rdata->userDefinedId() == 0)
    {
        logError(RTPS_EDP,"Reader XML endpoint with NO ID defined");
        delete(rdata);
        return false;
    }
    pdata->m_readers.push_back(rdata);
    return true;
}


bool EDPStaticXML::loadXMLWriterEndpoint(tinyxml2::XMLElement* xml_endpoint, StaticRTPSParticipantInfo* pdata)
{
    WriterProxyData* wdata = new WriterProxyData();

    tinyxml2::XMLNode* xml_endpoint_child = xml_endpoint;
    tinyxml2::XMLElement* element = xml_endpoint_child->FirstChildElement();

    while(element != nullptr)
    {
        std::string key(element->Name());
        if(key == "userId")
        {
            int16_t id = static_cast<int16_t>(std::strtol(element->GetText(), nullptr, 10));
            if(id<=0 || m_endpointIds.insert(id).second == false)
            {
                logError(RTPS_EDP,"Repeated or negative ID in XML file");
                delete(wdata);
                return false;
            }
            wdata->userDefinedId(id);
        }
        else if(key == "entityId")
        {
            int32_t id = std::strtol(element->GetText(), nullptr, 10);
            if(id<=0 || m_entityIds.insert(id).second == false)
            {
                logError(RTPS_EDP,"Repeated or negative entityId in XML file");
                delete(wdata);
                return false;
            }
            octet* c = (octet*)&id;
            wdata->guid().entityId.value[2] = c[0];
            wdata->guid().entityId.value[1] = c[1];
            wdata->guid().entityId.value[0] = c[2];
        }
        else if(key == "expectsInlineQos")
        {
            logWarning(RTPS_EDP,"BAD XML tag: Writers don't use expectInlineQos tag");
        }
        else if(key == "topicName")
        {
            wdata->topicName(std::string(element->GetText()));
        }
        else if(key == "topicDataType")
        {
            wdata->typeName(std::string(element->GetText()));
        }
        else if(key == "topicKind")
        {
            std::string auxString = std::string(element->GetText());
            if(auxString == "NO_KEY")
            {
                wdata->topicKind(NO_KEY);
                wdata->guid().entityId.value[3] = 0x03;
            }
            else if (auxString == "WITH_KEY")
            {
                wdata->topicKind(WITH_KEY);
                wdata->guid().entityId.value[3] = 0x02;
            }
            else
            {
                logError(RTPS_EDP,"Bad XML file, topic of kind: " << auxString << " is not valid");
                delete(wdata);return false;
            }
        }
        else if(key == "reliabilityQos")
        {
            std::string auxString = std::string(element->GetText());
            if(auxString == "RELIABLE_RELIABILITY_QOS")
            wdata->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            else if (auxString == "BEST_EFFORT_RELIABILITY_QOS")
            wdata->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
            else
            {
                logError(RTPS_EDP,"Bad XML file, endpoint of stateKind: " << auxString << " is not valid");
                delete(wdata);return false;
            }
        }
        else if(key == "unicastLocator")
        {
            Locator_t loc;
            loc.kind = 1;
            const char *address = element->Attribute("address");
            std::string auxString(address ? address : "");
            loc.set_IP4_address(auxString);
            int port = 0;
            element->QueryIntAttribute("port", &port);
            loc.port = port;
            wdata->unicastLocatorList().push_back(loc);
        }
        else if(key == "multicastLocator")
        {
            Locator_t loc;
            loc.kind = 1;
            const char *address = element->Attribute("address");
            std::string auxString(address ? address : "");
            loc.set_IP4_address(auxString);
            int port = 0;
            element->QueryIntAttribute("port", &port);
            loc.port = port;
            wdata->multicastLocatorList().push_back(loc);
        }
        else if(key == "topic")
        {
            const char * topicName = element->Attribute("name");
            wdata->topicName(std::string(topicName ? topicName : "EPROSIMA_UNKNOWN_STRING"));
            const char * typeName = element->Attribute("dataType");
            wdata->typeName(std::string(typeName ? typeName : "EPROSIMA_UNKNOWN_STRING"));
            const char * kind = element->Attribute("kind");
            std::string auxString(kind ? kind : "");
            if(auxString == "NO_KEY")
            {
                wdata->topicKind(NO_KEY);
                wdata->guid().entityId.value[3] = 0x03;
            }
            else if (auxString == "WITH_KEY")
            {
                wdata->topicKind(WITH_KEY);
                wdata->guid().entityId.value[3] = 0x02;
            }
            else
            {
                logError(RTPS_EDP,"Bad XML file, topic of kind: " << auxString << " is not valid");
                delete(wdata);return false;
            }
            if(wdata->topicName() == "EPROSIMA_UNKNOWN_STRING" || wdata->typeName() == "EPROSIMA_UNKNOWN_STRING")
            {
                logError(RTPS_EDP,"Bad XML file, topic: "<<wdata->topicName() << " or typeName: "<<wdata->typeName() << " undefined");
                delete(wdata);
                return false;
            }
        }
        else if(key == "durabilityQos")
        {
            std::string auxstring = std::string(element->GetText());
            if(auxstring == "TRANSIENT_LOCAL_DURABILITY_QOS")
            wdata->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            else if(auxstring == "VOLATILE_DURABILITY_QOS")
            wdata->m_qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
            else
            {
                logError(RTPS_EDP,"Bad XML file, durability of kind: " << auxstring << " is not valid");
                delete(wdata);return false;
            }
        }
        else if(key == "ownershipQos")
        {
            const char * kind = element->Attribute("kind");
            std::string auxstring(kind ? kind : "OWNERHSIP kind NOT PRESENT");
            if(auxstring == "SHARED_OWNERSHIP_QOS")
            wdata->m_qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;
            else if(auxstring == "EXCLUSIVE_OWNERSHIP_QOS")
            wdata->m_qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;
            else
            {
                logError(RTPS_EDP,"Bad XML file, ownership of kind: " << auxstring << " is not valid");
                delete(wdata);return false;
            }
            int strength = 0;
            element->QueryIntAttribute("strength", &strength);
            wdata->m_qos.m_ownershipStrength.value = strength;
        }
        else if(key == "partitionQos")
        {
            wdata->m_qos.m_partition.push_back(element->GetText());
        }
        else if(key == "livelinessQos")
        {
            const char * kind = element->Attribute("kind");
            std::string auxstring(kind ? kind : "LIVELINESS kind NOT PRESENT");
            if(auxstring == "AUTOMATIC_LIVELINESS_QOS")
            wdata->m_qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
            else if(auxstring == "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS")
            wdata->m_qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
            else if(auxstring == "MANUAL_BY_TOPIC_LIVELINESS_QOS")
            wdata->m_qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;
            else
            {
                logError(RTPS_EDP,"Bad XML file, liveliness of kind: " << auxstring << " is not valid");
                delete(wdata);return false;
            }
            const char * leaseDuration_ms = element->Attribute("leaseDuration_ms");
            auxstring = std::string(leaseDuration_ms ? leaseDuration_ms : "INF");
            if(auxstring == "INF")
            wdata->m_qos.m_liveliness.lease_duration = c_TimeInfinite;
            else
            {
                uint32_t milliseclease = std::strtoul(auxstring.c_str(), nullptr, 10);
                wdata->m_qos.m_liveliness.lease_duration = TimeConv::MilliSeconds2Time_t((double)milliseclease);
                if(milliseclease == 0){
                    logWarning(RTPS_EDP,"BAD XML:livelinessQos leaseDuration is 0");
                }
            }
        }
        else
        {
            logWarning(RTPS_EDP,"Unkown Endpoint-XML tag, ignoring "<< key)
        }

        element = element->NextSiblingElement();
    }
    if(wdata->userDefinedId() == 0)
    {
        logError(RTPS_EDP,"Writer XML endpoint with NO ID defined");
        delete(wdata);
        return false;
    }
    pdata->m_writers.push_back(wdata);
    return true;
}

bool EDPStaticXML::lookforReader(std::string partname, uint16_t id,
        ReaderProxyData** rdataptr)
{
    for(std::vector<StaticRTPSParticipantInfo*>::iterator pit = m_RTPSParticipants.begin();
            pit!=m_RTPSParticipants.end();++pit)
    {
        if((*pit)->m_RTPSParticipantName == partname || true) //it doenst matter the name fo the RTPSParticipant, only for organizational purposes
        {
            for(std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
                    rit!=(*pit)->m_readers.end();++rit)
            {
                if((*rit)->userDefinedId() == id)
                {
                    *rdataptr = *rit;
                    return true;
                }
            }
        }
    }
    return false;
}

bool EDPStaticXML::lookforWriter(std::string partname, uint16_t id,
        WriterProxyData** wdataptr)
{
    for(std::vector<StaticRTPSParticipantInfo*>::iterator pit = m_RTPSParticipants.begin();
            pit!=m_RTPSParticipants.end();++pit)
    {
        if((*pit)->m_RTPSParticipantName == partname || true) //it doenst matter the name fo the RTPSParticipant, only for organizational purposes
        {
            for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
                    wit!=(*pit)->m_writers.end();++wit)
            {
                if((*wit)->userDefinedId() == id)
                {
                    *wdataptr = *wit;
                    return true;
                }
            }
        }
    }
    return false;
}


}
} /* namespace rtps */
} /* namespace eprosima */

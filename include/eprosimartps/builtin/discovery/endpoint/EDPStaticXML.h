/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPStaticXML.h
 *
 */

#ifndef EDPSTATICXML_H_
#define EDPSTATICXML_H_

#include <set>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree;

namespace eprosima {
namespace rtps {

class ReaderProxyData;
class WriterProxyData;



class StaticParticipantInfo{
public:
	StaticParticipantInfo(){};
	virtual ~StaticParticipantInfo(){};
	std::string m_participantName;
	std::vector<ReaderProxyData*> m_readers;
	std::vector<WriterProxyData*> m_writers;
};


class EDPStaticXML {
public:
	EDPStaticXML();
	virtual ~EDPStaticXML();
	bool loadXMLFile(std::string& filename);
	bool loadXMLReaderEndpoint(ptree::value_type& xml_endpoint,StaticParticipantInfo* pdata);
	bool loadXMLWriterEndpoint(ptree::value_type& xml_endpoint,StaticParticipantInfo* pdata);

	bool lookforReader(std::string partname,uint16_t id,ReaderProxyData** rdataptr);
	bool lookforWriter(std::string partname,uint16_t id,WriterProxyData** wdataptr);

private:
	std::set<int16_t> m_endpointIds;

	std::vector<StaticParticipantInfo*> m_participants;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDPSTATICXML_H_ */

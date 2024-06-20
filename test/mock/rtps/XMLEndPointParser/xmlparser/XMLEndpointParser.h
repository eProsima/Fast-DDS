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
 * @file XMLEndpointParser.h
 *
 */

#ifndef FASTDDS_XMLPARSER__XMLENDPOINTPARSER_H
#define FASTDDS_XMLPARSER__XMLENDPOINTPARSER_H
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <xmlparser/XMLParserCommon.h>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

/**
 * Class XMLEndpointParser used to parse the XML file that contains information about remote endpoints.
 * @ingroup DISCVOERYMODULE
 */
class XMLEndpointParser
{
public:

    XMLEndpointParser()
    {
    }

    virtual ~XMLEndpointParser()
    {
    }

    /**
     * Load the XML file
     * @param filename Name of the file to load and parse.
     * @return True if correct.
     */
    XMLP_ret loadXMLFile(
            std::string&)
    {
        return XMLP_ret::XML_OK;
    }

private:

};


} // namespace xmlparser
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_XMLPARSER__XMLENDPOINTPARSER_H

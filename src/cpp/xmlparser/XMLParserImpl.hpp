// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
//
#ifndef FASTRTPS_XMLPARSER_XMLPARSERIMPL_H_
#define FASTRTPS_XMLPARSER_XMLPARSERIMPL_H_

#include <string>

#include <fastrtps/xmlparser/XMLParser.h>

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

/**
 * Class XMLParserImpl, contains private implementation of XMLParser.
 * @ingroup XMLPARSER_MODULE
 */
class XMLParserImpl
{
public:

    /**
     * Load a XML file.
     * @param filename Name for the file to be loaded.
     * @param root Root node.
     * @param is_default Is the default XML file.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret loadXML(
            const std::string& filename,
            up_base_node_t& root,
            const bool& is_default);
};

} // namespace xmlparser
} // namespace fastrtps
} // namespace eprosima

#endif // ifndef FASTRTPS_XMLPARSER_XMLPARSERIMPL_H_

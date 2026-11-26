# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#{{{ Unit grouping the XML parser constants and the TinyXML-2 library, built from its sources when they are found.

set(XMLPARSER_COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.h
    )
if(TINYXML2_SOURCE_DIR)
    list(APPEND XMLPARSER_COMMON_SOURCES
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

fastdds_add_unit(xmlparser::common
    SOURCES
    ${XMLPARSER_COMMON_SOURCES}
    INCLUDE_DIRECTORIES
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    LINK_LIBRARIES
    PUBLIC
    ${TINYXML2_LIBRARY}
    )

#}}}

#{{{ Unit grouping the XML parser source code, which loads the XML files into the XMLProfileManager.

set(XMLPARSER_PARSER_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLDynamicParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.h
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserExtras.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserUtils.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLTree.h
    )

fastdds_add_unit(xmlparser::parser
    SOURCES
    ${XMLPARSER_PARSER_SOURCES}
    DEPENDS
    log
    rtps::basic
    rtps::builtin-data
    rtps::participant-attributes
    rtps::shm-transport
    rtps::transport
    rtps::udp-transport
    time
    utils::network
    xmlparser::common
    xmlparser::profile-manager
    xtypes
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    ${TINYXML2_LIBRARY}
    )

#}}}

#{{{ Unit with the XMLProfileManager, which stores the profiles loaded from XML.
# A unit test not linking core replaces it with the XMLProfileManager mock.

fastdds_add_unit(xmlparser::profile-manager
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.h
    DEPENDS
    log
    rtps::basic
    rtps::builtin-data
    time
    xmlparser::common
    xmlparser::parser
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    ${TINYXML2_LIBRARY}
    foonathan_memory
    )

#}}}

#{{{ Unit with the XMLEndpointParser, which loads the static discovery endpoints from XML.
# A unit test not linking core replaces it with the XMLEndpointParser mock.

fastdds_add_unit(xmlparser::endpoint-parser
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLEndpointParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLEndpointParser.h
    DEPENDS
    log
    rtps::basic
    rtps::proxy-data
    time
    utils::network
    xmlparser::common
    xmlparser::parser
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    ${TINYXML2_LIBRARY}
    foonathan_memory
    )

#}}}

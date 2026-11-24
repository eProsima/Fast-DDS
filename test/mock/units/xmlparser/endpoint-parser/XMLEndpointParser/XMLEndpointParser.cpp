// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of XMLEndpointParser: replaces src/cpp/xmlparser/XMLEndpointParser.cpp and forwards the
// methods tests need to the XMLEndpointParserMock companion of the instance.

#include <xmlparser/XMLEndpointParser.h>

#include <mock/xmlparser/XMLEndpointParserMock.hpp>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

using Companion = mock::MockCompanion<XMLEndpointParser>;

XMLEndpointParser::XMLEndpointParser()
{
    Companion::attach(this);
}

XMLEndpointParser::~XMLEndpointParser()
{
    Companion::detach(this);
}

XMLP_ret XMLEndpointParser::loadXMLFile(
        std::string& filename)
{
    return Companion::get(this).loadXMLFile(filename);
}

} // namespace xmlparser
} // namespace fastdds
} // namespace eprosima

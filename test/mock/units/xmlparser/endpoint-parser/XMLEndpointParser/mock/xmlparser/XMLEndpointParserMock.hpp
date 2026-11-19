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

#ifndef FASTDDS_TEST_MOCK_XMLPARSER__XMLENDPOINTPARSERMOCK_HPP
#define FASTDDS_TEST_MOCK_XMLPARSER__XMLENDPOINTPARSERMOCK_HPP

#include <string>

#include <gmock/gmock.h>

#include <mock/MockCompanion.hpp>
#include <xmlparser/XMLEndpointParser.h>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

/**
 * Expectations of a XMLEndpointParser created with the mock implementation, reached with mock::mock_of().
 * Loading a file returns XMLP_ret::XML_OK unless a test sets another action.
 */
class XMLEndpointParserMock
{
public:

    XMLEndpointParserMock()
    {
        ON_CALL(*this, loadXMLFile(testing::_)).WillByDefault(testing::Return(XMLP_ret::XML_OK));
    }

    MOCK_METHOD1(loadXMLFile, XMLP_ret(std::string&));

};

} // namespace xmlparser

namespace mock {

template<>
struct MockOf<xmlparser::XMLEndpointParser>
{
    using type = xmlparser::XMLEndpointParserMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_XMLPARSER__XMLENDPOINTPARSERMOCK_HPP

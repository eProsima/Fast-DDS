// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/xmlparser/XMLParser.h>
#include <xmlparser/XMLTree.h>

#include <gtest/gtest.h>

using eprosima::fastrtps::xmlparser::XMLParser;

class XMLParserTests: public ::testing::Test
{
    public:
        XMLParserTests()
        {
        }

        ~XMLParserTests()
        {
        }
};

TEST_F(XMLParserTests, )
{
    XMLParser::loadXMLFile("test_xml_profiles.xml");
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
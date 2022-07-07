// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file HelloWorldGenDataType.cpp
 *
 */

#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "../../types.hpp"

using namespace eprosima::fastrtps;

template <>
eprosima::fastrtps::types::DynamicType_ptr
    DataType<DataTypeKind::HELLO_WORLD , GeneratorKind::XML>::generate_type_() const
{
    // Load XML file
    std::string xml_file_name = "HelloWorldXml.xml";

    if (eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK !=
        eprosima::fastrtps::xmlparser::XMLProfileManager::loadXMLFile(xml_file_name))
    {
        throw std::ios_base::failure(
            "Cannot open XML file. Please, run the publisher from the folder that contains this XML file.");
    }

    // Create Dynamic data
    return eprosima::fastrtps::xmlparser::XMLProfileManager::getDynamicTypeByName(name())->build();
}

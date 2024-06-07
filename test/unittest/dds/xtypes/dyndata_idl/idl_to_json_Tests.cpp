// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "types/types.hpp"

#include <gtest/gtest.h>

using json = nlohmann::json;

// ENUMERATION_BUILDER(
//     DataTypeKind::HELLO_WORLD,
//     );

// // Crear un test en el que se testeen tipos más simples
// TEST(idl_to_json_Tests, SimpleTypesTests)
// {

// }

std::string expected_json_complete = R"({
    "complex_array": [
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 0
                }
            },
            "inner_union": {
                "second": 0
            },
            "long_array": [
                [
                    [
                        0,
                        0,
                        0,
                        0
                    ],
                    [
                        0,
                        0,
                        0,
                        0
                    ],
                    [
                        0,
                        0,
                        0,
                        0
                    ]
                ],
                [
                    [
                        0,
                        0,
                        0,
                        0
                    ],
                    [
                        0,
                        0,
                        0,
                        0
                    ],
                    [
                        0,
                        0,
                        0,
                        0
                    ]
                ]
            ],
            "my_aliased_bounded_string": "",
            "my_aliased_enum": {
                "name": "A",
                "value": 0
            },
            "my_aliased_struct": {
                "my_bool": false,
                "my_char": "\u0000",
                "my_double": 0.0,
                "my_float": 0.0,
                "my_int8": 0,
                "my_long": 0,
                "my_longdouble": 0.0,
                "my_longlong": 0,
                "my_octet": 0,
                "my_short": 0,
                "my_uint8": 0,
                "my_ulong": 0,
                "my_ulonglong": 0,
                "my_ushort": 0,
                "my_wchar": "\u0000"
            },
            "my_bitmask": {
                "active": [],
                "binary": "00000000000000000000000000000000",
                "value": 0
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 0
            },
            "my_bool": false,
            "my_bounded_string": "",
            "my_bounded_wstring": "",
            "my_char": "\u0000",
            "my_double": 0.0,
            "my_enum": {
                "name": "A",
                "value": 0
            },
            "my_float": 0.0,
            "my_int8": 0,
            "my_long": 0,
            "my_longdouble": 0.0,
            "my_longlong": 0,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "A",
                "value": 0
            },
            "my_short": 0,
            "my_string": "",
            "my_uint8": 0,
            "my_ulong": 0,
            "my_ulonglong": 0,
            "my_ushort": 0,
            "my_wchar": "\u0000",
            "my_wstring": "",
            "short_long_map": null,
            "short_sequence": [],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        },
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 0
                }
            },
            "inner_union": {
                "second": 0
            },
            "long_array": [
                [
                    [
                        0,
                        0,
                        0,
                        0
                    ],
                    [
                        0,
                        0,
                        0,
                        0
                    ],
                    [
                        0,
                        0,
                        0,
                        0
                    ]
                ],
                [
                    [
                        0,
                        0,
                        0,
                        0
                    ],
                    [
                        0,
                        0,
                        0,
                        0
                    ],
                    [
                        0,
                        0,
                        0,
                        0
                    ]
                ]
            ],
            "my_aliased_bounded_string": "",
            "my_aliased_enum": {
                "name": "A",
                "value": 0
            },
            "my_aliased_struct": {
                "my_bool": false,
                "my_char": "\u0000",
                "my_double": 0.0,
                "my_float": 0.0,
                "my_int8": 0,
                "my_long": 0,
                "my_longdouble": 0.0,
                "my_longlong": 0,
                "my_octet": 0,
                "my_short": 0,
                "my_uint8": 0,
                "my_ulong": 0,
                "my_ulonglong": 0,
                "my_ushort": 0,
                "my_wchar": "\u0000"
            },
            "my_bitmask": {
                "active": [],
                "binary": "00000000000000000000000000000000",
                "value": 0
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 0
            },
            "my_bool": false,
            "my_bounded_string": "",
            "my_bounded_wstring": "",
            "my_char": "\u0000",
            "my_double": 0.0,
            "my_enum": {
                "name": "A",
                "value": 0
            },
            "my_float": 0.0,
            "my_int8": 0,
            "my_long": 0,
            "my_longdouble": 0.0,
            "my_longlong": 0,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "A",
                "value": 0
            },
            "my_short": 0,
            "my_string": "",
            "my_uint8": 0,
            "my_ulong": 0,
            "my_ulonglong": 0,
            "my_ushort": 0,
            "my_wchar": "\u0000",
            "my_wstring": "",
            "short_long_map": null,
            "short_sequence": [],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    ],
    "complex_map": null,
    "complex_sequence": [],
    "index": 3,
    "inner_struct": {
        "bitmask_sequence": [],
        "complex_union": {
            "fourth": {
                "second": 0
            }
        },
        "inner_union": {
            "second": 0
        },
        "long_array": [
            [
                [
                    0,
                    0,
                    0,
                    0
                ],
                [
                    0,
                    0,
                    0,
                    0
                ],
                [
                    0,
                    0,
                    0,
                    0
                ]
            ],
            [
                [
                    0,
                    0,
                    0,
                    0
                ],
                [
                    0,
                    0,
                    0,
                    0
                ],
                [
                    0,
                    0,
                    0,
                    0
                ]
            ]
        ],
        "my_aliased_bounded_string": "",
        "my_aliased_enum": {
            "name": "A",
            "value": 0
        },
        "my_aliased_struct": {
            "my_bool": false,
            "my_char": "\u0000",
            "my_double": 0.0,
            "my_float": 0.0,
            "my_int8": 0,
            "my_long": 0,
            "my_longdouble": 0.0,
            "my_longlong": 0,
            "my_octet": 0,
            "my_short": 0,
            "my_uint8": 0,
            "my_ulong": 0,
            "my_ulonglong": 0,
            "my_ushort": 0,
            "my_wchar": "\u0000"
        },
        "my_bitmask": {
            "active": [],
            "binary": "00000000000000000000000000000000",
            "value": 0
        },
        "my_bitset": {
            "a": 0,
            "b": false,
            "c": 0,
            "d": 0
        },
        "my_bool": false,
        "my_bounded_string": "",
        "my_bounded_wstring": "",
        "my_char": "\u0000",
        "my_double": 0.0,
        "my_enum": {
            "name": "A",
            "value": 0
        },
        "my_float": 0.0,
        "my_int8": 0,
        "my_long": 0,
        "my_longdouble": 0.0,
        "my_longlong": 0,
        "my_octet": 0,
        "my_recursive_alias": {
            "name": "A",
            "value": 0
        },
        "my_short": 0,
        "my_string": "",
        "my_uint8": 0,
        "my_ulong": 0,
        "my_ulonglong": 0,
        "my_ushort": 0,
        "my_wchar": "\u0000",
        "my_wstring": "",
        "short_long_map": null,
        "short_sequence": [],
        "string_alias_unbounded_map": null,
        "string_unbounded_map": null
    }
})";

TEST(idl_to_json_Tests, DataTest)
{
    // std::string topic_name = "TypeIntrospectionTopic";
    DataTypeKind data_type_complete = DataTypeKind::DATA_TEST;

    auto data_type_complete_ = data_type_factory(data_type_complete);

    eprosima::fastdds::dds::DynamicType::_ref_type dynamic_type_complete = data_type_complete_->generate_type_support_and_get_dyn_type();

    // Get dynamic data depending on data_type
    auto dyn_data_complete = eprosima::fastdds::dds::DynamicDataFactory::get_instance()->create_data(dynamic_type_complete);

    dyn_data_complete->set_uint32_value(dyn_data_complete->get_member_id_by_name("index"), 3);

    std::stringstream generated_json_complete;
    generated_json_complete << std::setw(4);
    if (eprosima::fastdds::dds::RETCODE_OK ==
            eprosima::fastdds::dds::json_serialize(
                dyn_data_complete,
                generated_json_complete,
                eprosima::fastdds::dds::DynamicDataJsonFormat::EPROSIMA))
    {
        std::cout << "---------------" << std::endl;
        std::cout << generated_json_complete.str() << std::endl;
        std::cout << "---------------" << std::endl;

        ASSERT_EQ(generated_json_complete.str(), expected_json_complete);
    }
    else
    {
        std::cout << "FAILED" << std::endl;
    }

}


int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

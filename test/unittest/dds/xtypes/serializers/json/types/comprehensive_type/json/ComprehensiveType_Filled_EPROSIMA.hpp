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

#include <map>
#include <string>

std::string expected_json_comprehensive_filled_eprosima_1 = R"({
    "complex_array": [
        {
            "bitmask_sequence": [
                {
                    "active": [
                        "flag0"
                    ],
                    "binary": "00000000000000000000000000000001",
                    "value": 1
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 1
                }
            },
            "enum_sequence": [
                {
                    "name": "B",
                    "value": 1
                }
            ],
            "inner_union": {
                "second": 1
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "B",
                "value": 1
            },
            "my_aliased_struct": {
                "my_bool": true,
                "my_char": "e",
                "my_double": 0.5,
                "my_float": 0.5,
                "my_int8": 1,
                "my_long": 1,
                "my_longdouble": 0.5,
                "my_longlong": 1,
                "my_octet": 0,
                "my_short": 1,
                "my_uint8": 1,
                "my_ulong": 1,
                "my_ulonglong": 1,
                "my_ushort": 1,
                "my_wchar": "e"
            },
            "my_bitmask": {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 1
            },
            "my_bool": true,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "e",
            "my_double": 0.5,
            "my_enum": {
                "name": "B",
                "value": 1
            },
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "B",
                "value": 1
            },
            "my_short": 1,
            "my_string": "my_string",
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 1
            },
            "short_sequence": [
                0,
                1
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        },
        {
            "bitmask_sequence": [
                {
                    "active": [
                        "flag0"
                    ],
                    "binary": "00000000000000000000000000000001",
                    "value": 1
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 1
                }
            },
            "enum_sequence": [
                {
                    "name": "B",
                    "value": 1
                }
            ],
            "inner_union": {
                "second": 1
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "B",
                "value": 1
            },
            "my_aliased_struct": {
                "my_bool": true,
                "my_char": "e",
                "my_double": 0.5,
                "my_float": 0.5,
                "my_int8": 1,
                "my_long": 1,
                "my_longdouble": 0.5,
                "my_longlong": 1,
                "my_octet": 0,
                "my_short": 1,
                "my_uint8": 1,
                "my_ulong": 1,
                "my_ulonglong": 1,
                "my_ushort": 1,
                "my_wchar": "e"
            },
            "my_bitmask": {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 1
            },
            "my_bool": true,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "e",
            "my_double": 0.5,
            "my_enum": {
                "name": "B",
                "value": 1
            },
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "B",
                "value": 1
            },
            "my_short": 1,
            "my_string": "my_string",
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 1
            },
            "short_sequence": [
                0,
                1
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    ],
    "complex_map": {
        "0": {
            "bitmask_sequence": [
                {
                    "active": [
                        "flag0"
                    ],
                    "binary": "00000000000000000000000000000001",
                    "value": 1
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 1
                }
            },
            "enum_sequence": [
                {
                    "name": "B",
                    "value": 1
                }
            ],
            "inner_union": {
                "second": 1
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "B",
                "value": 1
            },
            "my_aliased_struct": {
                "my_bool": true,
                "my_char": "e",
                "my_double": 0.5,
                "my_float": 0.5,
                "my_int8": 1,
                "my_long": 1,
                "my_longdouble": 0.5,
                "my_longlong": 1,
                "my_octet": 0,
                "my_short": 1,
                "my_uint8": 1,
                "my_ulong": 1,
                "my_ulonglong": 1,
                "my_ushort": 1,
                "my_wchar": "e"
            },
            "my_bitmask": {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 1
            },
            "my_bool": true,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "e",
            "my_double": 0.5,
            "my_enum": {
                "name": "B",
                "value": 1
            },
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "B",
                "value": 1
            },
            "my_short": 1,
            "my_string": "my_string",
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 1
            },
            "short_sequence": [
                0,
                1
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    },
    "complex_sequence": [
        {
            "bitmask_sequence": [
                {
                    "active": [
                        "flag0"
                    ],
                    "binary": "00000000000000000000000000000001",
                    "value": 1
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 1
                }
            },
            "enum_sequence": [
                {
                    "name": "B",
                    "value": 1
                }
            ],
            "inner_union": {
                "second": 1
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "B",
                "value": 1
            },
            "my_aliased_struct": {
                "my_bool": true,
                "my_char": "e",
                "my_double": 0.5,
                "my_float": 0.5,
                "my_int8": 1,
                "my_long": 1,
                "my_longdouble": 0.5,
                "my_longlong": 1,
                "my_octet": 0,
                "my_short": 1,
                "my_uint8": 1,
                "my_ulong": 1,
                "my_ulonglong": 1,
                "my_ushort": 1,
                "my_wchar": "e"
            },
            "my_bitmask": {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 1
            },
            "my_bool": true,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "e",
            "my_double": 0.5,
            "my_enum": {
                "name": "B",
                "value": 1
            },
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "B",
                "value": 1
            },
            "my_short": 1,
            "my_string": "my_string",
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 1
            },
            "short_sequence": [
                0,
                1
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    ],
    "index": 1,
    "inner_struct": {
        "bitmask_sequence": [
            {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            }
        ],
        "complex_union": {
            "fourth": {
                "second": 1
            }
        },
        "enum_sequence": [
            {
                "name": "B",
                "value": 1
            }
        ],
        "inner_union": {
            "second": 1
        },
        "long_array": [
            [
                [
                    1,
                    2,
                    3,
                    4
                ],
                [
                    5,
                    6,
                    7,
                    8
                ],
                [
                    9,
                    10,
                    11,
                    12
                ]
            ],
            [
                [
                    13,
                    14,
                    15,
                    16
                ],
                [
                    17,
                    18,
                    19,
                    20
                ],
                [
                    21,
                    22,
                    23,
                    24
                ]
            ]
        ],
        "my_aliased_bounded_string": "my_aliased_bounded_string",
        "my_aliased_enum": {
            "name": "B",
            "value": 1
        },
        "my_aliased_struct": {
            "my_bool": true,
            "my_char": "e",
            "my_double": 0.5,
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_short": 1,
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e"
        },
        "my_bitmask": {
            "active": [
                "flag0"
            ],
            "binary": "00000000000000000000000000000001",
            "value": 1
        },
        "my_bitset": {
            "a": 0,
            "b": false,
            "c": 0,
            "d": 1
        },
        "my_bool": true,
        "my_bounded_string": "my_bounded_string",
        "my_bounded_wstring": "my_bounded_wstring",
        "my_char": "e",
        "my_double": 0.5,
        "my_enum": {
            "name": "B",
            "value": 1
        },
        "my_float": 0.5,
        "my_int8": 1,
        "my_long": 1,
        "my_longdouble": 0.5,
        "my_longlong": 1,
        "my_octet": 0,
        "my_recursive_alias": {
            "name": "B",
            "value": 1
        },
        "my_short": 1,
        "my_string": "my_string",
        "my_uint8": 1,
        "my_ulong": 1,
        "my_ulonglong": 1,
        "my_ushort": 1,
        "my_wchar": "e",
        "my_wstring": "my_string",
        "short_long_map": {
            "1": 1
        },
        "short_sequence": [
            0,
            1
        ],
        "string_alias_unbounded_map": {
            "0": "string_alias_unbounded_map"
        },
        "string_unbounded_map": {
            "0": "string_unbounded_map"
        }
    }
})";

const std::string expected_json_comprehensive_filled_eprosima_2 = R"({
    "complex_array": [
        {
            "bitmask_sequence": [
                {
                    "active": [],
                    "binary": "00000000000000000000000000000000",
                    "value": 0
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 2
                }
            },
            "enum_sequence": [
                {
                    "name": "C",
                    "value": 2
                }
            ],
            "inner_union": {
                "second": 2
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "C",
                "value": 2
            },
            "my_aliased_struct": {
                "my_bool": false,
                "my_char": "o",
                "my_double": 1.0,
                "my_float": 1.0,
                "my_int8": 2,
                "my_long": 2,
                "my_longdouble": 1.0,
                "my_longlong": 2,
                "my_octet": 0,
                "my_short": 2,
                "my_uint8": 2,
                "my_ulong": 2,
                "my_ulonglong": 2,
                "my_ushort": 2,
                "my_wchar": "o"
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
                "d": 2
            },
            "my_bool": false,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "o",
            "my_double": 1.0,
            "my_enum": {
                "name": "C",
                "value": 2
            },
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "C",
                "value": 2
            },
            "my_short": 2,
            "my_string": "my_string",
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 2
            },
            "short_sequence": [
                0,
                2
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        },
        {
            "bitmask_sequence": [
                {
                    "active": [],
                    "binary": "00000000000000000000000000000000",
                    "value": 0
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 2
                }
            },
            "enum_sequence": [
                {
                    "name": "C",
                    "value": 2
                }
            ],
            "inner_union": {
                "second": 2
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "C",
                "value": 2
            },
            "my_aliased_struct": {
                "my_bool": false,
                "my_char": "o",
                "my_double": 1.0,
                "my_float": 1.0,
                "my_int8": 2,
                "my_long": 2,
                "my_longdouble": 1.0,
                "my_longlong": 2,
                "my_octet": 0,
                "my_short": 2,
                "my_uint8": 2,
                "my_ulong": 2,
                "my_ulonglong": 2,
                "my_ushort": 2,
                "my_wchar": "o"
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
                "d": 2
            },
            "my_bool": false,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "o",
            "my_double": 1.0,
            "my_enum": {
                "name": "C",
                "value": 2
            },
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "C",
                "value": 2
            },
            "my_short": 2,
            "my_string": "my_string",
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 2
            },
            "short_sequence": [
                0,
                2
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    ],
    "complex_map": {
        "0": {
            "bitmask_sequence": [
                {
                    "active": [],
                    "binary": "00000000000000000000000000000000",
                    "value": 0
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 2
                }
            },
            "enum_sequence": [
                {
                    "name": "C",
                    "value": 2
                }
            ],
            "inner_union": {
                "second": 2
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "C",
                "value": 2
            },
            "my_aliased_struct": {
                "my_bool": false,
                "my_char": "o",
                "my_double": 1.0,
                "my_float": 1.0,
                "my_int8": 2,
                "my_long": 2,
                "my_longdouble": 1.0,
                "my_longlong": 2,
                "my_octet": 0,
                "my_short": 2,
                "my_uint8": 2,
                "my_ulong": 2,
                "my_ulonglong": 2,
                "my_ushort": 2,
                "my_wchar": "o"
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
                "d": 2
            },
            "my_bool": false,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "o",
            "my_double": 1.0,
            "my_enum": {
                "name": "C",
                "value": 2
            },
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "C",
                "value": 2
            },
            "my_short": 2,
            "my_string": "my_string",
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 2
            },
            "short_sequence": [
                0,
                2
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    },
    "complex_sequence": [
        {
            "bitmask_sequence": [
                {
                    "active": [],
                    "binary": "00000000000000000000000000000000",
                    "value": 0
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 2
                }
            },
            "enum_sequence": [
                {
                    "name": "C",
                    "value": 2
                }
            ],
            "inner_union": {
                "second": 2
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "C",
                "value": 2
            },
            "my_aliased_struct": {
                "my_bool": false,
                "my_char": "o",
                "my_double": 1.0,
                "my_float": 1.0,
                "my_int8": 2,
                "my_long": 2,
                "my_longdouble": 1.0,
                "my_longlong": 2,
                "my_octet": 0,
                "my_short": 2,
                "my_uint8": 2,
                "my_ulong": 2,
                "my_ulonglong": 2,
                "my_ushort": 2,
                "my_wchar": "o"
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
                "d": 2
            },
            "my_bool": false,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "o",
            "my_double": 1.0,
            "my_enum": {
                "name": "C",
                "value": 2
            },
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "C",
                "value": 2
            },
            "my_short": 2,
            "my_string": "my_string",
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 2
            },
            "short_sequence": [
                0,
                2
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    ],
    "index": 2,
    "inner_struct": {
        "bitmask_sequence": [
            {
                "active": [],
                "binary": "00000000000000000000000000000000",
                "value": 0
            }
        ],
        "complex_union": {
            "fourth": {
                "second": 2
            }
        },
        "enum_sequence": [
            {
                "name": "C",
                "value": 2
            }
        ],
        "inner_union": {
            "second": 2
        },
        "long_array": [
            [
                [
                    1,
                    2,
                    3,
                    4
                ],
                [
                    5,
                    6,
                    7,
                    8
                ],
                [
                    9,
                    10,
                    11,
                    12
                ]
            ],
            [
                [
                    13,
                    14,
                    15,
                    16
                ],
                [
                    17,
                    18,
                    19,
                    20
                ],
                [
                    21,
                    22,
                    23,
                    24
                ]
            ]
        ],
        "my_aliased_bounded_string": "my_aliased_bounded_string",
        "my_aliased_enum": {
            "name": "C",
            "value": 2
        },
        "my_aliased_struct": {
            "my_bool": false,
            "my_char": "o",
            "my_double": 1.0,
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_short": 2,
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o"
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
            "d": 2
        },
        "my_bool": false,
        "my_bounded_string": "my_bounded_string",
        "my_bounded_wstring": "my_bounded_wstring",
        "my_char": "o",
        "my_double": 1.0,
        "my_enum": {
            "name": "C",
            "value": 2
        },
        "my_float": 1.0,
        "my_int8": 2,
        "my_long": 2,
        "my_longdouble": 1.0,
        "my_longlong": 2,
        "my_octet": 0,
        "my_recursive_alias": {
            "name": "C",
            "value": 2
        },
        "my_short": 2,
        "my_string": "my_string",
        "my_uint8": 2,
        "my_ulong": 2,
        "my_ulonglong": 2,
        "my_ushort": 2,
        "my_wchar": "o",
        "my_wstring": "my_string",
        "short_long_map": {
            "1": 2
        },
        "short_sequence": [
            0,
            2
        ],
        "string_alias_unbounded_map": {
            "0": "string_alias_unbounded_map"
        },
        "string_unbounded_map": {
            "0": "string_unbounded_map"
        }
    }
})";

const std::string expected_json_comprehensive_filled_eprosima_3 = R"({
    "complex_array": [
        {
            "bitmask_sequence": [
                {
                    "active": [
                        "flag0"
                    ],
                    "binary": "00000000000000000000000000000001",
                    "value": 1
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 3
                }
            },
            "enum_sequence": [
                {
                    "name": "A",
                    "value": 0
                }
            ],
            "inner_union": {
                "second": 3
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "A",
                "value": 0
            },
            "my_aliased_struct": {
                "my_bool": true,
                "my_char": "e",
                "my_double": 1.5,
                "my_float": 1.5,
                "my_int8": 3,
                "my_long": 3,
                "my_longdouble": 1.5,
                "my_longlong": 3,
                "my_octet": 0,
                "my_short": 3,
                "my_uint8": 3,
                "my_ulong": 3,
                "my_ulonglong": 3,
                "my_ushort": 3,
                "my_wchar": "e"
            },
            "my_bitmask": {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 3
            },
            "my_bool": true,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "e",
            "my_double": 1.5,
            "my_enum": {
                "name": "A",
                "value": 0
            },
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "A",
                "value": 0
            },
            "my_short": 3,
            "my_string": "my_string",
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 3
            },
            "short_sequence": [
                0,
                3
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        },
        {
            "bitmask_sequence": [
                {
                    "active": [
                        "flag0"
                    ],
                    "binary": "00000000000000000000000000000001",
                    "value": 1
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 3
                }
            },
            "enum_sequence": [
                {
                    "name": "A",
                    "value": 0
                }
            ],
            "inner_union": {
                "second": 3
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "A",
                "value": 0
            },
            "my_aliased_struct": {
                "my_bool": true,
                "my_char": "e",
                "my_double": 1.5,
                "my_float": 1.5,
                "my_int8": 3,
                "my_long": 3,
                "my_longdouble": 1.5,
                "my_longlong": 3,
                "my_octet": 0,
                "my_short": 3,
                "my_uint8": 3,
                "my_ulong": 3,
                "my_ulonglong": 3,
                "my_ushort": 3,
                "my_wchar": "e"
            },
            "my_bitmask": {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 3
            },
            "my_bool": true,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "e",
            "my_double": 1.5,
            "my_enum": {
                "name": "A",
                "value": 0
            },
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "A",
                "value": 0
            },
            "my_short": 3,
            "my_string": "my_string",
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 3
            },
            "short_sequence": [
                0,
                3
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    ],
    "complex_map": {
        "0": {
            "bitmask_sequence": [
                {
                    "active": [
                        "flag0"
                    ],
                    "binary": "00000000000000000000000000000001",
                    "value": 1
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 3
                }
            },
            "enum_sequence": [
                {
                    "name": "A",
                    "value": 0
                }
            ],
            "inner_union": {
                "second": 3
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "A",
                "value": 0
            },
            "my_aliased_struct": {
                "my_bool": true,
                "my_char": "e",
                "my_double": 1.5,
                "my_float": 1.5,
                "my_int8": 3,
                "my_long": 3,
                "my_longdouble": 1.5,
                "my_longlong": 3,
                "my_octet": 0,
                "my_short": 3,
                "my_uint8": 3,
                "my_ulong": 3,
                "my_ulonglong": 3,
                "my_ushort": 3,
                "my_wchar": "e"
            },
            "my_bitmask": {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 3
            },
            "my_bool": true,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "e",
            "my_double": 1.5,
            "my_enum": {
                "name": "A",
                "value": 0
            },
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "A",
                "value": 0
            },
            "my_short": 3,
            "my_string": "my_string",
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 3
            },
            "short_sequence": [
                0,
                3
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    },
    "complex_sequence": [
        {
            "bitmask_sequence": [
                {
                    "active": [
                        "flag0"
                    ],
                    "binary": "00000000000000000000000000000001",
                    "value": 1
                }
            ],
            "complex_union": {
                "fourth": {
                    "second": 3
                }
            },
            "enum_sequence": [
                {
                    "name": "A",
                    "value": 0
                }
            ],
            "inner_union": {
                "second": 3
            },
            "long_array": [
                [
                    [
                        1,
                        2,
                        3,
                        4
                    ],
                    [
                        5,
                        6,
                        7,
                        8
                    ],
                    [
                        9,
                        10,
                        11,
                        12
                    ]
                ],
                [
                    [
                        13,
                        14,
                        15,
                        16
                    ],
                    [
                        17,
                        18,
                        19,
                        20
                    ],
                    [
                        21,
                        22,
                        23,
                        24
                    ]
                ]
            ],
            "my_aliased_bounded_string": "my_aliased_bounded_string",
            "my_aliased_enum": {
                "name": "A",
                "value": 0
            },
            "my_aliased_struct": {
                "my_bool": true,
                "my_char": "e",
                "my_double": 1.5,
                "my_float": 1.5,
                "my_int8": 3,
                "my_long": 3,
                "my_longdouble": 1.5,
                "my_longlong": 3,
                "my_octet": 0,
                "my_short": 3,
                "my_uint8": 3,
                "my_ulong": 3,
                "my_ulonglong": 3,
                "my_ushort": 3,
                "my_wchar": "e"
            },
            "my_bitmask": {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            },
            "my_bitset": {
                "a": 0,
                "b": false,
                "c": 0,
                "d": 3
            },
            "my_bool": true,
            "my_bounded_string": "my_bounded_string",
            "my_bounded_wstring": "my_bounded_wstring",
            "my_char": "e",
            "my_double": 1.5,
            "my_enum": {
                "name": "A",
                "value": 0
            },
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_recursive_alias": {
                "name": "A",
                "value": 0
            },
            "my_short": 3,
            "my_string": "my_string",
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "1": 3
            },
            "short_sequence": [
                0,
                3
            ],
            "string_alias_unbounded_map": {
                "0": "string_alias_unbounded_map"
            },
            "string_unbounded_map": {
                "0": "string_unbounded_map"
            }
        }
    ],
    "index": 3,
    "inner_struct": {
        "bitmask_sequence": [
            {
                "active": [
                    "flag0"
                ],
                "binary": "00000000000000000000000000000001",
                "value": 1
            }
        ],
        "complex_union": {
            "fourth": {
                "second": 3
            }
        },
        "enum_sequence": [
            {
                "name": "A",
                "value": 0
            }
        ],
        "inner_union": {
            "second": 3
        },
        "long_array": [
            [
                [
                    1,
                    2,
                    3,
                    4
                ],
                [
                    5,
                    6,
                    7,
                    8
                ],
                [
                    9,
                    10,
                    11,
                    12
                ]
            ],
            [
                [
                    13,
                    14,
                    15,
                    16
                ],
                [
                    17,
                    18,
                    19,
                    20
                ],
                [
                    21,
                    22,
                    23,
                    24
                ]
            ]
        ],
        "my_aliased_bounded_string": "my_aliased_bounded_string",
        "my_aliased_enum": {
            "name": "A",
            "value": 0
        },
        "my_aliased_struct": {
            "my_bool": true,
            "my_char": "e",
            "my_double": 1.5,
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_short": 3,
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e"
        },
        "my_bitmask": {
            "active": [
                "flag0"
            ],
            "binary": "00000000000000000000000000000001",
            "value": 1
        },
        "my_bitset": {
            "a": 0,
            "b": false,
            "c": 0,
            "d": 3
        },
        "my_bool": true,
        "my_bounded_string": "my_bounded_string",
        "my_bounded_wstring": "my_bounded_wstring",
        "my_char": "e",
        "my_double": 1.5,
        "my_enum": {
            "name": "A",
            "value": 0
        },
        "my_float": 1.5,
        "my_int8": 3,
        "my_long": 3,
        "my_longdouble": 1.5,
        "my_longlong": 3,
        "my_octet": 0,
        "my_recursive_alias": {
            "name": "A",
            "value": 0
        },
        "my_short": 3,
        "my_string": "my_string",
        "my_uint8": 3,
        "my_ulong": 3,
        "my_ulonglong": 3,
        "my_ushort": 3,
        "my_wchar": "e",
        "my_wstring": "my_string",
        "short_long_map": {
            "1": 3
        },
        "short_sequence": [
            0,
            3
        ],
        "string_alias_unbounded_map": {
            "0": "string_alias_unbounded_map"
        },
        "string_unbounded_map": {
            "0": "string_unbounded_map"
        }
    }
})";

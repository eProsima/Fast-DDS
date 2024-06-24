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

const std::string expected_json_comprehensive_filled_omg_1 = R"({
    "complex_array": [
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 1
                }
            },
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
            "my_aliased_enum": "B",
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
            "my_bitmask": 1,
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
            "my_enum": "B",
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_recursive_alias": "B",
            "my_short": 1,
            "my_string": "my_string",
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 1
            },
            "short_sequence": [
                0,
                1
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        },
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 1
                }
            },
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
            "my_aliased_enum": "B",
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
            "my_bitmask": 1,
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
            "my_enum": "B",
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_recursive_alias": "B",
            "my_short": 1,
            "my_string": "my_string",
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 1
            },
            "short_sequence": [
                0,
                1
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    ],
    "complex_map": {
        "0": {
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
            "my_aliased_enum": "A",
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
            "my_bitmask": 0,
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
            "my_enum": "A",
            "my_float": 0.0,
            "my_int8": 0,
            "my_long": 0,
            "my_longdouble": 0.0,
            "my_longlong": 0,
            "my_octet": 0,
            "my_recursive_alias": "A",
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
        "1": {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 1
                }
            },
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
            "my_aliased_enum": "B",
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
            "my_bitmask": 1,
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
            "my_enum": "B",
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_recursive_alias": "B",
            "my_short": 1,
            "my_string": "my_string",
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 1
            },
            "short_sequence": [
                0,
                1
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    },
    "complex_sequence": [
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 1
                }
            },
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
            "my_aliased_enum": "B",
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
            "my_bitmask": 1,
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
            "my_enum": "B",
            "my_float": 0.5,
            "my_int8": 1,
            "my_long": 1,
            "my_longdouble": 0.5,
            "my_longlong": 1,
            "my_octet": 0,
            "my_recursive_alias": "B",
            "my_short": 1,
            "my_string": "my_string",
            "my_uint8": 1,
            "my_ulong": 1,
            "my_ulonglong": 1,
            "my_ushort": 1,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 1
            },
            "short_sequence": [
                0,
                1
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    ],
    "index": 1,
    "inner_struct": {
        "bitmask_sequence": [],
        "complex_union": {
            "fourth": {
                "second": 1
            }
        },
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
        "my_aliased_enum": "B",
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
        "my_bitmask": 1,
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
        "my_enum": "B",
        "my_float": 0.5,
        "my_int8": 1,
        "my_long": 1,
        "my_longdouble": 0.5,
        "my_longlong": 1,
        "my_octet": 0,
        "my_recursive_alias": "B",
        "my_short": 1,
        "my_string": "my_string",
        "my_uint8": 1,
        "my_ulong": 1,
        "my_ulonglong": 1,
        "my_ushort": 1,
        "my_wchar": "e",
        "my_wstring": "my_string",
        "short_long_map": {
            "0": 1
        },
        "short_sequence": [
            0,
            1
        ],
        "string_alias_unbounded_map": null,
        "string_unbounded_map": null
    }
})";

const std::string expected_json_comprehensive_filled_omg_2 = R"({
    "complex_array": [
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 2
                }
            },
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
            "my_aliased_enum": "C",
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
            "my_bitmask": 0,
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
            "my_enum": "C",
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_recursive_alias": "C",
            "my_short": 2,
            "my_string": "my_string",
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 2
            },
            "short_sequence": [
                0,
                2
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        },
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 2
                }
            },
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
            "my_aliased_enum": "C",
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
            "my_bitmask": 0,
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
            "my_enum": "C",
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_recursive_alias": "C",
            "my_short": 2,
            "my_string": "my_string",
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 2
            },
            "short_sequence": [
                0,
                2
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    ],
    "complex_map": {
        "0": {
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
            "my_aliased_enum": "A",
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
            "my_bitmask": 0,
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
            "my_enum": "A",
            "my_float": 0.0,
            "my_int8": 0,
            "my_long": 0,
            "my_longdouble": 0.0,
            "my_longlong": 0,
            "my_octet": 0,
            "my_recursive_alias": "A",
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
        "1": {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 2
                }
            },
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
            "my_aliased_enum": "C",
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
            "my_bitmask": 0,
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
            "my_enum": "C",
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_recursive_alias": "C",
            "my_short": 2,
            "my_string": "my_string",
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 2
            },
            "short_sequence": [
                0,
                2
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    },
    "complex_sequence": [
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 2
                }
            },
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
            "my_aliased_enum": "C",
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
            "my_bitmask": 0,
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
            "my_enum": "C",
            "my_float": 1.0,
            "my_int8": 2,
            "my_long": 2,
            "my_longdouble": 1.0,
            "my_longlong": 2,
            "my_octet": 0,
            "my_recursive_alias": "C",
            "my_short": 2,
            "my_string": "my_string",
            "my_uint8": 2,
            "my_ulong": 2,
            "my_ulonglong": 2,
            "my_ushort": 2,
            "my_wchar": "o",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 2
            },
            "short_sequence": [
                0,
                2
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    ],
    "index": 2,
    "inner_struct": {
        "bitmask_sequence": [],
        "complex_union": {
            "fourth": {
                "second": 2
            }
        },
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
        "my_aliased_enum": "C",
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
        "my_bitmask": 0,
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
        "my_enum": "C",
        "my_float": 1.0,
        "my_int8": 2,
        "my_long": 2,
        "my_longdouble": 1.0,
        "my_longlong": 2,
        "my_octet": 0,
        "my_recursive_alias": "C",
        "my_short": 2,
        "my_string": "my_string",
        "my_uint8": 2,
        "my_ulong": 2,
        "my_ulonglong": 2,
        "my_ushort": 2,
        "my_wchar": "o",
        "my_wstring": "my_string",
        "short_long_map": {
            "0": 2
        },
        "short_sequence": [
            0,
            2
        ],
        "string_alias_unbounded_map": null,
        "string_unbounded_map": null
    }
})";

const std::string expected_json_comprehensive_filled_omg_3 = R"({
    "complex_array": [
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 3
                }
            },
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
            "my_aliased_enum": "A",
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
            "my_bitmask": 1,
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
            "my_enum": "A",
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_recursive_alias": "A",
            "my_short": 3,
            "my_string": "my_string",
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 3
            },
            "short_sequence": [
                0,
                3
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        },
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 3
                }
            },
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
            "my_aliased_enum": "A",
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
            "my_bitmask": 1,
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
            "my_enum": "A",
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_recursive_alias": "A",
            "my_short": 3,
            "my_string": "my_string",
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 3
            },
            "short_sequence": [
                0,
                3
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    ],
    "complex_map": {
        "0": {
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
            "my_aliased_enum": "A",
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
            "my_bitmask": 0,
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
            "my_enum": "A",
            "my_float": 0.0,
            "my_int8": 0,
            "my_long": 0,
            "my_longdouble": 0.0,
            "my_longlong": 0,
            "my_octet": 0,
            "my_recursive_alias": "A",
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
        "1": {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 3
                }
            },
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
            "my_aliased_enum": "A",
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
            "my_bitmask": 1,
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
            "my_enum": "A",
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_recursive_alias": "A",
            "my_short": 3,
            "my_string": "my_string",
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 3
            },
            "short_sequence": [
                0,
                3
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    },
    "complex_sequence": [
        {
            "bitmask_sequence": [],
            "complex_union": {
                "fourth": {
                    "second": 3
                }
            },
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
            "my_aliased_enum": "A",
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
            "my_bitmask": 1,
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
            "my_enum": "A",
            "my_float": 1.5,
            "my_int8": 3,
            "my_long": 3,
            "my_longdouble": 1.5,
            "my_longlong": 3,
            "my_octet": 0,
            "my_recursive_alias": "A",
            "my_short": 3,
            "my_string": "my_string",
            "my_uint8": 3,
            "my_ulong": 3,
            "my_ulonglong": 3,
            "my_ushort": 3,
            "my_wchar": "e",
            "my_wstring": "my_string",
            "short_long_map": {
                "0": 3
            },
            "short_sequence": [
                0,
                3
            ],
            "string_alias_unbounded_map": null,
            "string_unbounded_map": null
        }
    ],
    "index": 3,
    "inner_struct": {
        "bitmask_sequence": [],
        "complex_union": {
            "fourth": {
                "second": 3
            }
        },
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
        "my_aliased_enum": "A",
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
        "my_bitmask": 1,
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
        "my_enum": "A",
        "my_float": 1.5,
        "my_int8": 3,
        "my_long": 3,
        "my_longdouble": 1.5,
        "my_longlong": 3,
        "my_octet": 0,
        "my_recursive_alias": "A",
        "my_short": 3,
        "my_string": "my_string",
        "my_uint8": 3,
        "my_ulong": 3,
        "my_ulonglong": 3,
        "my_ushort": 3,
        "my_wchar": "e",
        "my_wstring": "my_string",
        "short_long_map": {
            "0": 3
        },
        "short_sequence": [
            0,
            3
        ],
        "string_alias_unbounded_map": null,
        "string_unbounded_map": null
    }
})";

std::map<int, std::string> expected_json_comprehensive_filled_omg = {
    {1, expected_json_comprehensive_filled_omg_1},
    {2, expected_json_comprehensive_filled_omg_2},
    {3, expected_json_comprehensive_filled_omg_3}};

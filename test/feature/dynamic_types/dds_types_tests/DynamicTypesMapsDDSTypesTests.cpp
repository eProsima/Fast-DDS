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

#include <algorithm>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include <gtest/gtest.h>

#include "../DynamicTypesDDSTypesTest.hpp"
#include "../../../dds-types-test/helpers/basic_inner_types.hpp"
#include "../../../dds-types-test/mapsPubSubTypes.hpp"
#include "../../../dds-types-test/mapsTypeObjectSupport.hpp"
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr const char* const short_short_map_struct_name = "MapShortShort";
constexpr const char* const short_ushort_map_struct_name = "MapShortUShort";
constexpr const char* const short_long_map_struct_name = "MapShortLong";
constexpr const char* const short_ulong_map_struct_name = "MapShortULong";
constexpr const char* const short_longlong_map_struct_name = "MapShortLongLong";
constexpr const char* const short_ulonglong_map_struct_name = "MapShortULongLong";
constexpr const char* const short_float_map_struct_name = "MapShortFloat";
constexpr const char* const short_double_map_struct_name = "MapShortDouble";
constexpr const char* const short_longdouble_map_struct_name = "MapShortLongDouble";
constexpr const char* const short_boolean_map_struct_name = "MapShortBoolean";
constexpr const char* const short_octet_map_struct_name = "MapShortOctet";
constexpr const char* const short_char_map_struct_name = "MapShortChar";
constexpr const char* const short_wchar_map_struct_name = "MapShortWChar";
constexpr const char* const short_string_map_struct_name = "MapShortString";
constexpr const char* const short_wstring_map_struct_name = "MapShortWString";
constexpr const char* const short_inneraliasboundedstringhelper_map_struct_name =
        "MapShortInnerAliasBoundedStringHelper";
constexpr const char* const short_inneraliasboundedwstringhelper_map_struct_name =
        "MapShortInnerAliasBoundedWStringHelper";
constexpr const char* const short_innerenumhelper_map_struct_name = "MapShortInnerEnumHelper";
constexpr const char* const short_innerbitmaskhelper_map_struct_name = "MapShortInnerBitMaskHelper";
constexpr const char* const short_inneraliashelper_map_struct_name = "MapShortInnerAliasHelper";
constexpr const char* const short_inneraliasarrayhelper_map_struct_name = "MapShortInnerAliasArrayHelper";
constexpr const char* const short_inneraliassequencehelper_map_struct_name = "MapShortInnerAliasSequenceHelper";
constexpr const char* const short_inneraliasmaphelper_map_struct_name = "MapShortInnerAliasMapHelper";
constexpr const char* const short_innerunionhelper_map_struct_name = "MapShortInnerUnionHelper";
constexpr const char* const short_innerstructurehelper_map_struct_name = "MapShortInnerStructureHelper";
constexpr const char* const short_innerbitsethelper_map_struct_name = "MapShortInnerBitsetHelper";
constexpr const char* const ushort_short_map_struct_name = "MapUShortShort";
constexpr const char* const ushort_ushort_map_struct_name = "MapUShortUShort";
constexpr const char* const ushort_long_map_struct_name = "MapUShortLong";
constexpr const char* const ushort_ulong_map_struct_name = "MapUShortULong";
constexpr const char* const ushort_longlong_map_struct_name = "MapUShortLongLong";
constexpr const char* const ushort_ulonglong_map_struct_name = "MapUShortULongLong";
constexpr const char* const ushort_float_map_struct_name = "MapUShortFloat";
constexpr const char* const ushort_double_map_struct_name = "MapUShortDouble";
constexpr const char* const ushort_longdouble_map_struct_name = "MapUShortLongDouble";
constexpr const char* const ushort_boolean_map_struct_name = "MapUShortBoolean";
constexpr const char* const ushort_octet_map_struct_name = "MapUShortOctet";
constexpr const char* const ushort_char_map_struct_name = "MapUShortChar";
constexpr const char* const ushort_wchar_map_struct_name = "MapUShortWChar";
constexpr const char* const ushort_string_map_struct_name = "MapUShortString";
constexpr const char* const ushort_wstring_map_struct_name = "MapUShortWString";
constexpr const char* const ushort_inneraliasboundedstringhelper_map_struct_name =
        "MapUShortInnerAliasBoundedStringHelper";
constexpr const char* const ushort_inneraliasboundedwstringhelper_map_struct_name =
        "MapUShortInnerAliasBoundedWStringHelper";
constexpr const char* const ushort_innerenumhelper_map_struct_name = "MapUShortInnerEnumHelper";
constexpr const char* const ushort_innerbitmaskhelper_map_struct_name = "MapUShortInnerBitMaskHelper";
constexpr const char* const ushort_inneraliashelper_map_struct_name = "MapUShortInnerAliasHelper";
constexpr const char* const ushort_inneraliasarrayhelper_map_struct_name = "MapUShortInnerAliasArrayHelper";
constexpr const char* const ushort_inneraliassequencehelper_map_struct_name = "MapUShortInnerAliasSequenceHelper";
constexpr const char* const ushort_inneraliasmaphelper_map_struct_name = "MapUShortInnerAliasMapHelper";
constexpr const char* const ushort_innerunionhelper_map_struct_name = "MapUShortInnerUnionHelper";
constexpr const char* const ushort_innerstructurehelper_map_struct_name = "MapUShortInnerStructureHelper";
constexpr const char* const ushort_innerbitsethelper_map_struct_name = "MapUShortInnerBitsetHelper";
constexpr const char* const long_short_map_struct_name = "MapLongShort";
constexpr const char* const long_ushort_map_struct_name = "MapLongUShort";
constexpr const char* const long_long_map_struct_name = "MapLongLong";
constexpr const char* const long_ulong_map_struct_name = "MapLongULong";
constexpr const char* const long_longlong_map_struct_name = "MapLongKeyLongLongValue";
constexpr const char* const long_ulonglong_map_struct_name = "MapLongULongLong";
constexpr const char* const long_float_map_struct_name = "MapLongFloat";
constexpr const char* const long_double_map_struct_name = "MapLongDouble";
constexpr const char* const long_longdouble_map_struct_name = "MapLongKeyLongDoubleValue";
constexpr const char* const long_boolean_map_struct_name = "MapLongBoolean";
constexpr const char* const long_octet_map_struct_name = "MapLongOctet";
constexpr const char* const long_char_map_struct_name = "MapLongChar";
constexpr const char* const long_wchar_map_struct_name = "MapLongWChar";
constexpr const char* const long_string_map_struct_name = "MapLongString";
constexpr const char* const long_wstring_map_struct_name = "MapLongWString";
constexpr const char* const long_inneraliasboundedstringhelper_map_struct_name =
        "MapLongInnerAliasBoundedStringHelper";
constexpr const char* const long_inneraliasboundedwstringhelper_map_struct_name =
        "MapLongInnerAliasBoundedWStringHelper";
constexpr const char* const long_innerenumhelper_map_struct_name = "MapLongInnerEnumHelper";
constexpr const char* const long_innerbitmaskhelper_map_struct_name = "MapLongInnerBitMaskHelper";
constexpr const char* const long_inneraliashelper_map_struct_name = "MapLongInnerAliasHelper";
constexpr const char* const long_inneraliasarrayhelper_map_struct_name = "MapLongInnerAliasArrayHelper";
constexpr const char* const long_inneraliassequencehelper_map_struct_name = "MapLongInnerAliasSequenceHelper";
constexpr const char* const long_inneraliasmaphelper_map_struct_name = "MapLongInnerAliasMapHelper";
constexpr const char* const long_innerunionhelper_map_struct_name = "MapLongInnerUnionHelper";
constexpr const char* const long_innerstructurehelper_map_struct_name = "MapLongInnerStructureHelper";
constexpr const char* const long_innerbitsethelper_map_struct_name = "MapLongInnerBitsetHelper";
constexpr const char* const ulong_short_map_struct_name = "MapULongShort";
constexpr const char* const ulong_ushort_map_struct_name = "MapULongUShort";
constexpr const char* const ulong_long_map_struct_name = "MapULongLong";
constexpr const char* const ulong_ulong_map_struct_name = "MapULongULong";
constexpr const char* const ulong_longlong_map_struct_name = "MapKeyULongValueLongLong";
constexpr const char* const ulong_ulonglong_map_struct_name = "MapULongULongLong";
constexpr const char* const ulong_float_map_struct_name = "MapULongFloat";
constexpr const char* const ulong_double_map_struct_name = "MapULongDouble";
constexpr const char* const ulong_longdouble_map_struct_name = "MapKeyULongValueLongDouble";
constexpr const char* const ulong_boolean_map_struct_name = "MapULongBoolean";
constexpr const char* const ulong_octet_map_struct_name = "MapULongOctet";
constexpr const char* const ulong_char_map_struct_name = "MapULongChar";
constexpr const char* const ulong_wchar_map_struct_name = "MapULongWChar";
constexpr const char* const ulong_string_map_struct_name = "MapULongString";
constexpr const char* const ulong_wstring_map_struct_name = "MapULongWString";
constexpr const char* const ulong_inneraliasboundedstringhelper_map_struct_name =
        "MapULongInnerAliasBoundedStringHelper";
constexpr const char* const ulong_inneraliasboundedwstringhelper_map_struct_name =
        "MapULongInnerAliasBoundedWStringHelper";
constexpr const char* const ulong_innerenumhelper_map_struct_name = "MapULongInnerEnumHelper";
constexpr const char* const ulong_innerbitmaskhelper_map_struct_name = "MapULongInnerBitMaskHelper";
constexpr const char* const ulong_inneraliashelper_map_struct_name = "MapULongInnerAliasHelper";
constexpr const char* const ulong_inneraliasarrayhelper_map_struct_name = "MapULongInnerAliasArrayHelper";
constexpr const char* const ulong_inneraliassequencehelper_map_struct_name = "MapULongInnerAliasSequenceHelper";
constexpr const char* const ulong_inneraliasmaphelper_map_struct_name = "MapULongInnerAliasMapHelper";
constexpr const char* const ulong_innerunionhelper_map_struct_name = "MapULongInnerUnionHelper";
constexpr const char* const ulong_innerstructurehelper_map_struct_name = "MapULongInnerStructureHelper";
constexpr const char* const ulong_innerbitsethelper_map_struct_name = "MapULongInnerBitsetHelper";
constexpr const char* const longlong_short_map_struct_name = "MapLongLongShort";
constexpr const char* const longlong_ushort_map_struct_name = "MapLongLongUShort";
constexpr const char* const longlong_long_map_struct_name = "MapLongLongKeyLongValue";
constexpr const char* const longlong_ulong_map_struct_name = "MapLongLongULong";
constexpr const char* const longlong_longlong_map_struct_name = "MapLongLongLongLong";
constexpr const char* const longlong_ulonglong_map_struct_name = "MapLongLongULongLong";
constexpr const char* const longlong_float_map_struct_name = "MapLongLongFloat";
constexpr const char* const longlong_double_map_struct_name = "MapLongLongKeyDoubleValue";
constexpr const char* const longlong_longdouble_map_struct_name = "MapLongLongLongDouble";
constexpr const char* const longlong_boolean_map_struct_name = "MapLongLongBoolean";
constexpr const char* const longlong_octet_map_struct_name = "MapLongLongOctet";
constexpr const char* const longlong_char_map_struct_name = "MapLongLongChar";
constexpr const char* const longlong_wchar_map_struct_name = "MapLongLongWChar";
constexpr const char* const longlong_string_map_struct_name = "MapLongLongString";
constexpr const char* const longlong_wstring_map_struct_name = "MapLongLongWString";
constexpr const char* const longlong_inneraliasboundedstringhelper_map_struct_name =
        "MapLongLongInnerAliasBoundedStringHelper";
constexpr const char* const longlong_inneraliasboundedwstringhelper_map_struct_name =
        "MapLongLongInnerAliasBoundedWStringHelper";
constexpr const char* const longlong_innerenumhelper_map_struct_name = "MapLongLongInnerEnumHelper";
constexpr const char* const longlong_innerbitmaskhelper_map_struct_name = "MapLongLongInnerBitMaskHelper";
constexpr const char* const longlong_inneraliashelper_map_struct_name = "MapLongLongInnerAliasHelper";
constexpr const char* const longlong_inneraliasarrayhelper_map_struct_name = "MapLongLongInnerAliasArrayHelper";
constexpr const char* const longlong_inneraliassequencehelper_map_struct_name = "MapLongLongInnerAliasSequenceHelper";
constexpr const char* const longlong_inneraliasmaphelper_map_struct_name = "MapLongLongInnerAliasMapHelper";
constexpr const char* const longlong_innerunionhelper_map_struct_name = "MapLongLongInnerUnionHelper";
constexpr const char* const longlong_innerstructurehelper_map_struct_name = "MapLongLongInnerStructureHelper";
constexpr const char* const longlong_innerbitsethelper_map_struct_name = "MapLongLongInnerBitsetHelper";
constexpr const char* const ulonglong_short_map_struct_name = "MapULongLongShort";
constexpr const char* const ulonglong_ushort_map_struct_name = "MapULongLongUShort";
constexpr const char* const ulonglong_long_map_struct_name = "MapULongLongLong";
constexpr const char* const ulonglong_ulong_map_struct_name = "MapULongLongULong";
constexpr const char* const ulonglong_longlong_map_struct_name = "MapULongLongLongLong";
constexpr const char* const ulonglong_ulonglong_map_struct_name = "MapULongLongULongLong";
constexpr const char* const ulonglong_float_map_struct_name = "MapULongLongFloat";
constexpr const char* const ulonglong_double_map_struct_name = "MapKeyULongLongValueDouble";
constexpr const char* const ulonglong_longdouble_map_struct_name = "MapULongLongLongDouble";
constexpr const char* const ulonglong_boolean_map_struct_name = "MapULongLongBoolean";
constexpr const char* const ulonglong_octet_map_struct_name = "MapULongLongOctet";
constexpr const char* const ulonglong_char_map_struct_name = "MapULongLongChar";
constexpr const char* const ulonglong_wchar_map_struct_name = "MapULongLongWChar";
constexpr const char* const ulonglong_string_map_struct_name = "MapULongLongString";
constexpr const char* const ulonglong_wstring_map_struct_name = "MapULongLongWString";
constexpr const char* const ulonglong_inneraliasboundedstringhelper_map_struct_name =
        "MapULongLongInnerAliasBoundedStringHelper";
constexpr const char* const ulonglong_inneraliasboundedwstringhelper_map_struct_name =
        "MapULongLongInnerAliasBoundedWStringHelper";
constexpr const char* const ulonglong_innerenumhelper_map_struct_name = "MapULongLongInnerEnumHelper";
constexpr const char* const ulonglong_innerbitmaskhelper_map_struct_name = "MapULongLongInnerBitMaskHelper";
constexpr const char* const ulonglong_inneraliashelper_map_struct_name = "MapULongLongInnerAliasHelper";
constexpr const char* const ulonglong_inneraliasarrayhelper_map_struct_name = "MapULongLongInnerAliasArrayHelper";
constexpr const char* const ulonglong_inneraliassequencehelper_map_struct_name = "MapULongLongInnerAliasSequenceHelper";
constexpr const char* const ulonglong_inneraliasmaphelper_map_struct_name = "MapULongLongInnerAliasMapHelper";
constexpr const char* const ulonglong_innerunionhelper_map_struct_name = "MapULongLongInnerUnionHelper";
constexpr const char* const ulonglong_innerstructurehelper_map_struct_name = "MapULongLongInnerStructureHelper";
constexpr const char* const ulonglong_innerbitsethelper_map_struct_name = "MapULongLongInnerBitsetHelper";
constexpr const char* const string_short_map_struct_name = "MapStringShort";
constexpr const char* const string_ushort_map_struct_name = "MapStringUShort";
constexpr const char* const string_long_map_struct_name = "MapStringLong";
constexpr const char* const string_ulong_map_struct_name = "MapStringULong";
constexpr const char* const string_longlong_map_struct_name = "MapStringLongLong";
constexpr const char* const string_ulonglong_map_struct_name = "MapStringULongLong";
constexpr const char* const string_float_map_struct_name = "MapStringFloat";
constexpr const char* const string_double_map_struct_name = "MapStringDouble";
constexpr const char* const string_longdouble_map_struct_name = "MapStringLongDouble";
constexpr const char* const string_boolean_map_struct_name = "MapStringBoolean";
constexpr const char* const string_octet_map_struct_name = "MapStringOctet";
constexpr const char* const string_char_map_struct_name = "MapStringChar";
constexpr const char* const string_wchar_map_struct_name = "MapStringWChar";
constexpr const char* const string_string_map_struct_name = "MapStringString";
constexpr const char* const string_wstring_map_struct_name = "MapStringWString";
constexpr const char* const string_inneraliasboundedstringhelper_map_struct_name =
        "MapStringInnerAliasBoundedStringHelper";
constexpr const char* const string_inneraliasboundedwstringhelper_map_struct_name =
        "MapStringInnerAliasBoundedWStringHelper";
constexpr const char* const string_innerenumhelper_map_struct_name = "MapStringInnerEnumHelper";
constexpr const char* const string_innerbitmaskhelper_map_struct_name = "MapStringInnerBitMaskHelper";
constexpr const char* const string_inneraliashelper_map_struct_name = "MapStringInnerAliasHelper";
constexpr const char* const string_inneraliasarrayhelper_map_struct_name = "MapStringInnerAliasArrayHelper";
constexpr const char* const string_inneraliassequencehelper_map_struct_name = "MapStringInnerAliasSequenceHelper";
constexpr const char* const string_inneraliasmaphelper_map_struct_name = "MapStringInnerAliasMapHelper";
constexpr const char* const string_innerunionhelper_map_struct_name = "MapStringInnerUnionHelper";
constexpr const char* const string_innerstructurehelper_map_struct_name = "MapStringInnerStructureHelper";
constexpr const char* const string_innerbitsethelper_map_struct_name = "MapStringInnerBitsetHelper";
// Maps with wstring as key were omitted because there is not supported currently.
constexpr const char* const inneraliasboundedstringhelper_short_map_struct_name =
        "MapInnerAliasBoundedStringHelperShort";
constexpr const char* const inneraliasboundedstringhelper_ushort_map_struct_name =
        "MapInnerAliasBoundedStringHelperUShort";
constexpr const char* const inneraliasboundedstringhelper_long_map_struct_name = "MapInnerAliasBoundedStringHelperLong";
constexpr const char* const inneraliasboundedstringhelper_ulong_map_struct_name =
        "MapInnerAliasBoundedStringHelperULong";
constexpr const char* const inneraliasboundedstringhelper_longlong_map_struct_name =
        "MapInnerAliasBoundedStringHelperLongLong";
constexpr const char* const inneraliasboundedstringhelper_ulonglong_map_struct_name =
        "MapInnerAliasBoundedStringHelperULongLong";
constexpr const char* const inneraliasboundedstringhelper_float_map_struct_name =
        "MapInnerAliasBoundedStringHelperFloat";
constexpr const char* const inneraliasboundedstringhelper_double_map_struct_name =
        "MapInnerAliasBoundedStringHelperDouble";
constexpr const char* const inneraliasboundedstringhelper_longdouble_map_struct_name =
        "MapInnerAliasBoundedStringHelperLongDouble";
constexpr const char* const inneraliasboundedstringhelper_boolean_map_struct_name =
        "MapInnerAliasBoundedStringHelperBoolean";
constexpr const char* const inneraliasboundedstringhelper_octet_map_struct_name =
        "MapInnerAliasBoundedStringHelperOctet";
constexpr const char* const inneraliasboundedstringhelper_char_map_struct_name = "MapInnerAliasBoundedStringHelperChar";
constexpr const char* const inneraliasboundedstringhelper_wchar_map_struct_name =
        "MapInnerAliasBoundedStringHelperWChar";
constexpr const char* const inneraliasboundedstringhelper_string_map_struct_name =
        "MapInnerAliasBoundedStringHelperString";
constexpr const char* const inneraliasboundedstringhelper_wstring_map_struct_name =
        "MapInnerAliasBoundedStringHelperWString";
constexpr const char* const inneraliasboundedstringhelper_inneraliasboundedstringhelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelper";
constexpr const char* const inneraliasboundedstringhelper_inneraliasboundedwstringhelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelper";
constexpr const char* const inneraliasboundedstringhelper_innerenumhelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerEnumHelper";
constexpr const char* const inneraliasboundedstringhelper_innerbitmaskhelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerBitMaskHelper";
constexpr const char* const inneraliasboundedstringhelper_inneraliashelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerAliasHelper";
constexpr const char* const inneraliasboundedstringhelper_inneraliasarrayhelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerAliasArrayHelper";
constexpr const char* const inneraliasboundedstringhelper_inneraliassequencehelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerAliasSequenceHelper";
constexpr const char* const inneraliasboundedstringhelper_inneraliasmaphelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerAliasMapHelper";
constexpr const char* const inneraliasboundedstringhelper_innerunionhelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerUnionHelper";
constexpr const char* const inneraliasboundedstringhelper_innerstructurehelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerStructureHelper";
constexpr const char* const inneraliasboundedstringhelper_innerbitsethelper_map_struct_name =
        "MapInnerAliasBoundedStringHelperInnerBitsetHelper";
// Maps with alias of wstring as key were omitted because there is not supported currently.
constexpr const char* const bounded_small_map_struct_name = "BoundedSmallMap";
constexpr const char* const bounded_large_map_struct_name = "BoundedLargeMap";

constexpr const char* const var_short_short_map = "var_map_short_short";
constexpr const char* const var_short_ushort_map = "var_map_short_ushort";
constexpr const char* const var_short_long_map = "var_map_short_long";
constexpr const char* const var_short_ulong_map = "var_map_short_ulong";
constexpr const char* const var_short_longlong_map = "var_map_short_longlong";
constexpr const char* const var_short_ulonglong_map = "var_map_short_ulonglong";
constexpr const char* const var_short_float_map = "var_map_short_float";
constexpr const char* const var_short_double_map = "var_map_short_double";
constexpr const char* const var_short_longdouble_map = "var_map_short_longdouble";
constexpr const char* const var_short_boolean_map = "var_map_short_boolean";
constexpr const char* const var_short_octet_map = "var_map_short_octet";
constexpr const char* const var_short_char_map = "var_map_short_char";
constexpr const char* const var_short_wchar_map = "var_map_short_wchar";
constexpr const char* const var_short_string_map = "var_map_short_string";
constexpr const char* const var_short_wstring_map = "var_map_short_wstring";
constexpr const char* const var_short_inneraliasboundedstringhelper_map = "var_map_short_inneraliasboundedstringhelper";
constexpr const char* const var_short_inneraliasboundedwstringhelper_map =
        "var_map_short_inneraliasboundedwstringhelper";
constexpr const char* const var_short_innerenumhelper_map = "var_map_short_innerenumhelper";
constexpr const char* const var_short_innerbitmaskhelper_map = "var_map_short_innerbitmaskhelper";
constexpr const char* const var_short_inneraliashelper_map = "var_map_short_inneraliashelper";
constexpr const char* const var_short_inneraliasarrayhelper_map = "var_map_short_inneraliasarrayhelper";
constexpr const char* const var_short_inneraliassequencehelper_map = "var_map_short_inneraliassequencehelper";
constexpr const char* const var_short_inneraliasmaphelper_map = "var_map_short_inneraliasmaphelper";
constexpr const char* const var_short_innerunionhelper_map = "var_map_short_innerunionhelper";
constexpr const char* const var_short_innerstructurehelper_map = "var_map_short_innerstructurehelper";
constexpr const char* const var_short_innerbitsethelper_map = "var_map_short_innerbitsethelper";
constexpr const char* const var_ushort_short_map = "var_map_ushort_short";
constexpr const char* const var_ushort_ushort_map = "var_map_ushort_ushort";
constexpr const char* const var_ushort_long_map = "var_map_ushort_long";
constexpr const char* const var_ushort_ulong_map = "var_map_ushort_ulong";
constexpr const char* const var_ushort_longlong_map = "var_map_ushort_longlong";
constexpr const char* const var_ushort_ulonglong_map = "var_map_ushort_ulonglong";
constexpr const char* const var_ushort_float_map = "var_map_ushort_float";
constexpr const char* const var_ushort_double_map = "var_map_ushort_double";
constexpr const char* const var_ushort_longdouble_map = "var_map_ushort_longdouble";
constexpr const char* const var_ushort_boolean_map = "var_map_ushort_boolean";
constexpr const char* const var_ushort_octet_map = "var_map_ushort_octet";
constexpr const char* const var_ushort_char_map = "var_map_ushort_char";
constexpr const char* const var_ushort_wchar_map = "var_map_ushort_wchar";
constexpr const char* const var_ushort_string_map = "var_map_ushort_string";
constexpr const char* const var_ushort_wstring_map = "var_map_ushort_wstring";
constexpr const char* const var_ushort_inneraliasboundedstringhelper_map =
        "var_map_ushort_inneraliasboundedstringhelper";
constexpr const char* const var_ushort_inneraliasboundedwstringhelper_map =
        "var_map_ushort_inneraliasboundedwstringhelper";
constexpr const char* const var_ushort_innerenumhelper_map = "var_map_ushort_innerenumhelper";
constexpr const char* const var_ushort_innerbitmaskhelper_map = "var_map_ushort_innerbitmaskhelper";
constexpr const char* const var_ushort_inneraliashelper_map = "var_map_ushort_inneraliashelper";
constexpr const char* const var_ushort_inneraliasarrayhelper_map = "var_map_ushort_inneraliasarrayhelper";
constexpr const char* const var_ushort_inneraliassequencehelper_map = "var_map_ushort_inneraliassequencehelper";
constexpr const char* const var_ushort_inneraliasmaphelper_map = "var_map_ushort_inneraliasmaphelper";
constexpr const char* const var_ushort_innerunionhelper_map = "var_map_ushort_innerunionhelper";
constexpr const char* const var_ushort_innerstructurehelper_map = "var_map_ushort_innerstructurehelper";
constexpr const char* const var_ushort_innerbitsethelper_map = "var_map_ushort_innerbitsethelper";
constexpr const char* const var_long_short_map = "var_map_long_short";
constexpr const char* const var_long_ushort_map = "var_map_long_ushort";
constexpr const char* const var_long_long_map = "var_map_long_long";
constexpr const char* const var_long_ulong_map = "var_map_long_ulong";
constexpr const char* const var_long_longlong_map = "var_map_long_longlong";
constexpr const char* const var_long_ulonglong_map = "var_map_long_ulonglong";
constexpr const char* const var_long_float_map = "var_map_long_float";
constexpr const char* const var_long_double_map = "var_map_long_double";
constexpr const char* const var_long_longdouble_map = "var_map_long_longdouble";
constexpr const char* const var_long_boolean_map = "var_map_long_boolean";
constexpr const char* const var_long_octet_map = "var_map_long_octet";
constexpr const char* const var_long_char_map = "var_map_long_char";
constexpr const char* const var_long_wchar_map = "var_map_long_wchar";
constexpr const char* const var_long_string_map = "var_map_long_string";
constexpr const char* const var_long_wstring_map = "var_map_long_wstring";
constexpr const char* const var_long_inneraliasboundedstringhelper_map =
        "var_map_long_inneraliasboundedstringhelper";
constexpr const char* const var_long_inneraliasboundedwstringhelper_map =
        "var_map_long_inneraliasboundedwstringhelper";
constexpr const char* const var_long_innerenumhelper_map = "var_map_long_innerenumhelper";
constexpr const char* const var_long_innerbitmaskhelper_map = "var_map_long_innerbitmaskhelper";
constexpr const char* const var_long_inneraliashelper_map = "var_map_long_inneraliashelper";
constexpr const char* const var_long_inneraliasarrayhelper_map = "var_map_long_inneraliasarrayhelper";
constexpr const char* const var_long_inneraliassequencehelper_map = "var_map_long_inneraliassequencehelper";
constexpr const char* const var_long_inneraliasmaphelper_map = "var_map_long_inneraliasmaphelper";
constexpr const char* const var_long_innerunionhelper_map = "var_map_long_innerunionhelper";
constexpr const char* const var_long_innerstructurehelper_map = "var_map_long_innerstructurehelper";
constexpr const char* const var_long_innerbitsethelper_map = "var_map_long_innerbitsethelper";
constexpr const char* const var_ulong_short_map = "var_map_ulong_short";
constexpr const char* const var_ulong_ushort_map = "var_map_ulong_ushort";
constexpr const char* const var_ulong_long_map = "var_map_ulong_long";
constexpr const char* const var_ulong_ulong_map = "var_map_ulong_ulong";
constexpr const char* const var_ulong_longlong_map = "var_map_ulong_longlong";
constexpr const char* const var_ulong_ulonglong_map = "var_map_ulong_ulonglong";
constexpr const char* const var_ulong_float_map = "var_map_ulong_float";
constexpr const char* const var_ulong_double_map = "var_map_ulong_double";
constexpr const char* const var_ulong_longdouble_map = "var_map_ulong_longdouble";
constexpr const char* const var_ulong_boolean_map = "var_map_ulong_boolean";
constexpr const char* const var_ulong_octet_map = "var_map_ulong_octet";
constexpr const char* const var_ulong_char_map = "var_map_ulong_char";
constexpr const char* const var_ulong_wchar_map = "var_map_ulong_wchar";
constexpr const char* const var_ulong_string_map = "var_map_ulong_string";
constexpr const char* const var_ulong_wstring_map = "var_map_ulong_wstring";
constexpr const char* const var_ulong_inneraliasboundedstringhelper_map =
        "var_map_ulong_inneraliasboundedstringhelper";
constexpr const char* const var_ulong_inneraliasboundedwstringhelper_map =
        "var_map_ulong_inneraliasboundedwstringhelper";
constexpr const char* const var_ulong_innerenumhelper_map = "var_map_ulong_innerenumhelper";
constexpr const char* const var_ulong_innerbitmaskhelper_map = "var_map_ulong_innerbitmaskhelper";
constexpr const char* const var_ulong_inneraliashelper_map = "var_map_ulong_inneraliashelper";
constexpr const char* const var_ulong_inneraliasarrayhelper_map = "var_map_ulong_inneraliasarrayhelper";
constexpr const char* const var_ulong_inneraliassequencehelper_map = "var_map_ulong_inneraliassequencehelper";
constexpr const char* const var_ulong_inneraliasmaphelper_map = "var_map_ulong_inneraliasmaphelper";
constexpr const char* const var_ulong_innerunionhelper_map = "var_map_ulong_innerunionhelper";
constexpr const char* const var_ulong_innerstructurehelper_map = "var_map_ulong_innerstructurehelper";
constexpr const char* const var_ulong_innerbitsethelper_map = "var_map_ulong_innerbitsethelper";
constexpr const char* const var_longlong_short_map = "var_map_longlong_short";
constexpr const char* const var_longlong_ushort_map = "var_map_longlong_ushort";
constexpr const char* const var_longlong_long_map = "var_map_longlong_long";
constexpr const char* const var_longlong_ulong_map = "var_map_longlong_ulong";
constexpr const char* const var_longlong_longlong_map = "var_map_longlong_longlong";
constexpr const char* const var_longlong_ulonglong_map = "var_map_longlong_ulonglong";
constexpr const char* const var_longlong_float_map = "var_map_longlong_float";
constexpr const char* const var_longlong_double_map = "var_map_longlong_double";
constexpr const char* const var_longlong_longdouble_map = "var_map_longlong_longdouble";
constexpr const char* const var_longlong_boolean_map = "var_map_longlong_boolean";
constexpr const char* const var_longlong_octet_map = "var_map_longlong_octet";
constexpr const char* const var_longlong_char_map = "var_map_longlong_char";
constexpr const char* const var_longlong_wchar_map = "var_map_longlong_wchar";
constexpr const char* const var_longlong_string_map = "var_map_longlong_string";
constexpr const char* const var_longlong_wstring_map = "var_map_longlong_wstring";
constexpr const char* const var_longlong_inneraliasboundedstringhelper_map =
        "var_map_longlong_inneraliasboundedstringhelper";
constexpr const char* const var_longlong_inneraliasboundedwstringhelper_map =
        "var_map_longlong_inneraliasboundedwstringhelper";
constexpr const char* const var_longlong_innerenumhelper_map = "var_map_longlong_innerenumhelper";
constexpr const char* const var_longlong_innerbitmaskhelper_map = "var_map_longlong_innerbitmaskhelper";
constexpr const char* const var_longlong_inneraliashelper_map = "var_map_longlong_inneraliashelper";
constexpr const char* const var_longlong_inneraliasarrayhelper_map = "var_map_longlong_inneraliasarrayhelper";
constexpr const char* const var_longlong_inneraliassequencehelper_map = "var_map_longlong_inneraliassequencehelper";
constexpr const char* const var_longlong_inneraliasmaphelper_map = "var_map_longlong_inneraliasmaphelper";
constexpr const char* const var_longlong_innerunionhelper_map = "var_map_longlong_innerunionhelper";
constexpr const char* const var_longlong_innerstructurehelper_map = "var_map_longlong_innerstructurehelper";
constexpr const char* const var_longlong_innerbitsethelper_map = "var_map_longlong_innerbitsethelper";
constexpr const char* const var_ulonglong_short_map = "var_map_u_long_long_short";
constexpr const char* const var_ulonglong_ushort_map = "var_map_u_long_long_u_short";
constexpr const char* const var_ulonglong_long_map = "var_map_u_long_long_long";
constexpr const char* const var_ulonglong_ulong_map = "var_map_u_long_long_u_long";
constexpr const char* const var_ulonglong_longlong_map = "var_map_u_long_long_long_long";
constexpr const char* const var_ulonglong_ulonglong_map = "var_map_u_long_long_u_long_long";
constexpr const char* const var_ulonglong_float_map = "var_map_u_long_long_float";
constexpr const char* const var_ulonglong_double_map = "var_map_u_long_long_double";
constexpr const char* const var_ulonglong_longdouble_map = "var_map_u_long_long_long_double";
constexpr const char* const var_ulonglong_boolean_map = "var_map_u_long_long_boolean";
constexpr const char* const var_ulonglong_octet_map = "var_map_u_long_long_octet";
constexpr const char* const var_ulonglong_char_map = "var_map_u_long_long_char";
constexpr const char* const var_ulonglong_wchar_map = "var_map_u_long_long_wchar";
constexpr const char* const var_ulonglong_string_map = "var_map_u_long_long_string";
constexpr const char* const var_ulonglong_wstring_map = "var_map_u_long_long_wstring";
constexpr const char* const var_ulonglong_inneraliasboundedstringhelper_map =
        "var_map_u_long_long_inner_alias_bounded_string_helper";
constexpr const char* const var_ulonglong_inneraliasboundedwstringhelper_map =
        "var_map_u_long_long_inner_alias_bounded_wstring_helper";
constexpr const char* const var_ulonglong_innerenumhelper_map = "var_map_u_long_long_inner_enum_helper";
constexpr const char* const var_ulonglong_innerbitmaskhelper_map = "var_map_u_long_long_inner_bit_mask_helper";
constexpr const char* const var_ulonglong_inneraliashelper_map = "var_map_u_long_long_inner_alias_helper";
constexpr const char* const var_ulonglong_inneraliasarrayhelper_map = "var_map_u_long_long_inner_alias_array_helper";
constexpr const char* const var_ulonglong_inneraliassequencehelper_map =
        "var_map_u_long_long_inner_alias_sequence_helper";
constexpr const char* const var_ulonglong_inneraliasmaphelper_map = "var_map_u_long_long_inner_alias_map_helper";
constexpr const char* const var_ulonglong_innerunionhelper_map = "var_map_u_long_long_inner_union_helper";
constexpr const char* const var_ulonglong_innerstructurehelper_map = "var_map_u_long_long_inner_structure_helper";
constexpr const char* const var_ulonglong_innerbitsethelper_map = "var_map_u_long_long_inner_bitset_helper";
constexpr const char* const var_string_short_map = "var_map_string_short";
constexpr const char* const var_string_ushort_map = "var_map_string_ushort";
constexpr const char* const var_string_long_map = "var_map_string_long";
constexpr const char* const var_string_ulong_map = "var_map_string_ulong";
constexpr const char* const var_string_longlong_map = "var_map_string_longlong";
constexpr const char* const var_string_ulonglong_map = "var_map_string_ulonglong";
constexpr const char* const var_string_float_map = "var_map_string_float";
constexpr const char* const var_string_double_map = "var_map_string_double";
constexpr const char* const var_string_longdouble_map = "var_map_string_longdouble";
constexpr const char* const var_string_boolean_map = "var_map_string_boolean";
constexpr const char* const var_string_octet_map = "var_map_string_octet";
constexpr const char* const var_string_char_map = "var_map_string_char";
constexpr const char* const var_string_wchar_map = "var_map_string_wchar";
constexpr const char* const var_string_string_map = "var_map_string_string";
constexpr const char* const var_string_wstring_map = "var_map_string_wstring";
constexpr const char* const var_string_inneraliasboundedstringhelper_map =
        "var_map_string_inneraliasboundedstringhelper";
constexpr const char* const var_string_inneraliasboundedwstringhelper_map =
        "var_map_string_inneraliasboundedwstringhelper";
constexpr const char* const var_string_innerenumhelper_map = "var_map_string_innerenumhelper";
constexpr const char* const var_string_innerbitmaskhelper_map = "var_map_string_innerbitmaskhelper";
constexpr const char* const var_string_inneraliashelper_map = "var_map_string_inneraliashelper";
constexpr const char* const var_string_inneraliasarrayhelper_map = "var_map_string_inneraliasarrayhelper";
constexpr const char* const var_string_inneraliassequencehelper_map = "var_map_string_inneraliassequencehelper";
constexpr const char* const var_string_inneraliasmaphelper_map = "var_map_string_inneraliasmaphelper";
constexpr const char* const var_string_innerunionhelper_map = "var_map_string_innerunionhelper";
constexpr const char* const var_string_innerstructurehelper_map = "var_map_string_innerstructurehelper";
constexpr const char* const var_string_innerbitsethelper_map = "var_map_string_innerbitsethelper";
constexpr const char* const var_inneraliasboundedstringhelper_short_map = "var_map_inneraliasboundedstringhelper_short";
constexpr const char* const var_inneraliasboundedstringhelper_ushort_map =
        "var_map_inneraliasboundedstringhelper_ushort";
constexpr const char* const var_inneraliasboundedstringhelper_long_map = "var_map_inneraliasboundedstringhelper_long";
constexpr const char* const var_inneraliasboundedstringhelper_ulong_map = "var_map_inneraliasboundedstringhelper_ulong";
constexpr const char* const var_inneraliasboundedstringhelper_longlong_map =
        "var_map_inneraliasboundedstringhelper_longlong";
constexpr const char* const var_inneraliasboundedstringhelper_ulonglong_map =
        "var_map_inneraliasboundedstringhelper_ulonglong";
constexpr const char* const var_inneraliasboundedstringhelper_float_map = "var_map_inneraliasboundedstringhelper_float";
constexpr const char* const var_inneraliasboundedstringhelper_double_map =
        "var_map_inneraliasboundedstringhelper_double";
constexpr const char* const var_inneraliasboundedstringhelper_longdouble_map =
        "var_map_inneraliasboundedstringhelper_longdouble";
constexpr const char* const var_inneraliasboundedstringhelper_boolean_map =
        "var_map_inneraliasboundedstringhelper_boolean";
constexpr const char* const var_inneraliasboundedstringhelper_octet_map = "var_map_inneraliasboundedstringhelper_octet";
constexpr const char* const var_inneraliasboundedstringhelper_char_map = "var_map_inneraliasboundedstringhelper_char";
constexpr const char* const var_inneraliasboundedstringhelper_wchar_map = "var_map_inneraliasboundedstringhelper_wchar";
constexpr const char* const var_inneraliasboundedstringhelper_string_map =
        "var_map_inneraliasboundedstringhelper_string";
constexpr const char* const var_inneraliasboundedstringhelper_wstring_map =
        "var_map_inneraliasboundedstringhelper_wstring";
constexpr const char* const var_inneraliasboundedstringhelper_inneraliasboundedstringhelper_map =
        "var_map_inneraliasboundedstringhelper_inneraliasboundedstringhelper";
constexpr const char* const var_inneraliasboundedstringhelper_inneraliasboundedwstringhelper_map =
        "var_map_inneraliasboundedstringhelper_inneraliasboundedwstringhelper";
constexpr const char* const var_inneraliasboundedstringhelper_innerenumhelper_map =
        "var_map_inneraliasboundedstringhelper_innerenumhelper";
constexpr const char* const var_inneraliasboundedstringhelper_innerbitmaskhelper_map =
        "var_map_inneraliasboundedstringhelper_innerbitmaskhelper";
constexpr const char* const var_inneraliasboundedstringhelper_inneraliashelper_map =
        "var_map_inneraliasboundedstringhelper_inneraliashelper";
constexpr const char* const var_inneraliasboundedstringhelper_inneraliasarrayhelper_map =
        "var_map_inneraliasboundedstringhelper_inneraliasarrayhelper";
constexpr const char* const var_inneraliasboundedstringhelper_inneraliassequencehelper_map =
        "var_map_inneraliasboundedstringhelper_inneraliassequencehelper";
constexpr const char* const var_inneraliasboundedstringhelper_inneraliasmaphelper_map =
        "var_map_inneraliasboundedstringhelper_inneraliasmaphelper";
constexpr const char* const var_inneraliasboundedstringhelper_innerunionhelper_map =
        "var_map_inneraliasboundedstringhelper_innerunionhelper";
constexpr const char* const var_inneraliasboundedstringhelper_innerstructurehelper_map =
        "var_map_inneraliasboundedstringhelper_innerstructurehelper";
constexpr const char* const var_inneraliasboundedstringhelper_innerbitsethelper_map =
        "var_map_inneraliasboundedstringhelper_innerbitsethelper";
constexpr const char* const var_small_map = "var_small_map";
constexpr const char* const var_unbounded_string_long_bounded_small_map = "var_unbounded_string_long_bounded_small_map";
constexpr const char* const var_long_unbounded_string_bounded_small_map =
        "var_long_unbounded_string_bounded_small_map";
constexpr const char* const var_large_map = "var_large_map";
constexpr const char* const var_unbounded_string_long_bounded_large_map = "var_unbounded_string_long_bounded_large_map";
constexpr const char* const var_long_unbounded_string_bounded_large_map =
        "var_long_unbounded_string_bounded_large_map";

//{{{ Short key
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_short_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_short_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, int16_t> value {
        {std::int16_t(100), std::int16_t(1)},
        {std::int16_t(-100), std::int16_t(2)},
        {std::int16_t(50), std::int16_t(-1)},
        {std::int16_t(-50), std::int16_t(-2)}
    };
    int16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_short_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortShort struct_data;
        TypeSupport static_pubsubType {new MapShortShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_ushort_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_ushort_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, uint16_t> value {
        {std::int16_t(100), std::uint16_t(1)},
        {std::int16_t(-100), std::uint16_t(2)},
        {std::int16_t(50), std::uint16_t(100)},
        {std::int16_t(-50), std::uint16_t(200)}
    };
    uint16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_ushort_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortUShort struct_data;
        TypeSupport static_pubsubType {new MapShortUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_ushort().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_ushort().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_ushort().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_long_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_long_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, int32_t> value {
        {std::int16_t(100), 10000},
        {std::int16_t(-100), 20000},
        {std::int16_t(50), -10000},
        {std::int16_t(-50), -20000}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_long_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortLong struct_data;
        TypeSupport static_pubsubType {new MapShortLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_ulong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_ulong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, uint32_t> value {
        {std::int16_t(100), 10000u},
        {std::int16_t(-100), 20000u},
        {std::int16_t(50), 1000000u},
        {std::int16_t(-50), 2000000u}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_ulong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortULong struct_data;
        TypeSupport static_pubsubType {new MapShortULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_ulong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_ulong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_ulong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_longlong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_longlong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, int64_t> value {
        {std::int16_t(100), 100000000},
        {std::int16_t(-100), 200000000},
        {std::int16_t(50), -100000000},
        {std::int16_t(-50), -200000000}
    };
    int64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_longlong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortLongLong struct_data;
        TypeSupport static_pubsubType {new MapShortLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_longlong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_longlong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_longlong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_ulonglong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_ulonglong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, uint64_t> value {
        {std::int16_t(100), 100000000u},
        {std::int16_t(-100), 200000000u},
        {std::int16_t(50), 10000000000u},
        {std::int16_t(-50), 20000000000u}
    };
    uint64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_ulonglong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortULongLong struct_data;
        TypeSupport static_pubsubType {new MapShortULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_ulonglong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_ulonglong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_ulonglong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_float_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_float_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, float> value {
        {std::int16_t(100), 3.1f},
        {std::int16_t(-100), 100.1f},
        {std::int16_t(50), -100.3f},
        {std::int16_t(-50), -200.3f}
    };
    float test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_float_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortFloat struct_data;
        TypeSupport static_pubsubType {new MapShortFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_float().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_float().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_float().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_double_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_double_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, double> value {
        {std::int16_t(100), 100000000.3},
        {std::int16_t(-100), 200000000.3},
        {std::int16_t(50), -10000000000.5},
        {std::int16_t(-50), -20000000000.5}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_double_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortDouble struct_data;
        TypeSupport static_pubsubType {new MapShortDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_longdouble_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_longdouble_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT128),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, long double> value {
        {std::int16_t(100), 100000000.3},
        {std::int16_t(-100), 200000000.3},
        {std::int16_t(50), -10000000000.5},
        {std::int16_t(-50), -20000000000.5}
    };
    long double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_longdouble_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float128_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float128_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortLongDouble struct_data;
        TypeSupport static_pubsubType {new MapShortLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_longdouble().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_longdouble().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_longdouble().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_boolean_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_boolean_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BOOLEAN),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, bool> value {
        {std::int16_t(100), true},
        {std::int16_t(-100), false},
        {std::int16_t(50), true},
        {std::int16_t(-50), false}
    };
    bool test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_boolean_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_boolean_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_boolean_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortBoolean struct_data;
        TypeSupport static_pubsubType {new MapShortBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_boolean().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_boolean().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_boolean().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_octet_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_octet_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BYTE),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, fastdds::rtps::octet> value {
        {std::int16_t(100), fastdds::rtps::octet(1)},
        {std::int16_t(-100), fastdds::rtps::octet(2)},
        {std::int16_t(50), fastdds::rtps::octet(100)},
        {std::int16_t(-50), fastdds::rtps::octet(200)}
    };
    fastdds::rtps::octet test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_octet_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_byte_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_byte_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortOctet struct_data;
        TypeSupport static_pubsubType {new MapShortOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_octet().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_octet().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_octet().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_char_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_char_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR8),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, char> value {
        {std::int16_t(100), 'a'},
        {std::int16_t(-100), 'A'},
        {std::int16_t(50), '{'},
        {std::int16_t(-50), '}'}
    };
    char test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_char_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char8_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char8_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortChar struct_data;
        TypeSupport static_pubsubType {new MapShortCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_char().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_char().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_char().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_wchar_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_wchar_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, wchar_t> value {
        {std::int16_t(100), L'a'},
        {std::int16_t(-100), L'A'},
        {std::int16_t(50), L'{'},
        {std::int16_t(-50), L'}'}
    };
    wchar_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_wchar_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortWChar struct_data;
        TypeSupport static_pubsubType {new MapShortWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_wchar().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_wchar().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_wchar().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_string_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_string_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int16_t, std::string> value {
        {std::int16_t(-100), "we"},
        {std::int16_t(-50), "are"},
        {std::int16_t(50), "testing"},
        {std::int16_t(600), "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_string_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortString struct_data;
        TypeSupport static_pubsubType {new MapShortStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_string().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_string().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_string().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_wstring_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_wstring_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                DynamicTypeBuilderFactory:: get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int16_t, std::wstring> value {
        {std::int16_t(-100), L"we"},
        {std::int16_t(-50), L"are"},
        {std::int16_t(50), L"testing"},
        {std::int16_t(600), L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_wstring_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortWString struct_data;
        TypeSupport static_pubsubType {new MapShortWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_wstring().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_wstring().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_wstring().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerAliasBoundedStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_inneraliasboundedstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_inneraliasboundedstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int16_t, std::string> value {
        {std::int16_t(-100), "we"},
        {std::int16_t(-50), "are"},
        {std::int16_t(50), "testing"},
        {std::int16_t(600), "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_inneraliasboundedstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerAliasBoundedStringHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerAliasBoundedStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_inneraliasboundedstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_inneraliasboundedstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_inneraliasboundedstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second.to_string());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerAliasBoundedStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerAliasBoundedWStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_inneraliasboundedwstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_inneraliasboundedwstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int16_t, std::wstring> value {
        {std::int16_t(-100), L"we"},
        {std::int16_t(-50), L"are"},
        {std::int16_t(50), L"testing"},
        {std::int16_t(600), L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_inneraliasboundedwstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerAliasBoundedWStringHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerAliasBoundedWStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_inneraliasboundedwstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_inneraliasboundedwstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_inneraliasboundedwstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerAliasBoundedWStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_innerenumhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_innerenumhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_enum_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, InnerEnumHelper> value {
        {std::int16_t(-100), InnerEnumHelper::ENUM_VALUE_2},
        {std::int16_t(50), InnerEnumHelper::ENUM_VALUE_1},
        {std::int16_t(600), InnerEnumHelper::ENUM_VALUE_3},
        {std::int16_t(-50), InnerEnumHelper::ENUM_VALUE_2}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_innerenumhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                static_cast<int32_t>(map_element.second)));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, static_cast<InnerEnumHelper>(test_value));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_innerenumhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_innerenumhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_innerenumhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_innerbitmaskhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_innerbitmaskhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_bitmask_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, uint32_t> value {
        {std::int16_t(-100), InnerBitMaskHelperBits::flag0},
        {std::int16_t(50), 0u},
        {std::int16_t(600), InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0},
        {std::int16_t(-50), InnerBitMaskHelperBits::flag4}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_innerbitmaskhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_innerbitmaskhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_innerbitmaskhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_innerbitmaskhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_inneraliashelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_inneraliashelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_alias_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, int32_t> value {
        {std::int16_t(-100), 102},
        {std::int16_t(50), 1},
        {std::int16_t(600), -32},
        {std::int16_t(-50), 43}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_inneraliashelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_inneraliashelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_inneraliashelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_inneraliashelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerAliasArrayHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_inneraliasarrayhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_inneraliasarrayhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, Int16Seq> value {
        {std::int16_t(-100), {102, -102} },
        {std::int16_t(50), {1, -1} },
        {std::int16_t(600), {-32, 32} },
        {std::int16_t(-50), {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_inneraliasarrayhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerAliasArrayHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerAliasArrayHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_inneraliasarrayhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_inneraliasarrayhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_inneraliasarrayhelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerAliasArrayHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerAliasSequenceHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_inneraliassequencehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_inneraliassequencehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, Int16Seq> value {
        {std::int16_t(-100), {102, -102, 304, -304} },
        {std::int16_t(50), {} },
        {std::int16_t(600), {-32} },
        {std::int16_t(-50), {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_inneraliassequencehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerAliasSequenceHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerAliasSequenceHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_inneraliassequencehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_inneraliassequencehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_inneraliassequencehelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerAliasSequenceHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerAliasMapHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_inneraliasmaphelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_inneraliasmaphelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, std::map<int32_t, int32_t>> value {
        {std::int16_t(-100), {
             {1000, -102},
             {2000, -103},
             {-100, 1000}
         }},
        {std::int16_t(50), {
         }},
        {std::int16_t(600), {
             {-1000, 102},
             {-2000, 103}
         }},
        {std::int16_t(-50), {
             {-3000, 302},
         }}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_inneraliasmaphelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->set_int32_value(inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first)),
                    inner_map_element.second));
        }

        map_data->return_loaned_value(inner_map_data);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->get_int32_value(test_value, inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first))));
            EXPECT_EQ(inner_map_element.second, test_value);
        }

        map_data->return_loaned_value(inner_map_data);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerAliasMapHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerAliasMapHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_inneraliasmaphelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_inneraliasmaphelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_inneraliasmaphelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerAliasMapHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_innerunionhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_innerunionhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_union_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, std::pair<const char* const, int16_t>> value {
        {std::int16_t(-100), {union_long_member_name, std::int16_t(32)}},
        {std::int16_t(50), {union_float_member_name, std::int16_t(-12)}},
        {std::int16_t(600), {union_short_member_name, std::int16_t(1)}},
        {std::int16_t(-50), {union_long_member_name, std::int16_t(-32)}}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_innerunionhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->set_int16_value(inner_union->get_member_id_by_name(map_element.second.first),
                map_element.second.second));

        map_data->return_loaned_value(inner_union);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->get_float64_value(test_value,
                inner_union->get_member_id_by_name(map_element.second.first)));
        EXPECT_EQ(map_element.second.second, static_cast<int16_t>(test_value));

        map_data->return_loaned_value(inner_union);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_innerunionhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_innerunionhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_innerunionhelper().end(), it);
            if (union_long_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.longValue()));
            }
            else if (union_float_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.floatValue()));
            }
            else
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.shortValue()));
            }

        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_innerstructurehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_innerstructurehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_struct_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, std::pair<int32_t, float>> value {
        {std::int16_t(-100), {32, 1.0f}},
        {std::int16_t(50), {-12, -1.0f}},
        {std::int16_t(600), {1, -10.1f}},
        {std::int16_t(-50), {-32, 100.3f}}
    };
    int32_t test_value1;
    float test_value2;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_innerstructurehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_int32_value(inner_structure->get_member_id_by_name(struct_long_member_name),
                map_element.second.first));
        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_float32_value(inner_structure->get_member_id_by_name(struct_float_member_name),
                map_element.second.second));

        map_data->return_loaned_value(inner_structure);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_int32_value(test_value1,
                inner_structure->get_member_id_by_name(struct_long_member_name)));
        EXPECT_EQ(map_element.second.first, test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_float32_value(test_value2,
                inner_structure->get_member_id_by_name(struct_float_member_name)));
        EXPECT_EQ(map_element.second.second, test_value2);

        map_data->return_loaned_value(inner_structure);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_innerstructurehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_innerstructurehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_innerstructurehelper().end(), it);
            EXPECT_EQ(map_element.second.first, it->second.field1());
            EXPECT_EQ(map_element.second.second, it->second.field2());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapShortInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_innerbitsethelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_innerbitsethelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                create_inner_bitset_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int16_t, std::tuple<uint8_t, bool, uint16_t, int16_t>> value {
        {std::int16_t(-100), {std::uint8_t(5), true, std::uint16_t(1000), std::int16_t(2000)}},
        {std::int16_t(50), {std::uint8_t(7), false, std::uint16_t(555), std::int16_t(20)}},
        {std::int16_t(600), {std::uint8_t(0), true, std::uint16_t(0), std::int16_t(0)}}
    };
    uint8_t test_value1;
    bool test_value2;
    uint16_t test_value3;
    int16_t test_value4;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_short_innerbitsethelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint8_value(inner_bitset->get_member_id_by_name(bitfield_a),
                std::get<0>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_boolean_value(inner_bitset->get_member_id_by_name(bitfield_b),
                std::get<1>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint16_value(inner_bitset->get_member_id_by_name(bitfield_c),
                std::get<2>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_int16_value(inner_bitset->get_member_id_by_name(bitfield_d),
                std::get<3>(map_element.second)));

        map_data->return_loaned_value(inner_bitset);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint8_value(test_value1,
                inner_bitset->get_member_id_by_name(bitfield_a)));
        EXPECT_EQ(std::get<0>(map_element.second), test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_boolean_value(test_value2,
                inner_bitset->get_member_id_by_name(bitfield_b)));
        EXPECT_EQ(std::get<1>(map_element.second), test_value2);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint16_value(test_value3,
                inner_bitset->get_member_id_by_name(bitfield_c)));
        EXPECT_EQ(std::get<2>(map_element.second), test_value3);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_int16_value(test_value4,
                inner_bitset->get_member_id_by_name(bitfield_d)));
        EXPECT_EQ(std::get<3>(map_element.second), test_value4);

        map_data->return_loaned_value(inner_bitset);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapShortInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new MapShortInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_short_innerbitsethelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_short_innerbitsethelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_short_innerbitsethelper().end(), it);
            EXPECT_EQ(std::get<0>(map_element.second), it->second.a);
            EXPECT_EQ(std::get<1>(map_element.second), it->second.b);
            EXPECT_EQ(std::get<2>(map_element.second), it->second.c);
            EXPECT_EQ(std::get<3>(map_element.second), it->second.d);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapShortInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}
//}}}

//{{{ UShort key
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_short_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_short_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, int16_t> value {
        {std::uint16_t(100), std::int16_t(1)},
        {std::uint16_t(200), std::int16_t(2)},
        {std::uint16_t(50), std::int16_t(-1)},
        {std::uint16_t(70), std::int16_t(-2)}
    };
    int16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_short_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortShort struct_data;
        TypeSupport static_pubsubType {new MapUShortShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_ushort_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_ushort_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, uint16_t> value {
        {std::uint16_t(100), std::uint16_t(1)},
        {std::uint16_t(200), std::uint16_t(2)},
        {std::uint16_t(50), std::uint16_t(100)},
        {std::uint16_t(70), std::uint16_t(200)}
    };
    uint16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_ushort_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortUShort struct_data;
        TypeSupport static_pubsubType {new MapUShortUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_ushort().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_ushort().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_ushort().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_long_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_long_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, int32_t> value {
        {std::uint16_t(100), 10000},
        {std::uint16_t(200), 20000},
        {std::uint16_t(50), -10000},
        {std::uint16_t(70), -20000}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_long_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortLong struct_data;
        TypeSupport static_pubsubType {new MapUShortLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_ulong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_ulong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, uint32_t> value {
        {std::uint16_t(100), 10000u},
        {std::uint16_t(200), 20000u},
        {std::uint16_t(50), 1000000u},
        {std::uint16_t(70), 2000000u}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_ulong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortULong struct_data;
        TypeSupport static_pubsubType {new MapUShortULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_ulong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_ulong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_ulong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_longlong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_longlong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, int64_t> value {
        {std::uint16_t(100), 100000000},
        {std::uint16_t(200), 200000000},
        {std::uint16_t(50), -100000000},
        {std::uint16_t(70), -200000000}
    };
    int64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_longlong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortLongLong struct_data;
        TypeSupport static_pubsubType {new MapUShortLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_longlong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_longlong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_longlong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_ulonglong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_ulonglong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, uint64_t> value {
        {std::uint16_t(100), 100000000u},
        {std::uint16_t(200), 200000000u},
        {std::uint16_t(50), 10000000000u},
        {std::uint16_t(70), 20000000000u}
    };
    uint64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_ulonglong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortULongLong struct_data;
        TypeSupport static_pubsubType {new MapUShortULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_ulonglong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_ulonglong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_ulonglong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_float_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_float_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, float> value {
        {std::uint16_t(100), 3.1f},
        {std::uint16_t(200), 100.1f},
        {std::uint16_t(50), -100.3f},
        {std::uint16_t(70), -200.3f}
    };
    float test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_float_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortFloat struct_data;
        TypeSupport static_pubsubType {new MapUShortFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_float().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_float().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_float().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_double_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_double_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, double> value {
        {std::uint16_t(100), 100000000.3},
        {std::uint16_t(200), 200000000.3},
        {std::uint16_t(50), -10000000000.5},
        {std::uint16_t(70), -20000000000.5}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_double_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortDouble struct_data;
        TypeSupport static_pubsubType {new MapUShortDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_longdouble_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_longdouble_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT128),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, long double> value {
        {std::uint16_t(100), 100000000.3},
        {std::uint16_t(200), 200000000.3},
        {std::uint16_t(50), -10000000000.5},
        {std::uint16_t(70), -20000000000.5}
    };
    long double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_longdouble_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float128_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float128_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortLongDouble struct_data;
        TypeSupport static_pubsubType {new MapUShortLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_longdouble().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_longdouble().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_longdouble().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_boolean_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_boolean_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BOOLEAN),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, bool> value {
        {std::uint16_t(100), true},
        {std::uint16_t(200), false},
        {std::uint16_t(50), true},
        {std::uint16_t(70), false}
    };
    bool test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_boolean_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_boolean_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_boolean_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortBoolean struct_data;
        TypeSupport static_pubsubType {new MapUShortBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_boolean().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_boolean().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_boolean().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_octet_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_octet_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BYTE),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, fastdds::rtps::octet> value {
        {std::uint16_t(100), fastdds::rtps::octet(1)},
        {std::uint16_t(200), fastdds::rtps::octet(2)},
        {std::uint16_t(50), fastdds::rtps::octet(100)},
        {std::uint16_t(70), fastdds::rtps::octet(200)}
    };
    fastdds::rtps::octet test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_octet_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_byte_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_byte_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortOctet struct_data;
        TypeSupport static_pubsubType {new MapUShortOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_octet().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_octet().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_octet().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_char_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_char_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR8),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, char> value {
        {std::uint16_t(100), 'a'},
        {std::uint16_t(200), 'A'},
        {std::uint16_t(50), '{'},
        {std::uint16_t(70), '}'}
    };
    char test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_char_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char8_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char8_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortChar struct_data;
        TypeSupport static_pubsubType {new MapUShortCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_char().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_char().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_char().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_wchar_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_wchar_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, wchar_t> value {
        {std::uint16_t(100), L'a'},
        {std::uint16_t(200), L'A'},
        {std::uint16_t(50), L'{'},
        {std::uint16_t(70), L'}'}
    };
    wchar_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_wchar_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortWChar struct_data;
        TypeSupport static_pubsubType {new MapUShortWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_wchar().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_wchar().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_wchar().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_string_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_string_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint16_t, std::string> value {
        {std::uint16_t(10), "we"},
        {std::uint16_t(40), "are"},
        {std::uint16_t(50), "testing"},
        {std::uint16_t(600), "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_string_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortString struct_data;
        TypeSupport static_pubsubType {new MapUShortStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_string().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_string().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_string().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_wstring_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_wstring_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                DynamicTypeBuilderFactory:: get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint16_t, std::wstring> value {
        {std::uint16_t(1), L"we"},
        {std::uint16_t(4), L"are"},
        {std::uint16_t(5), L"testing"},
        {std::uint16_t(60), L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_wstring_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortWString struct_data;
        TypeSupport static_pubsubType {new MapUShortWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_wstring().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_wstring().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_wstring().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerAliasBoundedStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_inneraliasboundedstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_inneraliasboundedstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint16_t, std::string> value {
        {std::uint16_t(10), "we"},
        {std::uint16_t(40), "are"},
        {std::uint16_t(50), "testing"},
        {std::uint16_t(600), "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_inneraliasboundedstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerAliasBoundedStringHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerAliasBoundedStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_inneraliasboundedstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_inneraliasboundedstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_inneraliasboundedstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second.to_string());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerAliasBoundedStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerAliasBoundedWStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_inneraliasboundedwstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_inneraliasboundedwstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint16_t, std::wstring> value {
        {std::uint16_t(10), L"we"},
        {std::uint16_t(40), L"are"},
        {std::uint16_t(50), L"testing"},
        {std::uint16_t(600), L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_inneraliasboundedwstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerAliasBoundedWStringHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerAliasBoundedWStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_inneraliasboundedwstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_inneraliasboundedwstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_inneraliasboundedwstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerAliasBoundedWStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_innerenumhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_innerenumhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_enum_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, InnerEnumHelper> value {
        {std::uint16_t(100), InnerEnumHelper::ENUM_VALUE_2},
        {std::uint16_t(50), InnerEnumHelper::ENUM_VALUE_1},
        {std::uint16_t(600), InnerEnumHelper::ENUM_VALUE_3},
        {std::uint16_t(70), InnerEnumHelper::ENUM_VALUE_2}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_innerenumhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                static_cast<int32_t>(map_element.second)));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, static_cast<InnerEnumHelper>(test_value));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_innerenumhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_innerenumhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_innerenumhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }


    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_innerbitmaskhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_innerbitmaskhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_bitmask_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, uint32_t> value {
        {std::uint16_t(100), InnerBitMaskHelperBits::flag0},
        {std::uint16_t(50), 0},
        {std::uint16_t(600), InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0},
        {std::uint16_t(70), InnerBitMaskHelperBits::flag4}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_innerbitmaskhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_innerbitmaskhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_innerbitmaskhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_innerbitmaskhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_inneraliashelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_inneraliashelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_alias_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, int32_t> value {
        {std::uint16_t(100), 102},
        {std::uint16_t(50), 1},
        {std::uint16_t(600), -32},
        {std::uint16_t(70), 43}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_inneraliashelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_inneraliashelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_inneraliashelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_inneraliashelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerAliasArrayHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_inneraliasarrayhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_inneraliasarrayhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, Int16Seq> value {
        {std::uint16_t(100), {102, -102} },
        {std::uint16_t(50), {1, -1} },
        {std::uint16_t(600), {-32, 32} },
        {std::uint16_t(70), {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_inneraliasarrayhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerAliasArrayHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerAliasArrayHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_inneraliasarrayhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_inneraliasarrayhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_inneraliasarrayhelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerAliasArrayHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerAliasSequenceHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_inneraliassequencehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_inneraliassequencehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, Int16Seq> value {
        {std::uint16_t(10), {102, -102, 304, -304} },
        {std::uint16_t(50), {} },
        {std::uint16_t(60), {-32} },
        {std::uint16_t(70), {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_inneraliassequencehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerAliasSequenceHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerAliasSequenceHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_inneraliassequencehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_inneraliassequencehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_inneraliassequencehelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerAliasSequenceHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerAliasMapHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_inneraliasmaphelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_inneraliasmaphelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, std::map<int32_t, int32_t>> value {
        {std::uint16_t(100), {
             {1000, -102},
             {2000, -103},
             {-100, 1000}
         }},
        {std::uint16_t(50), {
         }},
        {std::uint16_t(600), {
             {-1000, 102},
             {-2000, 103}
         }},
        {std::uint16_t(70), {
             {-3000, 302},
         }}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_inneraliasmaphelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->set_int32_value(inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first)),
                    inner_map_element.second));
        }

        map_data->return_loaned_value(inner_map_data);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->get_int32_value(test_value, inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first))));
            EXPECT_EQ(inner_map_element.second, test_value);
        }

        map_data->return_loaned_value(inner_map_data);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerAliasMapHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerAliasMapHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_inneraliasmaphelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_inneraliasmaphelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_inneraliasmaphelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerAliasMapHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_innerunionhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_innerunionhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_union_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, std::pair<const char* const, int16_t>> value {
        {std::uint16_t(10), {union_long_member_name, std::int16_t(32)}},
        {std::uint16_t(50), {union_float_member_name, std::int16_t(-12)}},
        {std::uint16_t(60), {union_short_member_name, std::int16_t(1)}},
        {std::uint16_t(70), {union_long_member_name, std::int16_t(-32)}}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_innerunionhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->set_int16_value(inner_union->get_member_id_by_name(map_element.second.first),
                map_element.second.second));

        map_data->return_loaned_value(inner_union);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->get_float64_value(test_value,
                inner_union->get_member_id_by_name(map_element.second.first)));
        EXPECT_EQ(map_element.second.second, static_cast<int16_t>(test_value));

        map_data->return_loaned_value(inner_union);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_innerunionhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_innerunionhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_innerunionhelper().end(), it);
            if (union_long_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.longValue()));
            }
            else if (union_float_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.floatValue()));
            }
            else
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.shortValue()));
            }

        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_innerstructurehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_innerstructurehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_struct_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, std::pair<int32_t, float>> value {
        {std::uint16_t(100), {32, 1.0f}},
        {std::uint16_t(50), {-12, -1.0f}},
        {std::uint16_t(600), {1, -10.1f}},
        {std::uint16_t(70), {-32, 100.3f}}
    };
    int32_t test_value1;
    float test_value2;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_innerstructurehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_int32_value(inner_structure->get_member_id_by_name(struct_long_member_name),
                map_element.second.first));
        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_float32_value(inner_structure->get_member_id_by_name(struct_float_member_name),
                map_element.second.second));

        map_data->return_loaned_value(inner_structure);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_int32_value(test_value1,
                inner_structure->get_member_id_by_name(struct_long_member_name)));
        EXPECT_EQ(map_element.second.first, test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_float32_value(test_value2,
                inner_structure->get_member_id_by_name(struct_float_member_name)));
        EXPECT_EQ(map_element.second.second, test_value2);

        map_data->return_loaned_value(inner_structure);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_innerstructurehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_innerstructurehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_innerstructurehelper().end(), it);
            EXPECT_EQ(map_element.second.first, it->second.field1());
            EXPECT_EQ(map_element.second.second, it->second.field2());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapUShortInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_innerbitsethelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_innerbitsethelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                create_inner_bitset_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint16_t, std::tuple<uint8_t, bool, uint16_t, int16_t>> value {
        {std::uint16_t(100), {std::uint8_t(5), true, std::uint16_t(1000), std::int16_t(2000)}},
        {std::uint16_t(50), {std::uint8_t(7), false, std::uint16_t(555), std::int16_t(20)}},
        {std::uint16_t(600), {std::uint8_t(0), true, std::uint16_t(0), std::int16_t(0)}}
    };
    uint8_t test_value1;
    bool test_value2;
    uint16_t test_value3;
    int16_t test_value4;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ushort_innerbitsethelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint8_value(inner_bitset->get_member_id_by_name(bitfield_a),
                std::get<0>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_boolean_value(inner_bitset->get_member_id_by_name(bitfield_b),
                std::get<1>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint16_value(inner_bitset->get_member_id_by_name(bitfield_c),
                std::get<2>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_int16_value(inner_bitset->get_member_id_by_name(bitfield_d),
                std::get<3>(map_element.second)));

        map_data->return_loaned_value(inner_bitset);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint8_value(test_value1,
                inner_bitset->get_member_id_by_name(bitfield_a)));
        EXPECT_EQ(std::get<0>(map_element.second), test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_boolean_value(test_value2,
                inner_bitset->get_member_id_by_name(bitfield_b)));
        EXPECT_EQ(std::get<1>(map_element.second), test_value2);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint16_value(test_value3,
                inner_bitset->get_member_id_by_name(bitfield_c)));
        EXPECT_EQ(std::get<2>(map_element.second), test_value3);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_int16_value(test_value4,
                inner_bitset->get_member_id_by_name(bitfield_d)));
        EXPECT_EQ(std::get<3>(map_element.second), test_value4);

        map_data->return_loaned_value(inner_bitset);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapUShortInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new MapUShortInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ushort_innerbitsethelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ushort_innerbitsethelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ushort_innerbitsethelper().end(), it);
            EXPECT_EQ(std::get<0>(map_element.second), it->second.a);
            EXPECT_EQ(std::get<1>(map_element.second), it->second.b);
            EXPECT_EQ(std::get<2>(map_element.second), it->second.c);
            EXPECT_EQ(std::get<3>(map_element.second), it->second.d);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapUShortInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}
//}}}

//{{{ Long key
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_short_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_short_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, int16_t> value {
        {-100, std::int16_t(1)},
        {200, std::int16_t(2)},
        {-50, std::int16_t(-1)},
        {70, std::int16_t(-2)}
    };
    int16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_short_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongShort struct_data;
        TypeSupport static_pubsubType {new MapLongShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_ushort_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_ushort_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, uint16_t> value {
        {-100, std::uint16_t(1)},
        {200, std::uint16_t(2)},
        {-50, std::uint16_t(100)},
        {70, std::uint16_t(200)}
    };
    uint16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_ushort_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongUShort struct_data;
        TypeSupport static_pubsubType {new MapLongUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_ushort().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_ushort().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_ushort().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_long_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_long_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, int32_t> value {
        {-100, 10000},
        {200, 20000},
        {-50, -10000},
        {70, -20000}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_long_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLong struct_data;
        TypeSupport static_pubsubType {new MapLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_ulong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_ulong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, uint32_t> value {
        {-100, 10000u},
        {200, 20000u},
        {-50, 1000000u},
        {70, 2000000u}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_ulong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongULong struct_data;
        TypeSupport static_pubsubType {new MapLongULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_ulong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_ulong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_ulong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongKeyLongLongValue)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_longlong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_longlong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, int64_t> value {
        {-100, 100000000},
        {200, 200000000},
        {-50, -100000000},
        {70, -200000000}
    };
    int64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_longlong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongKeyLongLongValue struct_data;
        TypeSupport static_pubsubType {new MapLongKeyLongLongValuePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_longlong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_longlong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_longlong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongKeyLongLongValue_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_ulonglong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_ulonglong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, uint64_t> value {
        {-100, 100000000u},
        {200, 200000000u},
        {-50, 10000000000u},
        {70, 20000000000u}
    };
    uint64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_ulonglong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongULongLong struct_data;
        TypeSupport static_pubsubType {new MapLongULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_ulonglong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_ulonglong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_ulonglong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_float_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_float_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, float> value {
        {-100, 3.1f},
        {200, 100.1f},
        {-50, -100.3f},
        {70, -200.3f}
    };
    float test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_float_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongFloat struct_data;
        TypeSupport static_pubsubType {new MapLongFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_float().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_float().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_float().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_double_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_double_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, double> value {
        {-100, 100000000.3},
        {200, 200000000.3},
        {-50, -10000000000.5},
        {70, -20000000000.5}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_double_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongDouble struct_data;
        TypeSupport static_pubsubType {new MapLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongKeyLongDoubleValue)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_longdouble_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_longdouble_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT128),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, long double> value {
        {-100, 100000000.3},
        {200, 200000000.3},
        {-50, -10000000000.5},
        {70, -20000000000.5}
    };
    long double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_longdouble_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float128_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float128_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongKeyLongDoubleValue struct_data;
        TypeSupport static_pubsubType {new MapLongKeyLongDoubleValuePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_longdouble().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_longdouble().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_longdouble().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongKeyLongDoubleValue_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_boolean_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_boolean_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BOOLEAN),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, bool> value {
        {-100, true},
        {200, false},
        {-50, true},
        {70, false}
    };
    bool test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_boolean_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_boolean_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_boolean_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongBoolean struct_data;
        TypeSupport static_pubsubType {new MapLongBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_boolean().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_boolean().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_boolean().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_octet_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_octet_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BYTE),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, fastdds::rtps::octet> value {
        {-100, fastdds::rtps::octet(1)},
        {200, fastdds::rtps::octet(2)},
        {-50, fastdds::rtps::octet(100)},
        {70, fastdds::rtps::octet(200)}
    };
    fastdds::rtps::octet test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_octet_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_byte_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_byte_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongOctet struct_data;
        TypeSupport static_pubsubType {new MapLongOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_octet().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_octet().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_octet().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_char_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_char_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR8),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, char> value {
        {-100, 'a'},
        {200, 'A'},
        {-50, '{'},
        {70, '}'}
    };
    char test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_char_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char8_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char8_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongChar struct_data;
        TypeSupport static_pubsubType {new MapLongCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_char().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_char().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_char().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_wchar_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_wchar_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, wchar_t> value {
        {-100, L'a'},
        {200, L'A'},
        {-50, L'{'},
        {70, L'}'}
    };
    wchar_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_wchar_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongWChar struct_data;
        TypeSupport static_pubsubType {new MapLongWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_wchar().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_wchar().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_wchar().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_string_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_string_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int32_t, std::string> value {
        {-10, "we"},
        {-40, "are"},
        {50, "testing"},
        {600, "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_string_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongString struct_data;
        TypeSupport static_pubsubType {new MapLongStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_string().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_string().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_string().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_wstring_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_wstring_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int32_t, std::wstring> value {
        {-10, L"we"},
        {-40, L"are"},
        {50, L"testing"},
        {600, L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_wstring_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongWString struct_data;
        TypeSupport static_pubsubType {new MapLongWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_wstring().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_wstring().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_wstring().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerAliasBoundedStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_inneraliasboundedstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_inneraliasboundedstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int32_t, std::string> value {
        {-10, "we"},
        {-40, "are"},
        {50, "testing"},
        {600, "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_inneraliasboundedstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerAliasBoundedStringHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerAliasBoundedStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_inneraliasboundedstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_inneraliasboundedstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_inneraliasboundedstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second.to_string());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerAliasBoundedStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerAliasBoundedWStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_inneraliasboundedwstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_inneraliasboundedwstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int32_t, std::wstring> value {
        {-10, L"we"},
        {-40, L"are"},
        {50, L"testing"},
        {600, L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_inneraliasboundedwstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerAliasBoundedWStringHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerAliasBoundedWStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_inneraliasboundedwstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_inneraliasboundedwstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_inneraliasboundedwstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerAliasBoundedWStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_innerenumhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_innerenumhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_enum_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, InnerEnumHelper> value {
        {-100, InnerEnumHelper::ENUM_VALUE_2},
        {50, InnerEnumHelper::ENUM_VALUE_1},
        {-600, InnerEnumHelper::ENUM_VALUE_3},
        {70, InnerEnumHelper::ENUM_VALUE_2}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_innerenumhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                static_cast<int32_t>(map_element.second)));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, static_cast<InnerEnumHelper>(test_value));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_innerenumhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_innerenumhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_innerenumhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_innerbitmaskhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_innerbitmaskhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_bitmask_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, uint32_t> value {
        {-100, InnerBitMaskHelperBits::flag0},
        {50, 0},
        {-600, InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0},
        {70, InnerBitMaskHelperBits::flag4}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_innerbitmaskhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_innerbitmaskhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_innerbitmaskhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_innerbitmaskhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_inneraliashelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_inneraliashelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_alias_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, int32_t> value {
        {-100, 102},
        {50, 1},
        {-600, -32},
        {70, 43}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_inneraliashelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_inneraliashelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_inneraliashelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_inneraliashelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerAliasArrayHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_inneraliasarrayhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_inneraliasarrayhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, Int16Seq> value {
        {-100, {102, -102} },
        {50, {1, -1} },
        {-600, {-32, 32} },
        {70, {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_inneraliasarrayhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerAliasArrayHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerAliasArrayHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_inneraliasarrayhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_inneraliasarrayhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_inneraliasarrayhelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerAliasArrayHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerAliasSequenceHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_inneraliassequencehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_inneraliassequencehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, Int16Seq> value {
        {-10, {102, -102, 304, -304} },
        {-50, {} },
        {60, {-32} },
        {70, {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_inneraliassequencehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerAliasSequenceHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerAliasSequenceHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_inneraliassequencehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_inneraliassequencehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_inneraliassequencehelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerAliasSequenceHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerAliasMapHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_inneraliasmaphelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_inneraliasmaphelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, std::map<int32_t, int32_t>> value {
        {-100, {
             {1000, -102},
             {2000, -103},
             {-100, 1000}
         }},
        {-50, {
         }},
        {600, {
             {-1000, 102},
             {-2000, 103}
         }},
        {70, {
             {-3000, 302},
         }}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_inneraliasmaphelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->set_int32_value(inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first)),
                    inner_map_element.second));
        }

        map_data->return_loaned_value(inner_map_data);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->get_int32_value(test_value, inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first))));
            EXPECT_EQ(inner_map_element.second, test_value);
        }

        map_data->return_loaned_value(inner_map_data);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerAliasMapHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerAliasMapHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_inneraliasmaphelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_inneraliasmaphelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_inneraliasmaphelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerAliasMapHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_innerunionhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_innerunionhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_union_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, std::pair<const char* const, int16_t>> value {
        {-10, {union_long_member_name, std::int16_t(32)}},
        {-50, {union_float_member_name, std::int16_t(-12)}},
        {60, {union_short_member_name, std::int16_t(1)}},
        {70, {union_long_member_name, std::int16_t(-32)}}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_innerunionhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->set_int16_value(inner_union->get_member_id_by_name(map_element.second.first),
                map_element.second.second));

        map_data->return_loaned_value(inner_union);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->get_float64_value(test_value,
                inner_union->get_member_id_by_name(map_element.second.first)));
        EXPECT_EQ(map_element.second.second, static_cast<int16_t>(test_value));

        map_data->return_loaned_value(inner_union);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_innerunionhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_innerunionhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_innerunionhelper().end(), it);
            if (union_long_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.longValue()));
            }
            else if (union_float_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.floatValue()));
            }
            else
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.shortValue()));
            }

        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_innerstructurehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_innerstructurehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_struct_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, std::pair<int32_t, float>> value {
        {-100, {32, 1.0f}},
        {50, {-12, -1.0f}},
        {-600, {1, -10.1f}},
        {70, {-32, 100.3f}}
    };
    int32_t test_value1;
    float test_value2;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_innerstructurehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_int32_value(inner_structure->get_member_id_by_name(struct_long_member_name),
                map_element.second.first));
        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_float32_value(inner_structure->get_member_id_by_name(struct_float_member_name),
                map_element.second.second));

        map_data->return_loaned_value(inner_structure);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_int32_value(test_value1,
                inner_structure->get_member_id_by_name(struct_long_member_name)));
        EXPECT_EQ(map_element.second.first, test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_float32_value(test_value2,
                inner_structure->get_member_id_by_name(struct_float_member_name)));
        EXPECT_EQ(map_element.second.second, test_value2);

        map_data->return_loaned_value(inner_structure);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_innerstructurehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_innerstructurehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_innerstructurehelper().end(), it);
            EXPECT_EQ(map_element.second.first, it->second.field1());
            EXPECT_EQ(map_element.second.second, it->second.field2());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_innerbitsethelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_innerbitsethelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                create_inner_bitset_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, std::tuple<uint8_t, bool, uint16_t, int16_t>> value {
        {-100, {std::uint8_t(5), true, std::uint16_t(1000), std::int16_t(2000)}},
        {50, {std::uint8_t(7), false, std::uint16_t(555), std::int16_t(20)}},
        {600, {std::uint8_t(0), true, std::uint16_t(0), std::int16_t(0)}}
    };
    uint8_t test_value1;
    bool test_value2;
    uint16_t test_value3;
    int16_t test_value4;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_long_innerbitsethelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint8_value(inner_bitset->get_member_id_by_name(bitfield_a),
                std::get<0>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_boolean_value(inner_bitset->get_member_id_by_name(bitfield_b),
                std::get<1>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint16_value(inner_bitset->get_member_id_by_name(bitfield_c),
                std::get<2>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_int16_value(inner_bitset->get_member_id_by_name(bitfield_d),
                std::get<3>(map_element.second)));

        map_data->return_loaned_value(inner_bitset);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint8_value(test_value1,
                inner_bitset->get_member_id_by_name(bitfield_a)));
        EXPECT_EQ(std::get<0>(map_element.second), test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_boolean_value(test_value2,
                inner_bitset->get_member_id_by_name(bitfield_b)));
        EXPECT_EQ(std::get<1>(map_element.second), test_value2);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint16_value(test_value3,
                inner_bitset->get_member_id_by_name(bitfield_c)));
        EXPECT_EQ(std::get<2>(map_element.second), test_value3);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_int16_value(test_value4,
                inner_bitset->get_member_id_by_name(bitfield_d)));
        EXPECT_EQ(std::get<3>(map_element.second), test_value4);

        map_data->return_loaned_value(inner_bitset);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new MapLongInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_long_innerbitsethelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_long_innerbitsethelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_long_innerbitsethelper().end(), it);
            EXPECT_EQ(std::get<0>(map_element.second), it->second.a);
            EXPECT_EQ(std::get<1>(map_element.second), it->second.b);
            EXPECT_EQ(std::get<2>(map_element.second), it->second.c);
            EXPECT_EQ(std::get<3>(map_element.second), it->second.d);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}
//}}}

//{{{ ULong key
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_short_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_short_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, int16_t> value {
        {100u, std::int16_t(1)},
        {200u, std::int16_t(2)},
        {50u, std::int16_t(-1)},
        {70u, std::int16_t(-2)}
    };
    int16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_short_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongShort struct_data;
        TypeSupport static_pubsubType {new MapULongShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_ushort_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_ushort_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, uint16_t> value {
        {100u, std::uint16_t(1)},
        {200u, std::uint16_t(2)},
        {50u, std::uint16_t(100)},
        {70u, std::uint16_t(200)}
    };
    uint16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_ushort_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongUShort struct_data;
        TypeSupport static_pubsubType {new MapULongUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_ushort().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_ushort().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_ushort().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_long_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_long_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, int32_t> value {
        {100u, 10000},
        {200u, 20000},
        {50u, -10000},
        {70u, -20000}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_long_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLong struct_data;
        TypeSupport static_pubsubType {new MapULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_ulong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_ulong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, uint32_t> value {
        {100u, 10000u},
        {200u, 20000u},
        {50u, 1000000u},
        {70u, 2000000u}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_ulong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongULong struct_data;
        TypeSupport static_pubsubType {new MapULongULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_ulong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_ulong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_ulong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapKeyULongValueLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_longlong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_longlong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, int64_t> value {
        {100u, 100000000},
        {200u, 200000000},
        {50u, -100000000},
        {70u, -200000000}
    };
    int64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_longlong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapKeyULongValueLongLong struct_data;
        TypeSupport static_pubsubType {new MapKeyULongValueLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_longlong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_longlong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_longlong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapKeyULongValueLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_ulonglong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_ulonglong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, uint64_t> value {
        {100u, 100000000u},
        {200u, 200000000u},
        {50u, 10000000000u},
        {70u, 20000000000u}
    };
    uint64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_ulonglong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongULongLong struct_data;
        TypeSupport static_pubsubType {new MapULongULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_ulonglong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_ulonglong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_ulonglong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_float_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_float_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, float> value {
        {100u, 3.1f},
        {200u, 100.1f},
        {50u, -100.3f},
        {70u, -200.3f}
    };
    float test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_float_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongFloat struct_data;
        TypeSupport static_pubsubType {new MapULongFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_float().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_float().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_float().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_double_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_double_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, double> value {
        {100u, 100000000.3},
        {200u, 200000000.3},
        {50u, -10000000000.5},
        {70u, -20000000000.5}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_double_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongDouble struct_data;
        TypeSupport static_pubsubType {new MapULongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapKeyULongValueLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_longdouble_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_longdouble_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT128),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, long double> value {
        {100u, 100000000.3},
        {200u, 200000000.3},
        {50u, -10000000000.5},
        {70u, -20000000000.5}
    };
    long double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_longdouble_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float128_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float128_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapKeyULongValueLongDouble struct_data;
        TypeSupport static_pubsubType {new MapKeyULongValueLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_longdouble().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_longdouble().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_longdouble().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapKeyULongValueLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_boolean_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_boolean_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BOOLEAN),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, bool> value {
        {100u, true},
        {200u, false},
        {50u, true},
        {70u, false}
    };
    bool test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_boolean_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_boolean_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_boolean_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongBoolean struct_data;
        TypeSupport static_pubsubType {new MapULongBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_boolean().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_boolean().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_boolean().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_octet_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_octet_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BYTE),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, fastdds::rtps::octet> value {
        {100u, fastdds::rtps::octet(1)},
        {200u, fastdds::rtps::octet(2)},
        {50u, fastdds::rtps::octet(100)},
        {70u, fastdds::rtps::octet(200)}
    };
    fastdds::rtps::octet test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_octet_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_byte_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_byte_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongOctet struct_data;
        TypeSupport static_pubsubType {new MapULongOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_octet().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_octet().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_octet().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_char_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_char_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR8),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, char> value {
        {100u, 'a'},
        {200u, 'A'},
        {50u, '{'},
        {70u, '}'}
    };
    char test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_char_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char8_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char8_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongChar struct_data;
        TypeSupport static_pubsubType {new MapULongCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_char().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_char().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_char().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_wchar_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_wchar_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, wchar_t> value {
        {100u, L'a'},
        {200u, L'A'},
        {50u, L'{'},
        {70u, L'}'}
    };
    wchar_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_wchar_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongWChar struct_data;
        TypeSupport static_pubsubType {new MapULongWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_wchar().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_wchar().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_wchar().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_string_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_string_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint32_t, std::string> value {
        {10, "we"},
        {40, "are"},
        {50, "testing"},
        {600, "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_string_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongString struct_data;
        TypeSupport static_pubsubType {new MapULongStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_string().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_string().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_string().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_wstring_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_wstring_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                DynamicTypeBuilderFactory:: get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint32_t, std::wstring> value {
        {10, L"we"},
        {40, L"are"},
        {50, L"testing"},
        {600, L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_wstring_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongWString struct_data;
        TypeSupport static_pubsubType {new MapULongWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_wstring().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_wstring().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_wstring().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerAliasBoundedStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_inneraliasboundedstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_inneraliasboundedstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint32_t, std::string> value {
        {10, "we"},
        {40, "are"},
        {50, "testing"},
        {600, "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_inneraliasboundedstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerAliasBoundedStringHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerAliasBoundedStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_inneraliasboundedstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_inneraliasboundedstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_inneraliasboundedstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second.to_string());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerAliasBoundedStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerAliasBoundedWStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_inneraliasboundedwstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_inneraliasboundedwstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint32_t, std::wstring> value {
        {10, L"we"},
        {40, L"are"},
        {50, L"testing"},
        {600, L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_inneraliasboundedwstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerAliasBoundedWStringHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerAliasBoundedWStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_inneraliasboundedwstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_inneraliasboundedwstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_inneraliasboundedwstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerAliasBoundedWStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_innerenumhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_innerenumhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_enum_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, InnerEnumHelper> value {
        {100u, InnerEnumHelper::ENUM_VALUE_2},
        {50u, InnerEnumHelper::ENUM_VALUE_1},
        {600u, InnerEnumHelper::ENUM_VALUE_3},
        {70u, InnerEnumHelper::ENUM_VALUE_2}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_innerenumhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                static_cast<int32_t>(map_element.second)));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, static_cast<InnerEnumHelper>(test_value));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_innerenumhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_innerenumhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_innerenumhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_innerbitmaskhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_innerbitmaskhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_bitmask_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, uint32_t> value {
        {100u, InnerBitMaskHelperBits::flag0},
        {50u, 0},
        {600u, InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0},
        {70u, InnerBitMaskHelperBits::flag4}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_innerbitmaskhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_innerbitmaskhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_innerbitmaskhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_innerbitmaskhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_inneraliashelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_inneraliashelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_alias_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, int32_t> value {
        {100u, 102},
        {50u, 1},
        {600u, -32},
        {70u, 43}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_inneraliashelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_inneraliashelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_inneraliashelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_inneraliashelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerAliasArrayHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_inneraliasarrayhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_inneraliasarrayhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, Int16Seq> value {
        {100u, {102, -102} },
        {50u, {1, -1} },
        {600u, {-32, 32} },
        {70u, {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_inneraliasarrayhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerAliasArrayHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerAliasArrayHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_inneraliasarrayhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_inneraliasarrayhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_inneraliasarrayhelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerAliasArrayHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerAliasSequenceHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_inneraliassequencehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_inneraliassequencehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, Int16Seq> value {
        {10u, {102, -102, 304, -304} },
        {50u, {} },
        {60u, {-32} },
        {70u, {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_inneraliassequencehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerAliasSequenceHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerAliasSequenceHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_inneraliassequencehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_inneraliassequencehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_inneraliassequencehelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerAliasSequenceHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerAliasMapHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_inneraliasmaphelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_inneraliasmaphelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, std::map<int32_t, int32_t>> value {
        {100u, {
             {1000, -102},
             {2000, -103},
             {-100, 1000}
         }},
        {50u, {
         }},
        {600u, {
             {-1000, 102},
             {-2000, 103}
         }},
        {70u, {
             {-3000, 302},
         }}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_inneraliasmaphelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->set_int32_value(inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first)),
                    inner_map_element.second));
        }

        map_data->return_loaned_value(inner_map_data);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->get_int32_value(test_value, inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first))));
            EXPECT_EQ(inner_map_element.second, test_value);
        }

        map_data->return_loaned_value(inner_map_data);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerAliasMapHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerAliasMapHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_inneraliasmaphelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_inneraliasmaphelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_inneraliasmaphelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerAliasMapHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_innerunionhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_innerunionhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_union_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, std::pair<const char* const, int16_t>> value {
        {10u, {union_long_member_name, std::int16_t(32)}},
        {50u, {union_float_member_name, std::int16_t(-12)}},
        {60u, {union_short_member_name, std::int16_t(1)}},
        {70u, {union_long_member_name, std::int16_t(-32)}}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_innerunionhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->set_int16_value(inner_union->get_member_id_by_name(map_element.second.first),
                map_element.second.second));

        map_data->return_loaned_value(inner_union);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->get_float64_value(test_value,
                inner_union->get_member_id_by_name(map_element.second.first)));
        EXPECT_EQ(map_element.second.second, static_cast<int16_t>(test_value));

        map_data->return_loaned_value(inner_union);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_innerunionhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_innerunionhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_innerunionhelper().end(), it);
            if (union_long_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.longValue()));
            }
            else if (union_float_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.floatValue()));
            }
            else
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.shortValue()));
            }

        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_innerstructurehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_innerstructurehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_struct_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, std::pair<int32_t, float>> value {
        {100u, {32, 1.0f}},
        {50u, {-12, -1.0f}},
        {600u, {1, -10.1f}},
        {70u, {-32, 100.3f}}
    };
    int32_t test_value1;
    float test_value2;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_innerstructurehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_int32_value(inner_structure->get_member_id_by_name(struct_long_member_name),
                map_element.second.first));
        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_float32_value(inner_structure->get_member_id_by_name(struct_float_member_name),
                map_element.second.second));

        map_data->return_loaned_value(inner_structure);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_int32_value(test_value1,
                inner_structure->get_member_id_by_name(struct_long_member_name)));
        EXPECT_EQ(map_element.second.first, test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_float32_value(test_value2,
                inner_structure->get_member_id_by_name(struct_float_member_name)));
        EXPECT_EQ(map_element.second.second, test_value2);

        map_data->return_loaned_value(inner_structure);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_innerstructurehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_innerstructurehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_innerstructurehelper().end(), it);
            EXPECT_EQ(map_element.second.first, it->second.field1());
            EXPECT_EQ(map_element.second.second, it->second.field2());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_innerbitsethelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_innerbitsethelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                create_inner_bitset_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint32_t, std::tuple<uint8_t, bool, uint16_t, int16_t>> value {
        {100u, {std::uint8_t(5), true, std::uint16_t(1000), std::int16_t(2000)}},
        {50u, {std::uint8_t(7), false, std::uint16_t(555), std::int16_t(20)}},
        {600u, {std::uint8_t(0), true, std::uint16_t(0), std::int16_t(0)}}
    };
    uint8_t test_value1;
    bool test_value2;
    uint16_t test_value3;
    int16_t test_value4;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulong_innerbitsethelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint8_value(inner_bitset->get_member_id_by_name(bitfield_a),
                std::get<0>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_boolean_value(inner_bitset->get_member_id_by_name(bitfield_b),
                std::get<1>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint16_value(inner_bitset->get_member_id_by_name(bitfield_c),
                std::get<2>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_int16_value(inner_bitset->get_member_id_by_name(bitfield_d),
                std::get<3>(map_element.second)));

        map_data->return_loaned_value(inner_bitset);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint8_value(test_value1,
                inner_bitset->get_member_id_by_name(bitfield_a)));
        EXPECT_EQ(std::get<0>(map_element.second), test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_boolean_value(test_value2,
                inner_bitset->get_member_id_by_name(bitfield_b)));
        EXPECT_EQ(std::get<1>(map_element.second), test_value2);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint16_value(test_value3,
                inner_bitset->get_member_id_by_name(bitfield_c)));
        EXPECT_EQ(std::get<2>(map_element.second), test_value3);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_int16_value(test_value4,
                inner_bitset->get_member_id_by_name(bitfield_d)));
        EXPECT_EQ(std::get<3>(map_element.second), test_value4);

        map_data->return_loaned_value(inner_bitset);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new MapULongInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_ulong_innerbitsethelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_ulong_innerbitsethelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_ulong_innerbitsethelper().end(), it);
            EXPECT_EQ(std::get<0>(map_element.second), it->second.a);
            EXPECT_EQ(std::get<1>(map_element.second), it->second.b);
            EXPECT_EQ(std::get<2>(map_element.second), it->second.c);
            EXPECT_EQ(std::get<3>(map_element.second), it->second.d);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}
//}}}

//{{{ LongLong key
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_short_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_short_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, int16_t> value {
        {-100, std::int16_t(1)},
        {200, std::int16_t(2)},
        {-50, std::int16_t(-1)},
        {70, std::int16_t(-2)}
    };
    int16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_short_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongShort struct_data;
        TypeSupport static_pubsubType {new MapLongLongShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_ushort_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_ushort_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, uint16_t> value {
        {-100, std::uint16_t(1)},
        {200, std::uint16_t(2)},
        {-50, std::uint16_t(100)},
        {70, std::uint16_t(200)}
    };
    uint16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_ushort_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongUShort struct_data;
        TypeSupport static_pubsubType {new MapLongLongUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_ushort().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_ushort().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_ushort().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongKeyLongValue)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_long_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_long_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, int32_t> value {
        {-100, 10000},
        {200, 20000},
        {-50, -10000},
        {70, -20000}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_long_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongKeyLongValue struct_data;
        TypeSupport static_pubsubType {new MapLongLongKeyLongValuePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongKeyLongValue_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_ulong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_ulong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, uint32_t> value {
        {-100, 10000u},
        {200, 20000u},
        {-50, 1000000u},
        {70, 2000000u}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_ulong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongULong struct_data;
        TypeSupport static_pubsubType {new MapLongLongULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_ulong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_ulong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_ulong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_longlong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_longlong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, int64_t> value {
        {-100, 100000000},
        {200, 200000000},
        {-50, -100000000},
        {70, -200000000}
    };
    int64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_longlong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongLongLong struct_data;
        TypeSupport static_pubsubType {new MapLongLongLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_longlong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_longlong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_longlong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_ulonglong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_ulonglong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, uint64_t> value {
        {-100, 100000000u},
        {200, 200000000u},
        {-50, 10000000000u},
        {70, 20000000000u}
    };
    uint64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_ulonglong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongULongLong struct_data;
        TypeSupport static_pubsubType {new MapLongLongULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_ulonglong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_ulonglong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_ulonglong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_float_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_float_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, float> value {
        {-100, 3.1f},
        {200, 100.1f},
        {-50, -100.3f},
        {70, -200.3f}
    };
    float test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_float_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongFloat struct_data;
        TypeSupport static_pubsubType {new MapLongLongFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_float().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_float().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_float().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongKeyDoubleValue)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_double_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_double_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, double> value {
        {-100, 100000000.3},
        {200, 200000000.3},
        {-50, -10000000000.5},
        {70, -20000000000.5}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_double_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongKeyDoubleValue struct_data;
        TypeSupport static_pubsubType {new MapLongLongKeyDoubleValuePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongKeyDoubleValue_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_longdouble_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_longdouble_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT128),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, long double> value {
        {-100, 100000000.3},
        {200, 200000000.3},
        {-50, -10000000000.5},
        {70, -20000000000.5}
    };
    long double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_longdouble_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float128_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float128_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongLongDouble struct_data;
        TypeSupport static_pubsubType {new MapLongLongLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_longdouble().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_longdouble().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_longdouble().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_boolean_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_boolean_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BOOLEAN),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, bool> value {
        {-100, true},
        {200, false},
        {-50, true},
        {70, false}
    };
    bool test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_boolean_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_boolean_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_boolean_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongBoolean struct_data;
        TypeSupport static_pubsubType {new MapLongLongBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_boolean().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_boolean().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_boolean().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_octet_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_octet_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BYTE),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, fastdds::rtps::octet> value {
        {-100, fastdds::rtps::octet(1)},
        {200, fastdds::rtps::octet(2)},
        {-50, fastdds::rtps::octet(100)},
        {70, fastdds::rtps::octet(200)}
    };
    fastdds::rtps::octet test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_octet_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_byte_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_byte_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongOctet struct_data;
        TypeSupport static_pubsubType {new MapLongLongOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_octet().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_octet().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_octet().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_char_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_char_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR8),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, char> value {
        {-100, 'a'},
        {200, 'A'},
        {-50, '{'},
        {70, '}'}
    };
    char test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_char_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char8_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char8_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongChar struct_data;
        TypeSupport static_pubsubType {new MapLongLongCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_char().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_char().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_char().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_wchar_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_wchar_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, wchar_t> value {
        {-100, L'a'},
        {200, L'A'},
        {-50, L'{'},
        {70, L'}'}
    };
    wchar_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_wchar_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongWChar struct_data;
        TypeSupport static_pubsubType {new MapLongLongWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_wchar().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_wchar().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_wchar().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_string_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_string_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int64_t, std::string> value {
        {-10, "we"},
        {-40, "are"},
        {50, "testing"},
        {600, "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_string_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongString struct_data;
        TypeSupport static_pubsubType {new MapLongLongStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_string().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_string().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_string().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_wstring_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_wstring_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                DynamicTypeBuilderFactory:: get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int64_t, std::wstring> value {
        {-10, L"we"},
        {-40, L"are"},
        {50, L"testing"},
        {600, L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_wstring_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongWString struct_data;
        TypeSupport static_pubsubType {new MapLongLongWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_wstring().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_wstring().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_wstring().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerAliasBoundedStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_inneraliasboundedstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_inneraliasboundedstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int64_t, std::string> value {
        {-10, "we"},
        {-40, "are"},
        {50, "testing"},
        {600, "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_inneraliasboundedstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerAliasBoundedStringHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerAliasBoundedStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_inneraliasboundedstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_inneraliasboundedstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_inneraliasboundedstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second.to_string());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerAliasBoundedStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerAliasBoundedWStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_inneraliasboundedwstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_inneraliasboundedwstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<int64_t, std::wstring> value {
        {-10, L"we"},
        {-40, L"are"},
        {50, L"testing"},
        {600, L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_inneraliasboundedwstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerAliasBoundedWStringHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerAliasBoundedWStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_inneraliasboundedwstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_inneraliasboundedwstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_inneraliasboundedwstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerAliasBoundedWStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_innerenumhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_innerenumhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_enum_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, InnerEnumHelper> value {
        {-100, InnerEnumHelper::ENUM_VALUE_2},
        {50, InnerEnumHelper::ENUM_VALUE_1},
        {-600, InnerEnumHelper::ENUM_VALUE_3},
        {70, InnerEnumHelper::ENUM_VALUE_2}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_innerenumhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                static_cast<int32_t>(map_element.second)));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, static_cast<InnerEnumHelper>(test_value));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_innerenumhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_innerenumhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_innerenumhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_innerbitmaskhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_innerbitmaskhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_bitmask_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, uint32_t> value {
        {-100, InnerBitMaskHelperBits::flag0},
        {50, 0},
        {-600, InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0},
        {70, InnerBitMaskHelperBits::flag4}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_innerbitmaskhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_innerbitmaskhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_innerbitmaskhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_innerbitmaskhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_inneraliashelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_inneraliashelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_alias_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, int32_t> value {
        {-100, 102},
        {50, 1},
        {-600, -32},
        {70, 43}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_inneraliashelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_inneraliashelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_inneraliashelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_inneraliashelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerAliasArrayHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_inneraliasarrayhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_inneraliasarrayhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, Int16Seq> value {
        {-100, {102, -102} },
        {50, {1, -1} },
        {-600, {-32, 32} },
        {70, {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_inneraliasarrayhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerAliasArrayHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerAliasArrayHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_inneraliasarrayhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_inneraliasarrayhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_inneraliasarrayhelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerAliasArrayHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerAliasSequenceHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_inneraliassequencehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_inneraliassequencehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, Int16Seq> value {
        {-10, {102, -102, 304, -304} },
        {-50, {} },
        {60, {-32} },
        {70, {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_inneraliassequencehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerAliasSequenceHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerAliasSequenceHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_inneraliassequencehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_inneraliassequencehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_inneraliassequencehelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerAliasSequenceHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerAliasMapHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_inneraliasmaphelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_inneraliasmaphelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, std::map<int32_t, int32_t>> value {
        {-100, {
             {1000, -102},
             {2000, -103},
             {-100, 1000}
         }},
        {50, {
         }},
        {-600, {
             {-1000, 102},
             {-2000, 103}
         }},
        {70, {
             {-3000, 302},
         }}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_inneraliasmaphelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->set_int32_value(inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first)),
                    inner_map_element.second));
        }

        map_data->return_loaned_value(inner_map_data);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->get_int32_value(test_value, inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first))));
            EXPECT_EQ(inner_map_element.second, test_value);
        }

        map_data->return_loaned_value(inner_map_data);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerAliasMapHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerAliasMapHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_inneraliasmaphelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_inneraliasmaphelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_inneraliasmaphelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerAliasMapHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_innerunionhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_innerunionhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_union_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, std::pair<const char* const, int16_t>> value {
        {-10, {union_long_member_name, std::int16_t(32)}},
        {-50, {union_float_member_name, std::int16_t(-12)}},
        {60, {union_short_member_name, std::int16_t(1)}},
        {70, {union_long_member_name, std::int16_t(-32)}}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_innerunionhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->set_int16_value(inner_union->get_member_id_by_name(map_element.second.first),
                map_element.second.second));

        map_data->return_loaned_value(inner_union);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->get_float64_value(test_value,
                inner_union->get_member_id_by_name(map_element.second.first)));
        EXPECT_EQ(map_element.second.second, static_cast<int16_t>(test_value));

        map_data->return_loaned_value(inner_union);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_innerunionhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_innerunionhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_innerunionhelper().end(), it);
            if (union_long_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.longValue()));
            }
            else if (union_float_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.floatValue()));
            }
            else
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.shortValue()));
            }

        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_innerstructurehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_innerstructurehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_struct_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, std::pair<int32_t, float>> value {
        {-100, {32, 1.0f}},
        {50, {-12, -1.0f}},
        {-600, {1, -10.1f}},
        {70, {-32, 100.3f}}
    };
    int32_t test_value1;
    float test_value2;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_innerstructurehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_int32_value(inner_structure->get_member_id_by_name(struct_long_member_name),
                map_element.second.first));
        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_float32_value(inner_structure->get_member_id_by_name(struct_float_member_name),
                map_element.second.second));

        map_data->return_loaned_value(inner_structure);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_int32_value(test_value1,
                inner_structure->get_member_id_by_name(struct_long_member_name)));
        EXPECT_EQ(map_element.second.first, test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_float32_value(test_value2,
                inner_structure->get_member_id_by_name(struct_float_member_name)));
        EXPECT_EQ(map_element.second.second, test_value2);

        map_data->return_loaned_value(inner_structure);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_innerstructurehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_innerstructurehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_innerstructurehelper().end(), it);
            EXPECT_EQ(map_element.second.first, it->second.field1());
            EXPECT_EQ(map_element.second.second, it->second.field2());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapLongLongInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_innerbitsethelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_innerbitsethelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                create_inner_bitset_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int64_t, std::tuple<uint8_t, bool, uint16_t, int16_t>> value {
        {-100, {std::uint8_t(5), true, std::uint16_t(1000), std::int16_t(2000)}},
        {50, {std::uint8_t(7), false, std::uint16_t(555), std::int16_t(20)}},
        {600, {std::uint8_t(0), true, std::uint16_t(0), std::int16_t(0)}}
    };
    uint8_t test_value1;
    bool test_value2;
    uint16_t test_value3;
    int16_t test_value4;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_longlong_innerbitsethelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint8_value(inner_bitset->get_member_id_by_name(bitfield_a),
                std::get<0>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_boolean_value(inner_bitset->get_member_id_by_name(bitfield_b),
                std::get<1>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint16_value(inner_bitset->get_member_id_by_name(bitfield_c),
                std::get<2>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_int16_value(inner_bitset->get_member_id_by_name(bitfield_d),
                std::get<3>(map_element.second)));

        map_data->return_loaned_value(inner_bitset);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint8_value(test_value1,
                inner_bitset->get_member_id_by_name(bitfield_a)));
        EXPECT_EQ(std::get<0>(map_element.second), test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_boolean_value(test_value2,
                inner_bitset->get_member_id_by_name(bitfield_b)));
        EXPECT_EQ(std::get<1>(map_element.second), test_value2);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint16_value(test_value3,
                inner_bitset->get_member_id_by_name(bitfield_c)));
        EXPECT_EQ(std::get<2>(map_element.second), test_value3);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_int16_value(test_value4,
                inner_bitset->get_member_id_by_name(bitfield_d)));
        EXPECT_EQ(std::get<3>(map_element.second), test_value4);

        map_data->return_loaned_value(inner_bitset);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapLongLongInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new MapLongLongInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_longlong_innerbitsethelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_longlong_innerbitsethelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_longlong_innerbitsethelper().end(), it);
            EXPECT_EQ(std::get<0>(map_element.second), it->second.a);
            EXPECT_EQ(std::get<1>(map_element.second), it->second.b);
            EXPECT_EQ(std::get<2>(map_element.second), it->second.c);
            EXPECT_EQ(std::get<3>(map_element.second), it->second.d);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapLongLongInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}
//}}}

//{{{ ULongLong key
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_short_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_short_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, int16_t> value {
        {100u, std::int16_t(1)},
        {200u, std::int16_t(2)},
        {50u, std::int16_t(-1)},
        {70u, std::int16_t(-2)}
    };
    int16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_short_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongShort struct_data;
        TypeSupport static_pubsubType {new MapULongLongShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_ushort_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_ushort_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, uint16_t> value {
        {100u, std::uint16_t(1)},
        {200u, std::uint16_t(2)},
        {50u, std::uint16_t(100)},
        {70u, std::uint16_t(200)}
    };
    uint16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_ushort_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongUShort struct_data;
        TypeSupport static_pubsubType {new MapULongLongUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_u_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_u_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_u_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_long_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_long_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, int32_t> value {
        {100u, 10000},
        {200u, 20000},
        {50u, -10000},
        {70u, -20000}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_long_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongLong struct_data;
        TypeSupport static_pubsubType {new MapULongLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_ulong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_ulong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, uint32_t> value {
        {100u, 10000u},
        {200u, 20000u},
        {50u, 1000000u},
        {70u, 2000000u}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_ulong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongULong struct_data;
        TypeSupport static_pubsubType {new MapULongLongULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_u_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_u_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_u_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_longlong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_longlong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, int64_t> value {
        {100u, 100000000},
        {200u, 200000000},
        {50u, -100000000},
        {70u, -200000000}
    };
    int64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_longlong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongLongLong struct_data;
        TypeSupport static_pubsubType {new MapULongLongLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_long_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_long_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_long_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_ulonglong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_ulonglong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, uint64_t> value {
        {100u, 100000000u},
        {200u, 200000000u},
        {50u, 10000000000u},
        {70u, 20000000000u}
    };
    uint64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_ulonglong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongULongLong struct_data;
        TypeSupport static_pubsubType {new MapULongLongULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_u_long_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_u_long_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_u_long_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_float_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_float_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, float> value {
        {100u, 3.1f},
        {200u, 100.1f},
        {50u, -100.3f},
        {70u, -200.3f}
    };
    float test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_float_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongFloat struct_data;
        TypeSupport static_pubsubType {new MapULongLongFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_float().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_float().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_float().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapKeyULongLongValueDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_double_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_double_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, double> value {
        {100u, 100000000.3},
        {200u, 200000000.3},
        {50u, -10000000000.5},
        {70u, -20000000000.5}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_double_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float64_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float64_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapKeyULongLongValueDouble struct_data;
        TypeSupport static_pubsubType {new MapKeyULongLongValueDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapKeyULongLongValueDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_longdouble_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_longdouble_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT128),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, long double> value {
        {100u, 100000000.3},
        {200u, 200000000.3},
        {50u, -10000000000.5},
        {70u, -20000000000.5}
    };
    long double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_longdouble_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float128_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float128_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongLongDouble struct_data;
        TypeSupport static_pubsubType {new MapULongLongLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_long_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_long_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_long_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_boolean_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_boolean_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BOOLEAN),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, bool> value {
        {100u, true},
        {200u, false},
        {50u, true},
        {70u, false}
    };
    bool test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_boolean_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_boolean_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_boolean_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongBoolean struct_data;
        TypeSupport static_pubsubType {new MapULongLongBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_boolean().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_boolean().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_boolean().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_octet_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_octet_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BYTE),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, fastdds::rtps::octet> value {
        {100u, fastdds::rtps::octet(1)},
        {200u, fastdds::rtps::octet(2)},
        {50u, fastdds::rtps::octet(100)},
        {70u, fastdds::rtps::octet(200)}
    };
    fastdds::rtps::octet test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_octet_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_byte_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_byte_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongOctet struct_data;
        TypeSupport static_pubsubType {new MapULongLongOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_octet().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_octet().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_octet().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_char_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_char_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR8),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, char> value {
        {100u, 'a'},
        {200u, 'A'},
        {50u, '{'},
        {70u, '}'}
    };
    char test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_char_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char8_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char8_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongChar struct_data;
        TypeSupport static_pubsubType {new MapULongLongCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_char().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_char().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_char().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_wchar_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_wchar_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, wchar_t> value {
        {100u, L'a'},
        {200u, L'A'},
        {50u, L'{'},
        {70u, L'}'}
    };
    wchar_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_wchar_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char16_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongWChar struct_data;
        TypeSupport static_pubsubType {new MapULongLongWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_wchar().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_wchar().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_wchar().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_string_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_string_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint64_t, std::string> value {
        {10, "we"},
        {40, "are"},
        {50, "testing"},
        {600, "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_string_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongString struct_data;
        TypeSupport static_pubsubType {new MapULongLongStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_string().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_string().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_string().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_wstring_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_wstring_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                DynamicTypeBuilderFactory:: get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint64_t, std::wstring> value {
        {10, L"we"},
        {40, L"are"},
        {50, L"testing"},
        {600, L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_wstring_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongWString struct_data;
        TypeSupport static_pubsubType {new MapULongLongWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_wstring().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_wstring().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_wstring().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerAliasBoundedStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_inneraliasboundedstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_inneraliasboundedstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint64_t, std::string> value {
        {10, "we"},
        {40, "are"},
        {50, "testing"},
        {600, "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_inneraliasboundedstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerAliasBoundedStringHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerAliasBoundedStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_alias_bounded_string_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_alias_bounded_string_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_alias_bounded_string_helper().end(), it);
            EXPECT_EQ(map_element.second, it->second.to_string());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerAliasBoundedStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerAliasBoundedWStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_inneraliasboundedwstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_inneraliasboundedwstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<uint64_t, std::wstring> value {
        {10, L"we"},
        {40, L"are"},
        {50, L"testing"},
        {600, L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_inneraliasboundedwstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerAliasBoundedWStringHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerAliasBoundedWStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_alias_bounded_wstring_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_alias_bounded_wstring_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_alias_bounded_wstring_helper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerAliasBoundedWStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_innerenumhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_innerenumhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_enum_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, InnerEnumHelper> value {
        {100u, InnerEnumHelper::ENUM_VALUE_2},
        {50u, InnerEnumHelper::ENUM_VALUE_1},
        {600u, InnerEnumHelper::ENUM_VALUE_3},
        {70u, InnerEnumHelper::ENUM_VALUE_2}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_innerenumhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                static_cast<int32_t>(map_element.second)));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, static_cast<InnerEnumHelper>(test_value));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_enum_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_enum_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_enum_helper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_innerbitmaskhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_innerbitmaskhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_bitmask_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, uint32_t> value {
        {100u, InnerBitMaskHelperBits::flag0},
        {50u, 0},
        {600u, InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0},
        {70u, InnerBitMaskHelperBits::flag4}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_innerbitmaskhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_bit_mask_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_bit_mask_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_bit_mask_helper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_inneraliashelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_inneraliashelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_alias_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, int32_t> value {
        {100u, 102},
        {50u, 1},
        {600u, -32},
        {70u, 43}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_inneraliashelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_alias_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_alias_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_alias_helper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerAliasArrayHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_inneraliasarrayhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_inneraliasarrayhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, Int16Seq> value {
        {100u, {102, -102} },
        {50u, {1, -1} },
        {600u, {-32, 32} },
        {70u, {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_inneraliasarrayhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerAliasArrayHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerAliasArrayHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_alias_array_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_alias_array_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_alias_array_helper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerAliasArrayHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerAliasSequenceHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_inneraliassequencehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_inneraliassequencehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, Int16Seq> value {
        {10u, {102, -102, 304, -304} },
        {50u, {} },
        {60u, {-32} },
        {70u, {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_inneraliassequencehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerAliasSequenceHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerAliasSequenceHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_alias_sequence_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_alias_sequence_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_alias_sequence_helper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerAliasSequenceHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerAliasMapHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_inneraliasmaphelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_inneraliasmaphelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, std::map<int32_t, int32_t>> value {
        {100u, {
             {1000, -102},
             {2000, -103},
             {-100, 1000}
         }},
        {50u, {
         }},
        {600u, {
             {-1000, 102},
             {-2000, 103}
         }},
        {70u, {
             {-3000, 302},
         }}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_inneraliasmaphelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->set_int32_value(inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first)),
                    inner_map_element.second));
        }

        map_data->return_loaned_value(inner_map_data);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->get_int32_value(test_value, inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first))));
            EXPECT_EQ(inner_map_element.second, test_value);
        }

        map_data->return_loaned_value(inner_map_data);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerAliasMapHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerAliasMapHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_alias_map_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_alias_map_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_alias_map_helper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerAliasMapHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_innerunionhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_innerunionhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_union_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, std::pair<const char* const, int16_t>> value {
        {10u, {union_long_member_name, std::int16_t(32)}},
        {50u, {union_float_member_name, std::int16_t(-12)}},
        {60u, {union_short_member_name, std::int16_t(1)}},
        {70u, {union_long_member_name, std::int16_t(-32)}}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_innerunionhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->set_int16_value(inner_union->get_member_id_by_name(map_element.second.first),
                map_element.second.second));

        map_data->return_loaned_value(inner_union);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_union->get_float64_value(test_value,
                inner_union->get_member_id_by_name(map_element.second.first)));
        EXPECT_EQ(map_element.second.second, static_cast<int16_t>(test_value));

        map_data->return_loaned_value(inner_union);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_union_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_union_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_union_helper().end(), it);
            if (union_long_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.longValue()));
            }
            else if (union_float_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.floatValue()));
            }
            else
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.shortValue()));
            }

        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_innerstructurehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_innerstructurehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_struct_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, std::pair<int32_t, float>> value {
        {100u, {32, 1.0f}},
        {50u, {-12, -1.0f}},
        {600u, {1, -10.1f}},
        {70u, {-32, 100.3f}}
    };
    int32_t test_value1;
    float test_value2;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_innerstructurehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_int32_value(inner_structure->get_member_id_by_name(struct_long_member_name),
                map_element.second.first));
        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_float32_value(inner_structure->get_member_id_by_name(struct_float_member_name),
                map_element.second.second));

        map_data->return_loaned_value(inner_structure);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_int32_value(test_value1,
                inner_structure->get_member_id_by_name(struct_long_member_name)));
        EXPECT_EQ(map_element.second.first, test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_float32_value(test_value2,
                inner_structure->get_member_id_by_name(struct_float_member_name)));
        EXPECT_EQ(map_element.second.second, test_value2);

        map_data->return_loaned_value(inner_structure);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_structure_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_structure_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_structure_helper().end(), it);
            EXPECT_EQ(map_element.second.first, it->second.field1());
            EXPECT_EQ(map_element.second.second, it->second.field2());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapULongLongInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_innerbitsethelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_innerbitsethelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                create_inner_bitset_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<uint64_t, std::tuple<uint8_t, bool, uint16_t, int16_t>> value {
        {100u, {std::uint8_t(5), true, std::uint16_t(1000), std::int16_t(2000)}},
        {50u, {std::uint8_t(7), false, std::uint16_t(555), std::int16_t(20)}},
        {600u, {std::uint8_t(0), true, std::uint16_t(0), std::int16_t(0)}}
    };
    uint8_t test_value1;
    bool test_value2;
    uint16_t test_value3;
    int16_t test_value4;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_ulonglong_innerbitsethelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint8_value(inner_bitset->get_member_id_by_name(bitfield_a),
                std::get<0>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_boolean_value(inner_bitset->get_member_id_by_name(bitfield_b),
                std::get<1>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint16_value(inner_bitset->get_member_id_by_name(bitfield_c),
                std::get<2>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_int16_value(inner_bitset->get_member_id_by_name(bitfield_d),
                std::get<3>(map_element.second)));

        map_data->return_loaned_value(inner_bitset);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(std::to_string(map_element.first)));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint8_value(test_value1,
                inner_bitset->get_member_id_by_name(bitfield_a)));
        EXPECT_EQ(std::get<0>(map_element.second), test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_boolean_value(test_value2,
                inner_bitset->get_member_id_by_name(bitfield_b)));
        EXPECT_EQ(std::get<1>(map_element.second), test_value2);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint16_value(test_value3,
                inner_bitset->get_member_id_by_name(bitfield_c)));
        EXPECT_EQ(std::get<2>(map_element.second), test_value3);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_int16_value(test_value4,
                inner_bitset->get_member_id_by_name(bitfield_d)));
        EXPECT_EQ(std::get<3>(map_element.second), test_value4);

        map_data->return_loaned_value(inner_bitset);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapULongLongInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new MapULongLongInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_u_long_long_inner_bitset_helper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_u_long_long_inner_bitset_helper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_u_long_long_inner_bitset_helper().end(), it);
            EXPECT_EQ(std::get<0>(map_element.second), it->second.a);
            EXPECT_EQ(std::get<1>(map_element.second), it->second.b);
            EXPECT_EQ(std::get<2>(map_element.second), it->second.c);
            EXPECT_EQ(std::get<3>(map_element.second), it->second.d);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapULongLongInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}
//}}}

//{{{ String key
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_short_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_short_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, int16_t> value {
        {"we", std::int16_t(1)},
        {"are", std::int16_t(2)},
        {"testing", std::int16_t(-1)},
        {"things", std::int16_t(-2)}
    };
    int16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_short_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringShort struct_data;
        TypeSupport static_pubsubType {new MapStringShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_ushort_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_ushort_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, uint16_t> value {
        {"we", std::uint16_t(1)},
        {"are", std::uint16_t(2)},
        {"testing", std::uint16_t(100)},
        {"things", std::uint16_t(200)}
    };
    uint16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_ushort_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint16_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint16_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringUShort struct_data;
        TypeSupport static_pubsubType {new MapStringUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_ushort().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_ushort().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_ushort().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_long_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_long_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, int32_t> value {
        {"we", 10000},
        {"are", 20000},
        {"testing", -10000},
        {"string", -20000}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_long_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringLong struct_data;
        TypeSupport static_pubsubType {new MapStringLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_ulong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_ulong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, uint32_t> value {
        {"we", 10000u},
        {"are", 20000u},
        {"testing", 1000000u},
        {"things", 2000000u}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_ulong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringULong struct_data;
        TypeSupport static_pubsubType {new MapStringULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_ulong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_ulong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_ulong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_longlong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_longlong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, int64_t> value {
        {"we", 100000000},
        {"are", 200000000},
        {"testing", -100000000},
        {"things", -200000000}
    };
    int64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_longlong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int64_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int64_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringLongLong struct_data;
        TypeSupport static_pubsubType {new MapStringLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_longlong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_longlong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_longlong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_ulonglong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_ulonglong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, uint64_t> value {
        {"we", 100000000u},
        {"are", 200000000u},
        {"testing", 10000000000u},
        {"things", 20000000000u}
    };
    uint64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_ulonglong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint64_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint64_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringULongLong struct_data;
        TypeSupport static_pubsubType {new MapStringULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_ulonglong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_ulonglong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_ulonglong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_float_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_float_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, float> value {
        {"we", 3.1f},
        {"are", 100.1f},
        {"testing", -100.3f},
        {"things", -200.3f}
    };
    float test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_float_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringFloat struct_data;
        TypeSupport static_pubsubType {new MapStringFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_float().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_float().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_float().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_double_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_double_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, double> value {
        {"we", 100000000.3},
        {"are", 200000000.3},
        {"testing", -10000000000.5},
        {"things", -20000000000.5}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_double_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float64_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float64_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringDouble struct_data;
        TypeSupport static_pubsubType {new MapStringDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_longdouble_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_longdouble_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT128),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, long double> value {
        {"we", 100000000.3},
        {"are", 200000000.3},
        {"testing", -10000000000.5},
        {"things", -20000000000.5}
    };
    long double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_longdouble_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float128_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float128_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringLongDouble struct_data;
        TypeSupport static_pubsubType {new MapStringLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_longdouble().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_longdouble().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_longdouble().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_boolean_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_boolean_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BOOLEAN),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, bool> value {
        {"we", true},
        {"are", false},
        {"testing", true},
        {"things", false}
    };
    bool test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_boolean_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_boolean_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_boolean_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringBoolean struct_data;
        TypeSupport static_pubsubType {new MapStringBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_boolean().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_boolean().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_boolean().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_octet_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_octet_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BYTE),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, fastdds::rtps::octet> value {
        {"we", fastdds::rtps::octet(1)},
        {"are", fastdds::rtps::octet(2)},
        {"testing", fastdds::rtps::octet(100)},
        {"things", fastdds::rtps::octet(200)}
    };
    fastdds::rtps::octet test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_octet_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_byte_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_byte_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringOctet struct_data;
        TypeSupport static_pubsubType {new MapStringOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_octet().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_octet().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_octet().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_char_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_char_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR8),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, char> value {
        {"we", 'a'},
        {"are", 'A'},
        {"testing", '{'},
        {"things", '}'}
    };
    char test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_char_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char8_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char8_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringChar struct_data;
        TypeSupport static_pubsubType {new MapStringCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_char().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_char().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_char().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_wchar_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_wchar_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, wchar_t> value {
        {"we", L'a'},
        {"are", L'A'},
        {"testing", L'{'},
        {"things", L'}'}
    };
    wchar_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_wchar_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char16_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char16_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringWChar struct_data;
        TypeSupport static_pubsubType {new MapStringWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_wchar().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_wchar().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_wchar().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_string_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_string_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<std::string, std::string> value {
        {"we", "we"},
        {"are", "are"},
        {"testing", "testing"},
        {"things", "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_string_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringString struct_data;
        TypeSupport static_pubsubType {new MapStringStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_string().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_string().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_string().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_wstring_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_wstring_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<std::string, std::wstring> value {
        {"we", L"we"},
        {"are", L"are"},
        {"testing", L"testing"},
        {"things", L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_wstring_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringWString struct_data;
        TypeSupport static_pubsubType {new MapStringWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_wstring().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_wstring().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_wstring().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerAliasBoundedStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_inneraliasboundedstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_inneraliasboundedstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<std::string, std::string> value {
        {"we", "we"},
        {"are", "are"},
        {"testing", "testing"},
        {"things", "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_inneraliasboundedstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerAliasBoundedStringHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerAliasBoundedStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_inneraliasboundedstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_inneraliasboundedstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_inneraliasboundedstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second.to_string());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerAliasBoundedStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerAliasBoundedWStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_inneraliasboundedwstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_inneraliasboundedwstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<std::string, std::wstring> value {
        {"we", L"we"},
        {"are", L"are"},
        {"testing", L"testing"},
        {"things", L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_inneraliasboundedwstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerAliasBoundedWStringHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerAliasBoundedWStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_inneraliasboundedwstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_inneraliasboundedwstringhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_inneraliasboundedwstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerAliasBoundedWStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_innerenumhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_innerenumhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_enum_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, InnerEnumHelper> value {
        {"we", InnerEnumHelper::ENUM_VALUE_2},
        {"are", InnerEnumHelper::ENUM_VALUE_1},
        {"testing", InnerEnumHelper::ENUM_VALUE_3},
        {"things", InnerEnumHelper::ENUM_VALUE_2}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_innerenumhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(map_element.first),
                static_cast<int32_t>(map_element.second)));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, static_cast<InnerEnumHelper>(test_value));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_innerenumhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_innerenumhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_innerenumhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_innerbitmaskhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_innerbitmaskhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_bitmask_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, uint32_t> value {
        {"we", InnerBitMaskHelperBits::flag0},
        {"are", 0},
        {"testing", InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0},
        {"things", InnerBitMaskHelperBits::flag4}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_innerbitmaskhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_innerbitmaskhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_innerbitmaskhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_innerbitmaskhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_inneraliashelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_inneraliashelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_alias_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, int32_t> value {
        {"we", 102},
        {"are", 1},
        {"testing", -32},
        {"things", 43}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_inneraliashelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_inneraliashelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_inneraliashelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_inneraliashelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerAliasArrayHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_inneraliasarrayhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_inneraliasarrayhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, Int16Seq> value {
        {"we", {102, -102} },
        {"are", {1, -1} },
        {"testing", {-32, 32} },
        {"things", {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_inneraliasarrayhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerAliasArrayHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerAliasArrayHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_inneraliasarrayhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_inneraliasarrayhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_inneraliasarrayhelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerAliasArrayHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerAliasSequenceHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_inneraliassequencehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_inneraliassequencehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, Int16Seq> value {
        {"we", {102, -102, 304, -304} },
        {"are", {} },
        {"testing", {-32} },
        {"things", {43, -43} }
    };
    Int16Seq test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_inneraliassequencehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerAliasSequenceHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerAliasSequenceHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_inneraliassequencehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_inneraliassequencehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_inneraliassequencehelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerAliasSequenceHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerAliasMapHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_inneraliasmaphelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_inneraliasmaphelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, std::map<int32_t, int32_t>> value {
        {"we", {
             {1000, -102},
             {2000, -103},
             {-100, 1000}
         }},
        {"are", {
         }},
        {"testing", {
             {-1000, 102},
             {-2000, 103}
         }},
        {"things", {
             {-3000, 302},
         }}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_inneraliasmaphelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->set_int32_value(inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first)),
                    inner_map_element.second));
        }

        map_data->return_loaned_value(inner_map_data);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->get_int32_value(test_value,
                    inner_map_data->get_member_id_by_name(std::to_string(inner_map_element.first))));
            EXPECT_EQ(inner_map_element.second, test_value);
        }

        map_data->return_loaned_value(inner_map_data);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerAliasMapHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerAliasMapHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_inneraliasmaphelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_inneraliasmaphelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_inneraliasmaphelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerAliasMapHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_innerunionhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_innerunionhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_union_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, std::pair<const char* const, int16_t>> value {
        {"we", {union_long_member_name, std::int16_t(32)}},
        {"are", {union_float_member_name, std::int16_t(-12)}},
        {"testing", {union_short_member_name, std::int16_t(1)}},
        {"things", {union_long_member_name, std::int16_t(-32)}}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_innerunionhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_union->set_int16_value(inner_union->get_member_id_by_name(map_element.second.first),
                map_element.second.second));

        map_data->return_loaned_value(inner_union);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_union->get_float64_value(test_value,
                inner_union->get_member_id_by_name(map_element.second.first)));
        EXPECT_EQ(map_element.second.second, static_cast<int16_t>(test_value));

        map_data->return_loaned_value(inner_union);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_innerunionhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_innerunionhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_innerunionhelper().end(), it);
            if (union_long_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.longValue()));
            }
            else if (union_float_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.floatValue()));
            }
            else
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.shortValue()));
            }

        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_innerstructurehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_innerstructurehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_struct_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, std::pair<int32_t, float>> value {
        {"we", {32, 1.0f}},
        {"are", {-12, -1.0f}},
        {"testing", {1, -10.1f}},
        {"things", {-32, 100.3f}}
    };
    int32_t test_value1;
    float test_value2;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_innerstructurehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_int32_value(inner_structure->get_member_id_by_name(struct_long_member_name),
                map_element.second.first));
        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_float32_value(inner_structure->get_member_id_by_name(struct_float_member_name),
                map_element.second.second));

        map_data->return_loaned_value(inner_structure);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_int32_value(test_value1,
                inner_structure->get_member_id_by_name(struct_long_member_name)));
        EXPECT_EQ(map_element.second.first, test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_float32_value(test_value2,
                inner_structure->get_member_id_by_name(struct_float_member_name)));
        EXPECT_EQ(map_element.second.second, test_value2);

        map_data->return_loaned_value(inner_structure);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_innerstructurehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_innerstructurehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_innerstructurehelper().end(), it);
            EXPECT_EQ(map_element.second.first, it->second.field1());
            EXPECT_EQ(map_element.second.second, it->second.field2());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapStringInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_innerbitsethelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_innerbitsethelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                create_inner_bitset_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, std::tuple<uint8_t, bool, uint16_t, int16_t>> value {
        {"we", {std::uint8_t(5), true, std::uint16_t(1000), std::int16_t(2000)}},
        {"are", {std::uint8_t(7), false, std::uint16_t(555), std::int16_t(20)}},
        {"testing", {std::uint8_t(0), true, std::uint16_t(0), std::int16_t(0)}}
    };
    uint8_t test_value1;
    bool test_value2;
    uint16_t test_value3;
    int16_t test_value4;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_string_innerbitsethelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint8_value(inner_bitset->get_member_id_by_name(bitfield_a),
                std::get<0>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_boolean_value(inner_bitset->get_member_id_by_name(bitfield_b),
                std::get<1>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint16_value(inner_bitset->get_member_id_by_name(bitfield_c),
                std::get<2>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_int16_value(inner_bitset->get_member_id_by_name(bitfield_d),
                std::get<3>(map_element.second)));

        map_data->return_loaned_value(inner_bitset);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint8_value(test_value1,
                inner_bitset->get_member_id_by_name(bitfield_a)));
        EXPECT_EQ(std::get<0>(map_element.second), test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_boolean_value(test_value2,
                inner_bitset->get_member_id_by_name(bitfield_b)));
        EXPECT_EQ(std::get<1>(map_element.second), test_value2);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint16_value(test_value3,
                inner_bitset->get_member_id_by_name(bitfield_c)));
        EXPECT_EQ(std::get<2>(map_element.second), test_value3);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_int16_value(test_value4,
                inner_bitset->get_member_id_by_name(bitfield_d)));
        EXPECT_EQ(std::get<3>(map_element.second), test_value4);

        map_data->return_loaned_value(inner_bitset);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapStringInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new MapStringInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_string_innerbitsethelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_string_innerbitsethelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_string_innerbitsethelper().end(), it);
            EXPECT_EQ(std::get<0>(map_element.second), it->second.a);
            EXPECT_EQ(std::get<1>(map_element.second), it->second.b);
            EXPECT_EQ(std::get<2>(map_element.second), it->second.c);
            EXPECT_EQ(std::get<3>(map_element.second), it->second.d);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapStringInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}
//}}}

// Maps with wstring as key were omitted because there is not supported currently.

//{{{ InnerAliasBoundedStringHelper key
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_short_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_short_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, int16_t> value {
        {"we", std::int16_t(1)},
        {"are", std::int16_t(2)},
        {"testing", std::int16_t(-1)},
        {"things", std::int16_t(-2)}
    };
    int16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_short_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperShort struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_short().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_short().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_short().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_ushort_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_ushort_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, uint16_t> value {
        {"we", std::uint16_t(1)},
        {"are", std::uint16_t(2)},
        {"testing", std::uint16_t(100)},
        {"things", std::uint16_t(200)}
    };
    uint16_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_ushort_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint16_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint16_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperUShort struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_ushort().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_ushort().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_ushort().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_long_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_long_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, int32_t> value {
        {"we", 10000},
        {"are", 20000},
        {"testing", -10000},
        {"string", -20000}
    };
    int32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_long_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperLong struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_long().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_long().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_long().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_ulong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_ulong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, uint32_t> value {
        {"we", 10000u},
        {"are", 20000u},
        {"testing", 1000000u},
        {"things", 2000000u}
    };
    uint32_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_ulong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperULong struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_ulong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_ulong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_ulong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_longlong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_longlong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, int64_t> value {
        {"we", 100000000},
        {"are", 200000000},
        {"testing", -100000000},
        {"things", -200000000}
    };
    int64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_longlong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int64_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int64_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperLongLong struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_longlong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_longlong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_longlong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_ulonglong_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_ulonglong_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_UINT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, uint64_t> value {
        {"we", 100000000u},
        {"are", 200000000u},
        {"testing", 10000000000u},
        {"things", 20000000000u}
    };
    uint64_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_ulonglong_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint64_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint64_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperULongLong struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_ulonglong().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_ulonglong().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_ulonglong().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_float_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_float_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT32),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, float> value {
        {"we", 3.1f},
        {"are", 100.1f},
        {"testing", -100.3f},
        {"things", -200.3f}
    };
    float test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_float_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperFloat struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_float().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_float().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_float().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_double_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_double_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT64),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, double> value {
        {"we", 100000000.3},
        {"are", 200000000.3},
        {"testing", -10000000000.5},
        {"things", -20000000000.5}
    };
    double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_double_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float64_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float64_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperDouble struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_double().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_double().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_double().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_longdouble_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_longdouble_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_FLOAT128),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, long double> value {
        {"we", 100000000.3},
        {"are", 200000000.3},
        {"testing", -10000000000.5},
        {"things", -20000000000.5}
    };
    long double test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_longdouble_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_float128_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_float128_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperLongDouble struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_longdouble().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_longdouble().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_longdouble().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_boolean_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_boolean_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BOOLEAN),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, bool> value {
        {"we", true},
        {"are", false},
        {"testing", true},
        {"things", false}
    };
    bool test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_boolean_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_boolean_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_boolean_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperBoolean struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_boolean().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_boolean().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_boolean().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_octet_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_octet_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_BYTE),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, fastdds::rtps::octet> value {
        {"we", fastdds::rtps::octet(1)},
        {"are", fastdds::rtps::octet(2)},
        {"testing", fastdds::rtps::octet(100)},
        {"things", fastdds::rtps::octet(200)}
    };
    fastdds::rtps::octet test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_octet_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_byte_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_byte_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperOctet struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_octet().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_octet().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_octet().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_char_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_char_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR8),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, char> value {
        {"we", 'a'},
        {"are", 'A'},
        {"testing", '{'},
        {"things", '}'}
    };
    char test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_char_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char8_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char8_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperChar struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_char().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_char().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_char().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_wchar_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_wchar_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_CHAR16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, wchar_t> value {
        {"we", L'a'},
        {"are", L'A'},
        {"testing", L'{'},
        {"things", L'}'}
    };
    wchar_t test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_wchar_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_char16_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_char16_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperWChar struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_wchar().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_wchar().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_wchar().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_string_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_string_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<std::string, std::string> value {
        {"we", "we"},
        {"are", "are"},
        {"testing", "testing"},
        {"things", "things"}
    };
    std::string test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_string_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperString struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_string().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_string().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_string().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_wstring_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_wstring_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                DynamicTypeBuilderFactory:: get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<std::string, std::wstring> value {
        {"we", L"we"},
        {"are", L"are"},
        {"testing", L"testing"},
        {"things", L"things"}
    };
    std::wstring test_value;
    auto map_data = data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_wstring_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperWString struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_wstring().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_wstring().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_wstring().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_inneraliasboundedstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_inneraliasboundedstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<std::string, std::string> value {
        {"we", "we"},
        {"are", "are"},
        {"testing", "testing"},
        {"things", "things"}
    };
    std::string test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(
                        var_inneraliasboundedstringhelper_inneraliasboundedstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(),
                struct_data.var_map_inneraliasboundedstringhelper_inneraliasboundedstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_inneraliasboundedstringhelper().find(
                map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_inneraliasboundedstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second.to_string());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_inneraliasboundedwstringhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_inneraliasboundedwstringhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::map<std::string, std::wstring> value {
        {"we", L"we"},
        {"are", L"are"},
        {"testing", L"testing"},
        {"things", L"things"}
    };
    std::wstring test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(
                        var_inneraliasboundedstringhelper_inneraliasboundedwstringhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_wstring_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_wstring_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(),
                struct_data.var_map_inneraliasboundedstringhelper_inneraliasboundedwstringhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_inneraliasboundedwstringhelper().find(
                map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_inneraliasboundedwstringhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_innerenumhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_innerenumhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_enum_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, InnerEnumHelper> value {
        {"we", InnerEnumHelper::ENUM_VALUE_2},
        {"are", InnerEnumHelper::ENUM_VALUE_1},
        {"testing", InnerEnumHelper::ENUM_VALUE_3},
        {"things", InnerEnumHelper::ENUM_VALUE_2}
    };
    int32_t test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_innerenumhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(map_element.first),
                static_cast<int32_t>(map_element.second)));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, static_cast<InnerEnumHelper>(test_value));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_innerenumhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_innerenumhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_innerenumhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_innerbitmaskhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_innerbitmaskhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_bitmask_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, uint32_t> value {
        {"we", InnerBitMaskHelperBits::flag0},
        {"are", 0},
        {"testing", InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0},
        {"things", InnerBitMaskHelperBits::flag4}
    };
    uint32_t test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(
                        var_inneraliasboundedstringhelper_innerbitmaskhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_uint32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_uint32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_innerbitmaskhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_innerbitmaskhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_innerbitmaskhelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_inneraliashelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_inneraliashelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_alias_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, int32_t> value {
        {"we", 102},
        {"are", 1},
        {"testing", -32},
        {"things", 43}
    };
    int32_t test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_inneraliashelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_inneraliashelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_inneraliashelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_inneraliashelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerAliasArrayHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_inneraliasarrayhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_inneraliasarrayhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, Int16Seq> value {
        {"we", {102, -102} },
        {"are", {1, -1} },
        {"testing", {-32, 32} },
        {"things", {43, -43} }
    };
    Int16Seq test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(
                        var_inneraliasboundedstringhelper_inneraliasarrayhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerAliasArrayHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerAliasArrayHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_inneraliasarrayhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_inneraliasarrayhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_inneraliasarrayhelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerAliasArrayHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerAliasSequenceHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_inneraliassequencehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_inneraliassequencehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, Int16Seq> value {
        {"we", {102, -102, 304, -304} },
        {"are", {} },
        {"testing", {-32} },
        {"things", {43, -43} }
    };
    Int16Seq test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(
                        var_inneraliasboundedstringhelper_inneraliassequencehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int16_values(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    // Check values
    for (auto const& map_element : value)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int16_values(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerAliasSequenceHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerAliasSequenceHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_inneraliassequencehelper().size());
        for (auto const& map_element : value)
        {
            auto it =
                    struct_data.var_map_inneraliasboundedstringhelper_inneraliassequencehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_inneraliassequencehelper().end(), it);
            EXPECT_TRUE(std::equal(map_element.second.begin(), map_element.second.end(), it->second.begin()));
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerAliasSequenceHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerAliasMapHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_inneraliasmaphelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_inneraliasmaphelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, std::map<int32_t, int32_t>> value {
        {"we", {
             {1000, -102},
             {2000, -103},
             {-100, 1000}
         }},
        {"are", {
         }},
        {"testing", {
             {-1000, 102},
             {-2000, 103}
         }},
        {"things", {
             {-3000, 302},
         }}
    };
    int32_t test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(
                        var_inneraliasboundedstringhelper_inneraliasmaphelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->set_int32_value(inner_map_data->get_member_id_by_name(std::to_string(
                        inner_map_element.first)),
                    inner_map_element.second));
        }

        map_data->return_loaned_value(inner_map_data);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_map_data = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        for (auto const& inner_map_element : map_element.second)
        {
            EXPECT_EQ(RETCODE_OK,
                    inner_map_data->get_int32_value(test_value,
                    inner_map_data->get_member_id_by_name(std::to_string(inner_map_element.first))));
            EXPECT_EQ(inner_map_element.second, test_value);
        }

        map_data->return_loaned_value(inner_map_data);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerAliasMapHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerAliasMapHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_inneraliasmaphelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_inneraliasmaphelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_inneraliasmaphelper().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerAliasMapHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_innerunionhelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_innerunionhelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_union_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, std::pair<const char* const, int16_t>> value {
        {"we", {union_long_member_name, std::int16_t(32)}},
        {"are", {union_float_member_name, std::int16_t(-12)}},
        {"testing", {union_short_member_name, std::int16_t(1)}},
        {"things", {union_long_member_name, std::int16_t(-32)}}
    };
    double test_value;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(var_inneraliasboundedstringhelper_innerunionhelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_union->set_int16_value(inner_union->get_member_id_by_name(map_element.second.first),
                map_element.second.second));

        map_data->return_loaned_value(inner_union);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_union = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_union->get_float64_value(test_value,
                inner_union->get_member_id_by_name(map_element.second.first)));
        EXPECT_EQ(map_element.second.second, static_cast<int16_t>(test_value));

        map_data->return_loaned_value(inner_union);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_innerunionhelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_innerunionhelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_innerunionhelper().end(), it);
            if (union_long_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.longValue()));
            }
            else if (union_float_member_name == map_element.second.first)
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.floatValue()));
            }
            else
            {
                EXPECT_EQ(map_element.second.second, static_cast<int16_t>(it->second.shortValue()));
            }

        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_innerstructurehelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_innerstructurehelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_struct_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, std::pair<int32_t, float>> value {
        {"we", {32, 1.0f}},
        {"are", {-12, -1.0f}},
        {"testing", {1, -10.1f}},
        {"things", {-32, 100.3f}}
    };
    int32_t test_value1;
    float test_value2;
    auto map_data =
            data->loan_value(data->get_member_id_by_name(
                        var_inneraliasboundedstringhelper_innerstructurehelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_int32_value(inner_structure->get_member_id_by_name(struct_long_member_name),
                map_element.second.first));
        EXPECT_EQ(RETCODE_OK,
                inner_structure->set_float32_value(inner_structure->get_member_id_by_name(struct_float_member_name),
                map_element.second.second));

        map_data->return_loaned_value(inner_structure);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_structure = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_int32_value(test_value1,
                inner_structure->get_member_id_by_name(struct_long_member_name)));
        EXPECT_EQ(map_element.second.first, test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_structure->get_float32_value(test_value2,
                inner_structure->get_member_id_by_name(struct_float_member_name)));
        EXPECT_EQ(map_element.second.second, test_value2);

        map_data->return_loaned_value(inner_structure);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_innerstructurehelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_innerstructurehelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_innerstructurehelper().end(), it);
            EXPECT_EQ(map_element.second.first, it->second.field1());
            EXPECT_EQ(map_element.second.second, it->second.field2());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MapInnerAliasBoundedStringHelperInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inneraliasboundedstringhelper_innerbitsethelper_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_inneraliasboundedstringhelper_innerbitsethelper_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                create_inner_alias_bounded_string_helper(),
                create_inner_bitset_helper(),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<std::string, std::tuple<uint8_t, bool, uint16_t, int16_t>> value {
        {"we", {std::uint8_t(5), true, std::uint16_t(1000), std::int16_t(2000)}},
        {"are", {std::uint8_t(7), false, std::uint16_t(555), std::int16_t(20)}},
        {"testing", {std::uint8_t(0), true, std::uint16_t(0), std::int16_t(0)}}
    };
    uint8_t test_value1;
    bool test_value2;
    uint16_t test_value3;
    int16_t test_value4;
    auto map_data = data->loan_value(data->get_member_id_by_name(
                        var_inneraliasboundedstringhelper_innerbitsethelper_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint8_value(inner_bitset->get_member_id_by_name(bitfield_a),
                std::get<0>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_boolean_value(inner_bitset->get_member_id_by_name(bitfield_b),
                std::get<1>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_uint16_value(inner_bitset->get_member_id_by_name(bitfield_c),
                std::get<2>(map_element.second)));
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->set_int16_value(inner_bitset->get_member_id_by_name(bitfield_d),
                std::get<3>(map_element.second)));

        map_data->return_loaned_value(inner_bitset);
    }
    // Check values
    for (auto const& map_element : value)
    {
        auto inner_bitset = map_data->loan_value(map_data->get_member_id_by_name(map_element.first));

        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint8_value(test_value1,
                inner_bitset->get_member_id_by_name(bitfield_a)));
        EXPECT_EQ(std::get<0>(map_element.second), test_value1);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_boolean_value(test_value2,
                inner_bitset->get_member_id_by_name(bitfield_b)));
        EXPECT_EQ(std::get<1>(map_element.second), test_value2);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_uint16_value(test_value3,
                inner_bitset->get_member_id_by_name(bitfield_c)));
        EXPECT_EQ(std::get<2>(map_element.second), test_value3);
        EXPECT_EQ(RETCODE_OK,
                inner_bitset->get_int16_value(test_value4,
                inner_bitset->get_member_id_by_name(bitfield_d)));
        EXPECT_EQ(std::get<3>(map_element.second), test_value4);

        map_data->return_loaned_value(inner_bitset);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        MapInnerAliasBoundedStringHelperInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new MapInnerAliasBoundedStringHelperInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value.size(), struct_data.var_map_inneraliasboundedstringhelper_innerbitsethelper().size());
        for (auto const& map_element : value)
        {
            auto it = struct_data.var_map_inneraliasboundedstringhelper_innerbitsethelper().find(map_element.first);
            ASSERT_NE(struct_data.var_map_inneraliasboundedstringhelper_innerbitsethelper().end(), it);
            EXPECT_EQ(std::get<0>(map_element.second), it->second.a);
            EXPECT_EQ(std::get<1>(map_element.second), it->second.b);
            EXPECT_EQ(std::get<2>(map_element.second), it->second.c);
            EXPECT_EQ(std::get<3>(map_element.second), it->second.d);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MapInnerAliasBoundedStringHelperInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}
//}}}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BoundedSmallMap)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bounded_small_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_small_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                1)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_unbounded_string_long_bounded_small_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                5)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_unbounded_string_bounded_small_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                5)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, int32_t> value_small_map {
        {-100, 10000},
    };
    const std::unordered_map<std::string, int32_t> value_unbounded_string_long_bounded_small_map {
        {"we", 10000},
        {"are", 20000},
        {"testing", -10000},
        {"things", -20000}
    };
    const std::unordered_map<int32_t, std::string> value_long_unbounded_string_long_bounded_small_map {
        {10, "we"},
        {50, "are"},
        {60, "testing"},
        {70, "things"}
    };
    int32_t test_value;
    std::string test_str_value;
    // Set values
    auto map_data = data->loan_value(data->get_member_id_by_name(var_small_map));
    ASSERT_TRUE(map_data);
    for (auto const& map_element : value_small_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));
    map_data = data->loan_value(data->get_member_id_by_name(var_unbounded_string_long_bounded_small_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value_unbounded_string_long_bounded_small_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));
    map_data = data->loan_value(data->get_member_id_by_name(var_long_unbounded_string_bounded_small_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value_long_unbounded_string_long_bounded_small_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    // Check values
    map_data = data->loan_value(data->get_member_id_by_name(var_small_map));
    ASSERT_TRUE(map_data);
    for (auto const& map_element : value_small_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));
    map_data = data->loan_value(data->get_member_id_by_name(var_unbounded_string_long_bounded_small_map));
    ASSERT_TRUE(map_data);
    for (auto const& map_element : value_unbounded_string_long_bounded_small_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));
    map_data = data->loan_value(data->get_member_id_by_name(var_long_unbounded_string_bounded_small_map));
    ASSERT_TRUE(map_data);
    for (auto const& map_element : value_long_unbounded_string_long_bounded_small_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_str_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_str_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        BoundedSmallMap struct_data;
        TypeSupport static_pubsubType {new BoundedSmallMapPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value_small_map.size(), struct_data.var_small_map().size());
        for (auto const& map_element : value_small_map)
        {
            auto it = struct_data.var_small_map().find(map_element.first);
            ASSERT_NE(struct_data.var_small_map().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
        EXPECT_EQ(value_unbounded_string_long_bounded_small_map.size(),
                struct_data.var_unbounded_string_long_bounded_small_map().size());
        for (auto const& map_element : value_unbounded_string_long_bounded_small_map)
        {
            auto it = struct_data.var_unbounded_string_long_bounded_small_map().find(map_element.first);
            ASSERT_NE(struct_data.var_unbounded_string_long_bounded_small_map().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
        EXPECT_EQ(value_long_unbounded_string_long_bounded_small_map.size(),
                struct_data.var_long_unbounded_string_bounded_small_map().size());
        for (auto const& map_element : value_long_unbounded_string_long_bounded_small_map)
        {
            auto it = struct_data.var_long_unbounded_string_bounded_small_map().find(map_element.first);
            ASSERT_NE(struct_data.var_long_unbounded_string_bounded_small_map().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BoundedSmallMap_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BoundedLargeMap)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bounded_large_map_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_large_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                41925)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_unbounded_string_long_bounded_large_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                255)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_unbounded_string_bounded_large_map);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(
                DynamicTypeBuilderFactory:: get_instance()->get_primitive_type(TK_INT32),
                DynamicTypeBuilderFactory:: get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))
                        ->build(),
                255)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    const std::unordered_map<int32_t, int32_t> value_large_map {
        {-100, 10000},
    };
    const std::unordered_map<std::string, int32_t> value_unbounded_string_long_bounded_large_map {
        {"we", 10000},
        {"are", 20000},
        {"testing", -10000},
        {"things", -20000}
    };
    const std::unordered_map<int32_t, std::string> value_long_unbounded_string_long_bounded_large_map {
        {10, "we"},
        {50, "are"},
        {60, "testing"},
        {70, "things"}
    };
    int32_t test_value;
    std::string test_str_value;
    // Set values
    auto map_data = data->loan_value(data->get_member_id_by_name(var_large_map));
    ASSERT_TRUE(map_data);
    for (auto const& map_element : value_large_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));
    map_data = data->loan_value(data->get_member_id_by_name(var_unbounded_string_long_bounded_large_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value_unbounded_string_long_bounded_large_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_int32_value(map_data->get_member_id_by_name(map_element.first),
                map_element.second));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));
    map_data = data->loan_value(data->get_member_id_by_name(var_long_unbounded_string_bounded_large_map));
    ASSERT_TRUE(map_data);
    // Set values
    for (auto const& map_element : value_long_unbounded_string_long_bounded_large_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->set_string_value(map_data->get_member_id_by_name(std::to_string(map_element.first)),
                map_element.second));
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    // Check values
    map_data = data->loan_value(data->get_member_id_by_name(var_large_map));
    ASSERT_TRUE(map_data);
    for (auto const& map_element : value_large_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));
    map_data = data->loan_value(data->get_member_id_by_name(var_unbounded_string_long_bounded_large_map));
    ASSERT_TRUE(map_data);
    for (auto const& map_element : value_unbounded_string_long_bounded_large_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_int32_value(test_value, map_data->get_member_id_by_name(map_element.first)));
        EXPECT_EQ(map_element.second, test_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));
    map_data = data->loan_value(data->get_member_id_by_name(var_long_unbounded_string_bounded_large_map));
    ASSERT_TRUE(map_data);
    for (auto const& map_element : value_long_unbounded_string_long_bounded_large_map)
    {
        EXPECT_EQ(RETCODE_OK,
                map_data->get_string_value(test_str_value, map_data->get_member_id_by_name(std::to_string(
                    map_element.first))));
        EXPECT_EQ(map_element.second, test_str_value);
    }
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(map_data));

    for (auto encoding : encodings)
    {
        BoundedLargeMap struct_data;
        TypeSupport static_pubsubType {new BoundedLargeMapPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value_large_map.size(), struct_data.var_large_map().size());
        for (auto const& map_element : value_large_map)
        {
            auto it = struct_data.var_large_map().find(map_element.first);
            ASSERT_NE(struct_data.var_large_map().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
        EXPECT_EQ(value_unbounded_string_long_bounded_large_map.size(),
                struct_data.var_unbounded_string_long_bounded_large_map().size());
        for (auto const& map_element : value_unbounded_string_long_bounded_large_map)
        {
            auto it = struct_data.var_unbounded_string_long_bounded_large_map().find(map_element.first);
            ASSERT_NE(struct_data.var_unbounded_string_long_bounded_large_map().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
        EXPECT_EQ(value_long_unbounded_string_long_bounded_large_map.size(),
                struct_data.var_long_unbounded_string_bounded_large_map().size());
        for (auto const& map_element : value_long_unbounded_string_long_bounded_large_map)
        {
            auto it = struct_data.var_long_unbounded_string_bounded_large_map().find(map_element.first);
            ASSERT_NE(struct_data.var_long_unbounded_string_bounded_large_map().end(), it);
            EXPECT_EQ(map_element.second, it->second);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BoundedLargeMap_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

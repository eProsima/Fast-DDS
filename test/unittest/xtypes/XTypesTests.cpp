// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/log/Log.h>
#include "idl/TypesTypeObject.h"
#include "idl/WideEnumTypeObject.h"
#include <gtest/gtest.h>
#include <tinyxml2.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

class XTypesTests: public ::testing::Test
{
    public:
        XTypesTests()
        {
            //registerTypesTypes();
        }

        ~XTypesTests()
        {
            TypeObjectFactory::DeleteInstance();
            Log::KillThread();
        }

        virtual void TearDown()
        {
        }
};

TEST_F(XTypesTests, EnumMinimalCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_enum = GetMinimalMyEnumStructObject();
    const TypeObject* my_bad_enum = GetMinimalMyBadEnumStructObject();
    const TypeObject* my_wide_enum = GetMinimalMyEnumWideStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_enum->consistent(*my_enum, consistencyQos, unused));
    ASSERT_TRUE(my_enum->consistent(*my_bad_enum, consistencyQos, unused));
    ASSERT_TRUE(my_enum->consistent(*my_wide_enum, consistencyQos, unused));
    ASSERT_TRUE(my_wide_enum->consistent(*my_enum, consistencyQos, unused));

    // Now don't ignore names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_enum->consistent(*my_bad_enum, consistencyQos, unused));
    ASSERT_FALSE(my_enum->consistent(*my_wide_enum, consistencyQos, unused));
    ASSERT_FALSE(my_wide_enum->consistent(*my_enum, consistencyQos, unused));

    // Now don't allow type widening
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_prevent_type_widening = true;
    ASSERT_TRUE(my_enum->consistent(*my_wide_enum, consistencyQos, unused));
    ASSERT_FALSE(my_wide_enum->consistent(*my_enum, consistencyQos, unused));
}

TEST_F(XTypesTests, EnumCompleteCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_enum = GetCompleteMyEnumStructObject();
    const TypeObject* my_bad_enum = GetCompleteMyBadEnumStructObject();
    const TypeObject* my_wide_enum = GetCompleteMyEnumWideStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_enum->consistent(*my_enum, consistencyQos, unused));
    ASSERT_TRUE(my_enum->consistent(*my_bad_enum, consistencyQos, unused));
    ASSERT_TRUE(my_enum->consistent(*my_wide_enum, consistencyQos, unused));
    ASSERT_TRUE(my_wide_enum->consistent(*my_enum, consistencyQos, unused));

    // Now don't ignore names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_enum->consistent(*my_bad_enum, consistencyQos, unused));
    ASSERT_FALSE(my_enum->consistent(*my_wide_enum, consistencyQos, unused));
    ASSERT_FALSE(my_wide_enum->consistent(*my_enum, consistencyQos, unused));

    // Now don't allow type widening
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_prevent_type_widening = true;
    ASSERT_TRUE(my_enum->consistent(*my_wide_enum, consistencyQos, unused));
    ASSERT_FALSE(my_wide_enum->consistent(*my_enum, consistencyQos, unused));
}

TEST_F(XTypesTests, AliasMinimalCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_enum = GetMinimalMyEnumStructObject();
    const TypeObject* my_alias_enum = GetMinimalMyAliasEnumStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_enum->consistent(*my_alias_enum, consistencyQos, unused));
    ASSERT_TRUE(my_alias_enum->consistent(*my_enum, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_TRUE(my_enum->consistent(*my_alias_enum, consistencyQos, unused));
    ASSERT_TRUE(my_alias_enum->consistent(*my_enum, consistencyQos, unused));
}

TEST_F(XTypesTests, AliasCompleteCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_enum = GetCompleteMyEnumStructObject();
    const TypeObject* my_alias_enum = GetCompleteMyAliasEnumStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_enum->consistent(*my_alias_enum, consistencyQos, unused));
    ASSERT_TRUE(my_alias_enum->consistent(*my_enum, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_TRUE(my_enum->consistent(*my_alias_enum, consistencyQos, unused));
    ASSERT_TRUE(my_alias_enum->consistent(*my_enum, consistencyQos, unused));
}

TEST_F(XTypesTests, BasicStructMinimalCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* basic_struct = GetMinimalBasicStructObject();
    const TypeObject* basic_names_struct = GetMinimalBasicNamesStructObject();
    const TypeObject* basic_bad_struct = GetMinimalBasicBadStructObject();
    const TypeObject* basic_wide_struct = GetMinimalBasicWideStructObject();
    const TypeObject* basic_wide_bad_struct = GetMinimalBadBasicWideStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(basic_struct->consistent(*basic_names_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_struct->consistent(*basic_bad_struct, consistencyQos, unused));
    ASSERT_TRUE(basic_struct->consistent(*basic_wide_struct, consistencyQos, unused));
    ASSERT_TRUE(basic_wide_struct->consistent(*basic_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_struct->consistent(*basic_wide_bad_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_bad_struct->consistent(*basic_struct, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(basic_struct->consistent(*basic_names_struct, consistencyQos, unused));
    ASSERT_TRUE(basic_struct->consistent(*basic_wide_struct, consistencyQos, unused));
    ASSERT_TRUE(basic_wide_struct->consistent(*basic_struct, consistencyQos, unused));

    // Don't allow type widening
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_prevent_type_widening = true;
    ASSERT_TRUE(basic_struct->consistent(*basic_wide_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_struct->consistent(*basic_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_struct->consistent(*basic_wide_bad_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_bad_struct->consistent(*basic_struct, consistencyQos, unused));

    // DISALLOW coercion
    consistencyQos.m_kind = DISALLOW_TYPE_COERCION;
    ASSERT_FALSE(basic_struct->consistent(*basic_wide_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_struct->consistent(*basic_struct, consistencyQos, unused));
}

TEST_F(XTypesTests, BasicStructCompleteCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* basic_struct = GetCompleteBasicStructObject();
    const TypeObject* basic_names_struct = GetCompleteBasicNamesStructObject();
    const TypeObject* basic_bad_struct = GetCompleteBasicBadStructObject();
    const TypeObject* basic_wide_struct = GetCompleteBasicWideStructObject();
    const TypeObject* basic_wide_bad_struct = GetCompleteBadBasicWideStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(basic_struct->consistent(*basic_names_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_struct->consistent(*basic_bad_struct, consistencyQos, unused));
    ASSERT_TRUE(basic_struct->consistent(*basic_wide_struct, consistencyQos, unused));
    ASSERT_TRUE(basic_wide_struct->consistent(*basic_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_struct->consistent(*basic_wide_bad_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_bad_struct->consistent(*basic_struct, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(basic_struct->consistent(*basic_names_struct, consistencyQos, unused));
    ASSERT_TRUE(basic_struct->consistent(*basic_wide_struct, consistencyQos, unused));
    ASSERT_TRUE(basic_wide_struct->consistent(*basic_struct, consistencyQos, unused));

    // Don't allow type widening
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_prevent_type_widening = true;
    ASSERT_TRUE(basic_struct->consistent(*basic_wide_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_struct->consistent(*basic_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_struct->consistent(*basic_wide_bad_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_bad_struct->consistent(*basic_struct, consistencyQos, unused));

    // DISALLOW coercion
    consistencyQos.m_kind = DISALLOW_TYPE_COERCION;
    ASSERT_FALSE(basic_struct->consistent(*basic_wide_struct, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_struct->consistent(*basic_struct, consistencyQos, unused));
}

TEST_F(XTypesTests, StringMinimalCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_string = GetMinimalStringStructObject();
    const TypeObject* my_lstring = GetMinimalLargeStringStructObject();
    const TypeObject* my_wstring = GetMinimalWStringStructObject();
    const TypeObject* my_lwstring = GetMinimalLargeWStringStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_string->consistent(*my_lstring, consistencyQos, unused));
    ASSERT_TRUE(my_lstring->consistent(*my_string, consistencyQos, unused));
    ASSERT_TRUE(my_wstring->consistent(*my_lwstring, consistencyQos, unused));
    ASSERT_TRUE(my_lwstring->consistent(*my_wstring, consistencyQos, unused));
    ASSERT_FALSE(my_string->consistent(*my_wstring, consistencyQos, unused));
    ASSERT_FALSE(my_wstring->consistent(*my_string, consistencyQos, unused));
    ASSERT_FALSE(my_lstring->consistent(*my_lwstring, consistencyQos, unused));
    ASSERT_FALSE(my_lwstring->consistent(*my_lstring, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_string->consistent(*my_lstring, consistencyQos, unused));
    ASSERT_FALSE(my_lstring->consistent(*my_string, consistencyQos, unused));
    ASSERT_FALSE(my_wstring->consistent(*my_lwstring, consistencyQos, unused));
    ASSERT_FALSE(my_lwstring->consistent(*my_wstring, consistencyQos, unused));

    // Don't ignoring string bounds
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_string_bounds = false;
    ASSERT_FALSE(my_string->consistent(*my_lstring, consistencyQos, unused));
    ASSERT_TRUE(my_lstring->consistent(*my_string, consistencyQos, unused));
    ASSERT_FALSE(my_wstring->consistent(*my_lwstring, consistencyQos, unused));
    ASSERT_TRUE(my_lwstring->consistent(*my_wstring, consistencyQos, unused));
}

TEST_F(XTypesTests, StringCompleteCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_string = GetCompleteStringStructObject();
    const TypeObject* my_lstring = GetCompleteLargeStringStructObject();
    const TypeObject* my_wstring = GetCompleteWStringStructObject();
    const TypeObject* my_lwstring = GetCompleteLargeWStringStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_string->consistent(*my_lstring, consistencyQos, unused));
    ASSERT_TRUE(my_lstring->consistent(*my_string, consistencyQos, unused));
    ASSERT_TRUE(my_wstring->consistent(*my_lwstring, consistencyQos, unused));
    ASSERT_TRUE(my_lwstring->consistent(*my_wstring, consistencyQos, unused));
    ASSERT_FALSE(my_string->consistent(*my_wstring, consistencyQos, unused));
    ASSERT_FALSE(my_wstring->consistent(*my_string, consistencyQos, unused));
    ASSERT_FALSE(my_lstring->consistent(*my_lwstring, consistencyQos, unused));
    ASSERT_FALSE(my_lwstring->consistent(*my_lstring, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_string->consistent(*my_lstring, consistencyQos, unused));
    ASSERT_FALSE(my_lstring->consistent(*my_string, consistencyQos, unused));
    ASSERT_FALSE(my_wstring->consistent(*my_lwstring, consistencyQos, unused));
    ASSERT_FALSE(my_lwstring->consistent(*my_wstring, consistencyQos, unused));

    // Don't ignoring string bounds
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_string_bounds = false;
    ASSERT_FALSE(my_string->consistent(*my_lstring, consistencyQos, unused));
    ASSERT_TRUE(my_lstring->consistent(*my_string, consistencyQos, unused));
    ASSERT_FALSE(my_wstring->consistent(*my_lwstring, consistencyQos, unused));
    ASSERT_TRUE(my_lwstring->consistent(*my_wstring, consistencyQos, unused));
}

TEST_F(XTypesTests, ArrayMinimalCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_array = GetMinimalArrayStructObject();
    const TypeObject* my_array_equal = GetMinimalArrayStructEqualObject();
    const TypeObject* my_array_bad = GetMinimalArrayBadStructObject();
    const TypeObject* my_array_dims = GetMinimalArrayDimensionsStructObject();
    const TypeObject* my_array_size = GetMinimalArraySizeStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_array->consistent(*my_array_equal, consistencyQos, unused));
    ASSERT_FALSE(my_array->consistent(*my_array_bad, consistencyQos, unused));
    ASSERT_FALSE(my_array->consistent(*my_array_dims, consistencyQos, unused));
    ASSERT_FALSE(my_array->consistent(*my_array_size, consistencyQos, unused));
    ASSERT_TRUE(my_array_equal->consistent(*my_array, consistencyQos, unused));
    ASSERT_FALSE(my_array_bad->consistent(*my_array, consistencyQos, unused));
    ASSERT_FALSE(my_array_dims->consistent(*my_array, consistencyQos, unused));
    ASSERT_FALSE(my_array_size->consistent(*my_array, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_array->consistent(*my_array_equal, consistencyQos, unused));
}

TEST_F(XTypesTests, ArrayCompleteCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_array = GetCompleteArrayStructObject();
    const TypeObject* my_array_equal = GetCompleteArrayStructEqualObject();
    const TypeObject* my_array_bad = GetCompleteArrayBadStructObject();
    const TypeObject* my_array_dims = GetCompleteArrayDimensionsStructObject();
    const TypeObject* my_array_size = GetCompleteArraySizeStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_array->consistent(*my_array_equal, consistencyQos, unused));
    ASSERT_FALSE(my_array->consistent(*my_array_bad, consistencyQos, unused));
    ASSERT_FALSE(my_array->consistent(*my_array_dims, consistencyQos, unused));
    ASSERT_FALSE(my_array->consistent(*my_array_size, consistencyQos, unused));
    ASSERT_TRUE(my_array_equal->consistent(*my_array, consistencyQos, unused));
    ASSERT_FALSE(my_array_bad->consistent(*my_array, consistencyQos, unused));
    ASSERT_FALSE(my_array_dims->consistent(*my_array, consistencyQos, unused));
    ASSERT_FALSE(my_array_size->consistent(*my_array, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_array->consistent(*my_array_equal, consistencyQos, unused));
}

TEST_F(XTypesTests, SequenceMinimalCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_sequence = GetMinimalSequenceStructObject();
    const TypeObject* my_sequence_equal = GetMinimalSequenceStructEqualObject();
    const TypeObject* my_sequence_bad = GetMinimalSequenceBadStructObject();
    const TypeObject* my_sequence_bound = GetMinimalSequenceBoundsStructObject();
    const TypeObject* my_sequence_sequence = GetMinimalSequenceSequenceStructObject();
    const TypeObject* my_sequence_sequence_bound = GetMinimalSequenceSequenceBoundsStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_sequence->consistent(*my_sequence_equal, consistencyQos, unused));
    ASSERT_FALSE(my_sequence->consistent(*my_sequence_bad, consistencyQos, unused));
    ASSERT_TRUE(my_sequence->consistent(*my_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_equal->consistent(*my_sequence, consistencyQos, unused));
    ASSERT_FALSE(my_sequence_bad->consistent(*my_sequence, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_bound->consistent(*my_sequence, consistencyQos, unused));

    ASSERT_TRUE(my_sequence_sequence->consistent(*my_sequence_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_sequence_bound->consistent(*my_sequence_sequence, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_sequence->consistent(*my_sequence_equal, consistencyQos, unused));
    ASSERT_FALSE(my_sequence_equal->consistent(*my_sequence, consistencyQos, unused));
    ASSERT_TRUE(my_sequence->consistent(*my_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_bound->consistent(*my_sequence, consistencyQos, unused));

    ASSERT_TRUE(my_sequence_sequence->consistent(*my_sequence_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_sequence_bound->consistent(*my_sequence_sequence, consistencyQos, unused));

    // Don't ignoring sequence bounds
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = false;
    ASSERT_TRUE(my_sequence->consistent(*my_sequence_equal, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_equal->consistent(*my_sequence, consistencyQos, unused));
    ASSERT_FALSE(my_sequence->consistent(*my_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_bound->consistent(*my_sequence, consistencyQos, unused));

    ASSERT_FALSE(my_sequence_sequence->consistent(*my_sequence_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_sequence_bound->consistent(*my_sequence_sequence, consistencyQos, unused));
}

TEST_F(XTypesTests, SequenceCompleteCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_sequence = GetCompleteSequenceStructObject();
    const TypeObject* my_sequence_equal = GetCompleteSequenceStructEqualObject();
    const TypeObject* my_sequence_bad = GetCompleteSequenceBadStructObject();
    const TypeObject* my_sequence_bound = GetCompleteSequenceBoundsStructObject();
    const TypeObject* my_sequence_sequence = GetCompleteSequenceSequenceStructObject();
    const TypeObject* my_sequence_sequence_bound = GetCompleteSequenceSequenceBoundsStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_sequence->consistent(*my_sequence_equal, consistencyQos, unused));
    ASSERT_FALSE(my_sequence->consistent(*my_sequence_bad, consistencyQos, unused));
    ASSERT_TRUE(my_sequence->consistent(*my_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_equal->consistent(*my_sequence, consistencyQos, unused));
    ASSERT_FALSE(my_sequence_bad->consistent(*my_sequence, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_bound->consistent(*my_sequence, consistencyQos, unused));

    ASSERT_TRUE(my_sequence_sequence->consistent(*my_sequence_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_sequence_bound->consistent(*my_sequence_sequence, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_sequence->consistent(*my_sequence_equal, consistencyQos, unused));
    ASSERT_FALSE(my_sequence_equal->consistent(*my_sequence, consistencyQos, unused));
    ASSERT_TRUE(my_sequence->consistent(*my_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_bound->consistent(*my_sequence, consistencyQos, unused));

    ASSERT_TRUE(my_sequence_sequence->consistent(*my_sequence_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_sequence_bound->consistent(*my_sequence_sequence, consistencyQos, unused));

    // Don't ignoring sequence bounds
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = false;
    ASSERT_TRUE(my_sequence->consistent(*my_sequence_equal, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_equal->consistent(*my_sequence, consistencyQos, unused));
    ASSERT_FALSE(my_sequence->consistent(*my_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_bound->consistent(*my_sequence, consistencyQos, unused));

    ASSERT_FALSE(my_sequence_sequence->consistent(*my_sequence_sequence_bound, consistencyQos, unused));
    ASSERT_TRUE(my_sequence_sequence_bound->consistent(*my_sequence_sequence, consistencyQos, unused));
}

TEST_F(XTypesTests, MapMinimalCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_map = GetMinimalMapStructObject();
    const TypeObject* my_map_equal = GetMinimalMapStructEqualObject();
    const TypeObject* my_map_bad_key = GetMinimalMapBadKeyStructObject();
    const TypeObject* my_map_bad_elem = GetMinimalMapBadElemStructObject();
    const TypeObject* my_map_bound = GetMinimalMapBoundsStructObject();
    const TypeObject* my_map_map = GetMinimalMapMapStructObject();
    const TypeObject* my_map_map_bound = GetMinimalMapMapBoundsStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_map->consistent(*my_map_equal, consistencyQos, unused));
    ASSERT_FALSE(my_map->consistent(*my_map_bad_key, consistencyQos, unused));
    ASSERT_FALSE(my_map->consistent(*my_map_bad_elem, consistencyQos, unused));
    ASSERT_TRUE(my_map->consistent(*my_map_bound, consistencyQos, unused));

    ASSERT_TRUE(my_map_equal->consistent(*my_map, consistencyQos, unused));
    ASSERT_FALSE(my_map_bad_key->consistent(*my_map, consistencyQos, unused));
    ASSERT_FALSE(my_map_bad_elem->consistent(*my_map, consistencyQos, unused));
    ASSERT_TRUE(my_map_bound->consistent(*my_map, consistencyQos, unused));

    ASSERT_TRUE(my_map_map->consistent(*my_map_map_bound, consistencyQos, unused));
    ASSERT_TRUE(my_map_map_bound->consistent(*my_map_map, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_map->consistent(*my_map_equal, consistencyQos, unused));
    ASSERT_FALSE(my_map_equal->consistent(*my_map, consistencyQos, unused));
    ASSERT_TRUE(my_map->consistent(*my_map_bound, consistencyQos, unused));
    ASSERT_TRUE(my_map_bound->consistent(*my_map, consistencyQos, unused));

    ASSERT_TRUE(my_map_map->consistent(*my_map_map_bound, consistencyQos, unused));
    ASSERT_TRUE(my_map_map_bound->consistent(*my_map_map, consistencyQos, unused));

    // Don't ignoring map bounds, doesn't apply on maps
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = false;
    ASSERT_TRUE(my_map->consistent(*my_map_equal, consistencyQos, unused));
    ASSERT_TRUE(my_map_equal->consistent(*my_map, consistencyQos, unused));
    ASSERT_FALSE(my_map->consistent(*my_map_bound, consistencyQos, unused));
    ASSERT_FALSE(my_map_bound->consistent(*my_map, consistencyQos, unused));

    ASSERT_FALSE(my_map_map->consistent(*my_map_map_bound, consistencyQos, unused));
    ASSERT_FALSE(my_map_map_bound->consistent(*my_map_map, consistencyQos, unused));
}

TEST_F(XTypesTests, MapCompleteCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* my_map = GetCompleteMapStructObject();
    const TypeObject* my_map_equal = GetCompleteMapStructEqualObject();
    const TypeObject* my_map_bad_key = GetCompleteMapBadKeyStructObject();
    const TypeObject* my_map_bad_elem = GetCompleteMapBadElemStructObject();
    const TypeObject* my_map_bound = GetCompleteMapBoundsStructObject();
    const TypeObject* my_map_map = GetCompleteMapMapStructObject();
    const TypeObject* my_map_map_bound = GetCompleteMapMapBoundsStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(my_map->consistent(*my_map_equal, consistencyQos, unused));
    ASSERT_FALSE(my_map->consistent(*my_map_bad_key, consistencyQos, unused));
    ASSERT_FALSE(my_map->consistent(*my_map_bad_elem, consistencyQos, unused));
    ASSERT_TRUE(my_map->consistent(*my_map_bound, consistencyQos, unused));

    ASSERT_TRUE(my_map_equal->consistent(*my_map, consistencyQos, unused));
    ASSERT_FALSE(my_map_bad_key->consistent(*my_map, consistencyQos, unused));
    ASSERT_FALSE(my_map_bad_elem->consistent(*my_map, consistencyQos, unused));
    ASSERT_TRUE(my_map_bound->consistent(*my_map, consistencyQos, unused));

    ASSERT_TRUE(my_map_map->consistent(*my_map_map_bound, consistencyQos, unused));
    ASSERT_TRUE(my_map_map_bound->consistent(*my_map_map, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(my_map->consistent(*my_map_equal, consistencyQos, unused));
    ASSERT_FALSE(my_map_equal->consistent(*my_map, consistencyQos, unused));
    ASSERT_TRUE(my_map->consistent(*my_map_bound, consistencyQos, unused));
    ASSERT_TRUE(my_map_bound->consistent(*my_map, consistencyQos, unused));

    ASSERT_TRUE(my_map_map->consistent(*my_map_map_bound, consistencyQos, unused));
    ASSERT_TRUE(my_map_map_bound->consistent(*my_map_map, consistencyQos, unused));

    // Don't ignoring map bounds, doesn't apply on maps
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = false;
    ASSERT_TRUE(my_map->consistent(*my_map_equal, consistencyQos, unused));
    ASSERT_TRUE(my_map_equal->consistent(*my_map, consistencyQos, unused));
    ASSERT_FALSE(my_map->consistent(*my_map_bound, consistencyQos, unused));
    ASSERT_FALSE(my_map_bound->consistent(*my_map, consistencyQos, unused));

    ASSERT_FALSE(my_map_map->consistent(*my_map_map_bound, consistencyQos, unused));
    ASSERT_FALSE(my_map_map_bound->consistent(*my_map_map, consistencyQos, unused));
}

TEST_F(XTypesTests, SimpleUnionMinimalCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* basic_union = GetMinimalSimpleUnionStructObject();
    const TypeObject* basic_union_equal = GetMinimalSimpleUnionStructEqualObject();
    const TypeObject* basic_union_names = GetMinimalSimpleUnionNamesStructObject();
    const TypeObject* basic_union_type = GetMinimalSimpleTypeUnionStructObject();
    const TypeObject* basic_bad_union = GetMinimalSimpleBadUnionStructObject();
    const TypeObject* basic_bad_union_disc = GetMinimalSimplBadDiscUnionStructObject();
    const TypeObject* basic_wide_union = GetMinimalSimpleWideUnionStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(basic_union->consistent(*basic_union_equal, consistencyQos, unused));
    ASSERT_TRUE(basic_union->consistent(*basic_union_names, consistencyQos, unused));
    ASSERT_FALSE(basic_union->consistent(*basic_union_type, consistencyQos, unused));
    ASSERT_FALSE(basic_union->consistent(*basic_bad_union, consistencyQos, unused));
    ASSERT_FALSE(basic_union->consistent(*basic_bad_union_disc, consistencyQos, unused));
    ASSERT_TRUE(basic_union_equal->consistent(*basic_union, consistencyQos, unused));
    ASSERT_TRUE(basic_union_names->consistent(*basic_union, consistencyQos, unused));
    ASSERT_FALSE(basic_union_type->consistent(*basic_union, consistencyQos, unused));
    ASSERT_FALSE(basic_bad_union->consistent(*basic_union, consistencyQos, unused));
    ASSERT_FALSE(basic_bad_union_disc->consistent(*basic_union, consistencyQos, unused));


    ASSERT_TRUE(basic_union->consistent(*basic_wide_union, consistencyQos, unused));
    ASSERT_TRUE(basic_wide_union->consistent(*basic_union, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(basic_union->consistent(*basic_union_names, consistencyQos, unused));
    ASSERT_FALSE(basic_union->consistent(*basic_union_equal, consistencyQos, unused));
    ASSERT_FALSE(basic_union_equal->consistent(*basic_union, consistencyQos, unused));
    ASSERT_TRUE(basic_union->consistent(*basic_wide_union, consistencyQos, unused));

    // Don't allow type widening
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_prevent_type_widening = true;
    ASSERT_TRUE(basic_union->consistent(*basic_wide_union, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_union->consistent(*basic_union, consistencyQos, unused));

    // DISALLOW coercion
    consistencyQos.m_kind = DISALLOW_TYPE_COERCION;
    ASSERT_FALSE(basic_union->consistent(*basic_wide_union, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_union->consistent(*basic_union, consistencyQos, unused));
}

TEST_F(XTypesTests, SimpleUnionCompleteCoercion)
{
    // Get Struct TypeObjects (always test struct to test the aliases and types hierarchy)
    TypeConsistencyEnforcementQosPolicy consistencyQos;
    TypeConsistencyEnforcementQosPolicy unused;
    const TypeObject* basic_union = GetCompleteSimpleUnionStructObject();
    const TypeObject* basic_union_equal = GetCompleteSimpleUnionStructEqualObject();
    const TypeObject* basic_union_names = GetCompleteSimpleUnionNamesStructObject();
    const TypeObject* basic_union_type = GetCompleteSimpleTypeUnionStructObject();
    const TypeObject* basic_bad_union = GetCompleteSimpleBadUnionStructObject();
    const TypeObject* basic_bad_union_disc = GetCompleteSimplBadDiscUnionStructObject();
    const TypeObject* basic_wide_union = GetCompleteSimpleWideUnionStructObject();

    // Configure the TypeConsistencyEnforcementQos
    consistencyQos.m_force_type_validation = true;
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_ignore_sequence_bounds = true;
    consistencyQos.m_ignore_string_bounds = true;
    consistencyQos.m_prevent_type_widening = false;
    consistencyQos.m_kind = ALLOW_TYPE_COERCION;

    // Check results
    ASSERT_TRUE(basic_union->consistent(*basic_union_equal, consistencyQos, unused));
    ASSERT_TRUE(basic_union->consistent(*basic_union_names, consistencyQos, unused));
    ASSERT_FALSE(basic_union->consistent(*basic_union_type, consistencyQos, unused));
    ASSERT_FALSE(basic_union->consistent(*basic_bad_union, consistencyQos, unused));
    ASSERT_FALSE(basic_union->consistent(*basic_bad_union_disc, consistencyQos, unused));
    ASSERT_TRUE(basic_union_equal->consistent(*basic_union, consistencyQos, unused));
    ASSERT_TRUE(basic_union_names->consistent(*basic_union, consistencyQos, unused));
    ASSERT_FALSE(basic_union_type->consistent(*basic_union, consistencyQos, unused));
    ASSERT_FALSE(basic_bad_union->consistent(*basic_union, consistencyQos, unused));
    ASSERT_FALSE(basic_bad_union_disc->consistent(*basic_union, consistencyQos, unused));


    ASSERT_TRUE(basic_union->consistent(*basic_wide_union, consistencyQos, unused));
    ASSERT_TRUE(basic_wide_union->consistent(*basic_union, consistencyQos, unused));

    // Don't ignoring member names
    consistencyQos.m_ignore_member_names = false;
    ASSERT_FALSE(basic_union->consistent(*basic_union_names, consistencyQos, unused));
    ASSERT_FALSE(basic_union->consistent(*basic_union_equal, consistencyQos, unused));
    ASSERT_FALSE(basic_union_equal->consistent(*basic_union, consistencyQos, unused));
    ASSERT_TRUE(basic_union->consistent(*basic_wide_union, consistencyQos, unused));

    // Don't allow type widening
    consistencyQos.m_ignore_member_names = true;
    consistencyQos.m_prevent_type_widening = true;
    ASSERT_TRUE(basic_union->consistent(*basic_wide_union, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_union->consistent(*basic_union, consistencyQos, unused));

    // DISALLOW coercion
    consistencyQos.m_kind = DISALLOW_TYPE_COERCION;
    ASSERT_FALSE(basic_union->consistent(*basic_wide_union, consistencyQos, unused));
    ASSERT_FALSE(basic_wide_union->consistent(*basic_union, consistencyQos, unused));
}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

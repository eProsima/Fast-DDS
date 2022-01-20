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

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fastdds/topic/DDSSQLFilter/DDSFilterFactory.hpp"

#include "fastdds/dds/core/StackAllocatedSequence.hpp"

#include "data_types/ContentFilterTestTypePubSubTypes.h"
#include "data_types/ContentFilterTestTypeTypeObject.h"

namespace eprosima {
namespace fastdds {
namespace dds {

// Name of all the primitive fields used along the tests
static const std::vector<std::pair<std::string, std::string>> primitive_fields
{
    {"char_field",        "CHAR"},
    {"uint8_field",       "INT"},
    {"int16_field",       "INT"},
    {"uint16_field",      "INT"},
    {"int32_field",       "INT"},
    {"uint32_field",      "INT"},
    {"int64_field",       "INT"},
    {"uint64_field",      "INT"},
    {"float_field",       "FLOAT"},
    {"double_field",      "FLOAT"},
    {"long_double_field", "FLOAT"},
    {"bool_field",        "BOOL"},
    {"string_field",      "STRING"},
    {"enum_field",        "ENUM"},
    {"enum2_field",       "ENUM2"}
};

static const std::map<std::string, std::set<std::string>> type_compatibility_matrix
{
    {"BOOL", {"BOOL", "INT"}},
    {"INT", {"INT", "FLOAT"}},
    {"FLOAT", {"INT", "FLOAT"}},
    {"CHAR", {"CHAR", "STRING"}},
    {"STRING", {"CHAR", "STRING"}},
    {"ENUM", {"INT", "ENUM"}},
    {"ENUM2", {"INT", "ENUM2"}}
};

static bool are_types_compatible(
        const std::string& type1,
        const std::string& type2)
{
    return type_compatibility_matrix.at(type1).count(type2) > 0;
}

using DDSFilterFactory = DDSSQLFilter::DDSFilterFactory;
using ReturnCode_t = DDSFilterFactory::ReturnCode_t;

class DDSSQLFilterTests : public testing::Test
{
    DDSFilterFactory uut;
    ContentFilterTestTypePubSubType type_support;

protected:

    const ReturnCode_t ok_code = ReturnCode_t::RETCODE_OK;
    const ReturnCode_t bad_code = ReturnCode_t::RETCODE_BAD_PARAMETER;

    struct TestCase
    {
        std::string expression;
        std::vector<std::string> parameters;
        ReturnCode_t result;
    };

    void run(
            const TestCase& test)
    {
        IContentFilter* filter_instance = nullptr;

        StackAllocatedSequence<const char*, 10> params;
        LoanableCollection::size_type n_params = static_cast<LoanableCollection::size_type>(test.parameters.size());
        params.length(n_params);
        for (LoanableCollection::size_type n = 0; n < n_params; ++n)
        {
            params[n] = test.parameters[n].c_str();
        }

        auto ret = uut.create_content_filter("DDSSQL", "ContentFilterTestType", &type_support,
                        test.expression.c_str(), params, filter_instance);
        EXPECT_EQ(ret, test.result) << " failed for expression '" << test.expression << "'";
        if (ret == ok_code)
        {
            uut.delete_content_filter("DDSSQL", filter_instance);
        }
    }

    void run(
            const std::vector<TestCase>& test_cases)
    {
        for (const TestCase& tc : test_cases)
        {
            run(tc);
        }
    }

};

TEST_F(DDSSQLFilterTests, field_access)
{
    static const std::string max_array_size_str = std::to_string(max_array_size);
    static const std::string max_seq_size_str = std::to_string(max_seq_size);

    // On the following list, the '@' is used as a generator and is replaced by all the primitive field names
    // All the entries generate a filter expression check with the form <a> = <a>
    static const std::vector<std::pair<std::string, ReturnCode_t>> checks
    {
        {"other_field", bad_code},
        {"@", ok_code},
        {"@[0]", bad_code},
        {"@.other_field", bad_code},
        {"struct_field", bad_code},
        {"struct_field[0]", bad_code},
        {"struct_field.@", ok_code},
        {"struct_field.other_field", bad_code},
        {"struct_field.", bad_code},
        {"array_@", bad_code},
        {"array_@[0]", ok_code},
        {"array_@[" + max_array_size_str + "]", bad_code},
        {"array_struct_field", bad_code},
        {"array_struct_field.@", bad_code},
        {"array_struct_field[0]", bad_code},
        {"array_struct_field[0].@", ok_code},
        {"array_struct_field[0].other_field", bad_code},
        {"array_struct_field[" + max_array_size_str + "]", bad_code},
        {"array_struct_field[" + max_array_size_str + "].@", bad_code},
        {"bounded_sequence_@", bad_code},
        {"bounded_sequence_@[0]", ok_code},
        {"bounded_sequence_@[" + max_seq_size_str + "]", bad_code},
        {"bounded_sequence_struct_field", bad_code},
        {"bounded_sequence_struct_field.@", bad_code},
        {"bounded_sequence_struct_field[0]", bad_code},
        {"bounded_sequence_struct_field[0].@", ok_code},
        {"bounded_sequence_struct_field[0].other_field", bad_code},
        {"bounded_sequence_struct_field[" + max_seq_size_str + "]", bad_code},
        {"bounded_sequence_struct_field[" + max_seq_size_str + "].@", bad_code},
        {"unbounded_sequence_@", bad_code},
        {"unbounded_sequence_@[0]", ok_code},
        {"unbounded_sequence_@[" + max_seq_size_str + "]", ok_code},
        {"unbounded_sequence_struct_field", bad_code},
        {"unbounded_sequence_struct_field.@", bad_code},
        {"unbounded_sequence_struct_field[0]", bad_code},
        {"unbounded_sequence_struct_field[0].@", ok_code},
        {"unbounded_sequence_struct_field[0].other_field", bad_code},
        {"unbounded_sequence_struct_field[" + max_seq_size_str + "]", bad_code},
        {"unbounded_sequence_struct_field[" + max_seq_size_str + "].@", ok_code}
    };

    // Generate test cases from the templated checks
    std::vector<TestCase> test_cases;
    for (auto& item : checks)
    {
        auto pos = item.first.find('@');
        if (std::string::npos == pos)
        {
            std::string s = item.first;
            s = s + " = " + s;
            test_cases.emplace_back(TestCase{ s, {}, item.second });
        }
        else
        {
            for (const auto& field : primitive_fields)
            {
                std::string s = item.first;
                s.replace(pos, 1, field.first);
                s = s + " = " + s;
                test_cases.emplace_back(TestCase{ s, {}, item.second });
            }
        }
    }

    run(test_cases);
}

TEST_F(DDSSQLFilterTests, type_compatibility_like)
{
    // field1 LIKE field2
    {
        std::vector<TestCase> test_cases;
        for (const auto& field1 : primitive_fields)
        {
            for (const auto& field2 : primitive_fields)
            {
                bool ok = (field1.second == "STRING" || field2.second == "STRING") &&
                        are_types_compatible(field1.second, field2.second);
                ReturnCode_t ret = ok ? ok_code : bad_code;
                test_cases.emplace_back(TestCase{ field1.first + " LIKE " + field2.first, {}, ret });
            }
        }
        run(test_cases);
    }

    // field LIKE operand
    // operand LIKE field
    {
        static const std::vector<std::pair<std::string, ReturnCode_t>> checks
        {
            // string values
            {"'XYZ'", ok_code},
            {"'%XYZ'", ok_code},
            {"'XYZ%'", ok_code},
            {"'%X%Y%Z%'", ok_code},
            // Char values
            {"'A'", ok_code},
            {"'%'", ok_code},
            {"'''", ok_code},
            // Boolean values
            {"FALSE", bad_code},
            {"TRUE", bad_code},
            // Integer values
            {"1", bad_code},
            {"-1", bad_code},
            // Floating point values
            {"1.0", bad_code},
            {"-1.0", bad_code},
            {"1e2", bad_code},
            {"-1e2", bad_code},
        };

        std::vector<TestCase> test_cases;
        for (const auto& field : primitive_fields)
        {
            bool ok = field.second == "STRING";
            for (auto& check : checks)
            {
                ReturnCode_t ret = ok ? check.second : bad_code;

                // field LIKE operand
                test_cases.emplace_back(TestCase{ field.first + " LIKE " + check.first, {}, ret });
                test_cases.emplace_back(TestCase{ field.first + " LIKE %0", {check.first}, ret });
                test_cases.emplace_back(TestCase{ field.first + " LIKE %1", {check.first}, bad_code });
                test_cases.emplace_back(TestCase{ field.first + " LIKE %0", {}, bad_code });

                // operand LIKE field
                test_cases.emplace_back(TestCase{ check.first + " LIKE " + field.first, {}, ret });
                test_cases.emplace_back(TestCase{ "%0 LIKE " + field.first, {check.first}, ret });
                test_cases.emplace_back(TestCase{ "%1 LIKE " + field.first, {check.first}, bad_code });
                test_cases.emplace_back(TestCase{ "%0 LIKE " + field.first, {}, bad_code });
            }
        }

        run(test_cases);
    }
}

TEST_F(DDSSQLFilterTests, type_compatibility_compare)
{
    static const std::vector<std::string> operators
    {
        " = ", " > ", " >= ", " < ", " <= ", " <> ", " != "
    };

    // field1 OP field2
    {
        std::vector<TestCase> test_cases;
        for (const auto& field1 : primitive_fields)
        {
            for (const auto& field2 : primitive_fields)
            {
                bool ok = are_types_compatible(field1.second, field2.second);
                ReturnCode_t ret = ok ? ok_code : bad_code;
                for (const std::string& op : operators)
                {
                    test_cases.emplace_back(TestCase{ field1.first + op + field2.first, {}, ret });
                }
            }
        }
        run(test_cases);
    }

    // field OP operand
    // operand OP field
    {
        static const std::vector<std::pair<std::string, std::string>> checks
        {
            // string values
            {"'XYZ'", "STRING"},
            {"'%XYZ'", "STRING"},
            {"'XYZ%'", "STRING"},
            {"'%X%Y%Z%'", "STRING"},
            // Char values
            {"'A'", "CHAR"},
            {"'%'", "CHAR"},
            {"'''", "CHAR"},
            // Boolean values
            {"FALSE", "BOOL"},
            {"TRUE", "BOOL"},
            // Integer values
            {"1", "INT"},
            {"-1", "INT"},
            // Floating point values
            {"1.0", "FLOAT"},
            {"-1.0", "FLOAT"},
            {"1e2", "FLOAT"},
            {"-1e2", "FLOAT"},
            // Enum Color values
            {"'RED'", "ENUM"},
            {"'GREEN'", "ENUM"},
            {"'BLUE'", "ENUM"},
            // Enum Material values
            {"'WOOD'", "ENUM2"},
            {"'PLASTIC'", "ENUM2"},
            {"'METAL'", "ENUM2"},
            {"'CONCRETE'", "ENUM2"},
        };

        std::vector<TestCase> test_cases;
        for (const auto& field : primitive_fields)
        {
            for (auto& check : checks)
            {
                bool ok = are_types_compatible(field.second, check.second);
                ReturnCode_t ret = ok ? ok_code : bad_code;

                for (const std::string& op : operators)
                {
                    // field OP operand
                    test_cases.emplace_back(TestCase{ field.first + op + check.first, {}, ret });
                    test_cases.emplace_back(TestCase{ field.first + op + "%0", {check.first}, ret });
                    test_cases.emplace_back(TestCase{ field.first + op + "%1", {check.first}, bad_code });
                    test_cases.emplace_back(TestCase{ field.first + op + "%0", {}, bad_code });

                    // operand OP field
                    test_cases.emplace_back(TestCase{ check.first + op + field.first, {}, ret });
                    test_cases.emplace_back(TestCase{ "%0" + op + field.first, {check.first}, ret });
                    test_cases.emplace_back(TestCase{ "%1" + op + field.first, {check.first}, bad_code });
                    test_cases.emplace_back(TestCase{ "%0" + op + field.first, {}, bad_code });
                }
            }
        }

        run(test_cases);
    }
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    registerContentFilterTestTypeTypes();
    return RUN_ALL_TESTS();
}

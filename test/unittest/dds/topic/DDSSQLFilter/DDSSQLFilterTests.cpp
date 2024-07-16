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

#include <array>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fastdds/topic/DDSSQLFilter/DDSFilterFactory.hpp"

#include "fastdds/dds/core/StackAllocatedSequence.hpp"
#include "fastdds/dds/log/Log.hpp"

#include "data_types/ContentFilterTestType.hpp"
#include "data_types/ContentFilterTestTypePubSubTypes.hpp"
#include "data_types/ContentFilterTestTypeTypeObjectSupport.hpp"

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
    {"INT", {"BOOL", "INT", "FLOAT", "ENUM", "ENUM2"}},
    {"FLOAT", {"INT", "FLOAT"}},
    {"CHAR", {"CHAR", "STRING", "ENUM_STR", "ENUM2_STR"}},
    {"STRING", {"CHAR", "STRING", "ENUM_STR", "ENUM2_STR"}},
    {"ENUM", {"INT", "ENUM", "ENUM_STR"}},
    {"ENUM2", {"INT", "ENUM2", "ENUM2_STR"}},
    {"ENUM_STR", {"ENUM", "CHAR", "STRING"}},
    {"ENUM2_STR", {"ENUM2", "CHAR", "STRING"}}
};

static const std::vector<std::string> operators
{
    " = ", " > ", " >= ", " < ", " <= ", " <> ", " != "
};

static const std::vector<std::pair<std::string, std::string>> checks_compare
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
    {"false", "BOOL"},
    {"TRUE", "BOOL"},
    {"true", "BOOL"},
    // Integer values
    {"1", "INT"},
    {"-1", "INT"},
    {"0xabcdef", "INT"},
    {"-0xFEEDBAC0", "INT"},
    // Floating point values
    {"1.0", "FLOAT"},
    {"-1.0", "FLOAT"},
    {"1e2", "FLOAT"},
    {"-1e2", "FLOAT"},
    // Enum Color values
    {"'RED'", "ENUM_STR"},
    {"'GREEN'", "ENUM_STR"},
    {"'BLUE'", "ENUM_STR"},
    // Enum Material values
    {"'WOOD'", "ENUM2_STR"},
    {"'PLASTIC'", "ENUM2_STR"},
    {"'METAL'", "ENUM2_STR"},
    {"'CONCRETE'", "ENUM2_STR"},
};

static bool are_types_compatible(
        const std::string& type1,
        const std::string& type2)
{
    return type_compatibility_matrix.at(type1).count(type2) > 0;
}

using DDSFilterFactory = DDSSQLFilter::DDSFilterFactory;

static ReturnCode_t create_content_filter(
        DDSFilterFactory& factory,
        const std::string& expression,
        const std::vector<std::string>& parameters,
        const ContentFilterTestTypePubSubType* type,
        IContentFilter*& filter_instance)
{
    StackAllocatedSequence<const char*, 10> params;
    LoanableCollection::size_type n_params = static_cast<LoanableCollection::size_type>(parameters.size());
    params.length(n_params);
    for (LoanableCollection::size_type n = 0; n < n_params; ++n)
    {
        params[n] = parameters[n].c_str();
    }

    return factory.create_content_filter("DDSSQL", "ContentFilterTestType", type,
                   expression.c_str(), params, filter_instance);
}

class DDSSQLFilterTests : public testing::Test
{
    DDSFilterFactory uut;
    ContentFilterTestTypePubSubType type_support;

protected:

    void SetUp() override
    {
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair type_ids;
        register_ContentFilterTestType_type_identifier(type_ids);
        eprosima::fastdds::dds::Log::ClearConsumers();
    }

    const ReturnCode_t ok_code = RETCODE_OK;
    const ReturnCode_t bad_code = RETCODE_BAD_PARAMETER;

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
        auto ret = create_content_filter(uut, test.expression, test.parameters, &type_support, filter_instance);
        EXPECT_EQ(ret, test.result)
            << " failed for expression \"" << test.expression << "\" [" << test.parameters.size() << "]";
        if (ret == ok_code)
        {
            uut.delete_content_filter("DDSSQL", filter_instance);
        }
    }

    void run(
            const std::vector<TestCase>& test_cases)
    {
        std::cout << "Test Cases: " << test_cases.size() << std::endl;
        for (const TestCase& tc : test_cases)
        {
            run(tc);
        }
    }

};

TEST_F(DDSSQLFilterTests, empty_expression)
{
    TestCase empty{ "", {}, RETCODE_OK };
    run(empty);
}

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
    // operand LIKE operand
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
            {"0x1F", bad_code},
            {"-0x1f", bad_code},
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
                test_cases.emplace_back(TestCase{ check.first + " like " + field.first, {}, ret });
                test_cases.emplace_back(TestCase{ "%0 like " + field.first, {check.first}, ret });
                test_cases.emplace_back(TestCase{ "%1 like " + field.first, {check.first}, bad_code });
                test_cases.emplace_back(TestCase{ "%0 like " + field.first, {}, bad_code });
            }
        }

        for (const auto& check1 : checks)
        {
            for (auto& check2 : checks)
            {
                // op1 LIKE op2
                test_cases.emplace_back(TestCase{ check1.first + " LIKE " + check2.first, {}, bad_code });
                test_cases.emplace_back(TestCase{ check1.first + " LIKE %0", {check2.first}, bad_code });
                test_cases.emplace_back(TestCase{ check1.first + " LIKE %1", {check2.first}, bad_code });
                test_cases.emplace_back(TestCase{ check1.first + " LIKE %0", {}, bad_code });

                // op2 LIKE op1
                test_cases.emplace_back(TestCase{ check2.first + " LIKE " + check1.first, {}, bad_code });
                test_cases.emplace_back(TestCase{ "%0 LIKE " + check1.first, {check2.first}, bad_code });
                test_cases.emplace_back(TestCase{ "%1 LIKE " + check1.first, {check2.first}, bad_code });
                test_cases.emplace_back(TestCase{ "%0 LIKE " + check1.first, {}, bad_code });
            }
        }

        run(test_cases);
    }
}

TEST_F(DDSSQLFilterTests, type_compatibility_match)
{
    // field1 MATCH field2
    {
        std::vector<TestCase> test_cases;
        for (const auto& field1 : primitive_fields)
        {
            for (const auto& field2 : primitive_fields)
            {
                bool ok = (field1.second == "STRING" || field2.second == "STRING") &&
                        are_types_compatible(field1.second, field2.second);
                ReturnCode_t ret = ok ? ok_code : bad_code;
                test_cases.emplace_back(TestCase{ field1.first + " MATCH " + field2.first, {}, ret });
            }
        }
        run(test_cases);
    }

    // field MATCH operand
    // operand MATCH field
    // operand MATCH operand
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
            {"false", bad_code},
            {"true", bad_code},
            // Integer values
            {"1", bad_code},
            {"-1", bad_code},
            {"0xab", bad_code},
            {"-0xCd", bad_code},
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

                // field MATCH operand
                test_cases.emplace_back(TestCase{ field.first + " MATCH " + check.first, {}, ret });
                test_cases.emplace_back(TestCase{ field.first + " MATCH %0", {check.first}, ret });
                test_cases.emplace_back(TestCase{ field.first + " MATCH %1", {check.first}, bad_code });
                test_cases.emplace_back(TestCase{ field.first + " MATCH %0", {}, bad_code });

                // operand MATCH field
                test_cases.emplace_back(TestCase{ check.first + " match " + field.first, {}, ret });
                test_cases.emplace_back(TestCase{ "%0 match " + field.first, {check.first}, ret });
                test_cases.emplace_back(TestCase{ "%1 match " + field.first, {check.first}, bad_code });
                test_cases.emplace_back(TestCase{ "%0 match " + field.first, {}, bad_code });
            }
        }

        for (const auto& check1 : checks)
        {
            for (auto& check2 : checks)
            {
                // op1 MATCH op2
                test_cases.emplace_back(TestCase{ check1.first + " MATCH " + check2.first, {}, bad_code });
                test_cases.emplace_back(TestCase{ check1.first + " MATCH %0", {check2.first}, bad_code });
                test_cases.emplace_back(TestCase{ check1.first + " MATCH %1", {check2.first}, bad_code });
                test_cases.emplace_back(TestCase{ check1.first + " MATCH %0", {}, bad_code });

                // op2 MATCH op1
                test_cases.emplace_back(TestCase{ check2.first + " MATCH " + check1.first, {}, bad_code });
                test_cases.emplace_back(TestCase{ "%0 MATCH " + check1.first, {check2.first}, bad_code });
                test_cases.emplace_back(TestCase{ "%1 MATCH " + check1.first, {check2.first}, bad_code });
                test_cases.emplace_back(TestCase{ "%0 MATCH " + check1.first, {}, bad_code });
            }
        }

        run(test_cases);
    }
}

TEST_F(DDSSQLFilterTests, type_compatibility_compare_field_op_field)
{
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
}

TEST_F(DDSSQLFilterTests, type_compatibility_compare_field_op_operand)
{
    // field OP operand
    {
        std::vector<TestCase> test_cases;
        for (const auto& field : primitive_fields)
        {
            for (auto& check : checks_compare)
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
                }
            }
        }

        run(test_cases);
    }
}

TEST_F(DDSSQLFilterTests, type_compatibility_compare_operand_op_field)
{
    // operand OP field
    {
        std::vector<TestCase> test_cases;
        for (const auto& field : primitive_fields)
        {
            for (auto& check : checks_compare)
            {
                bool ok = are_types_compatible(field.second, check.second);
                ReturnCode_t ret = ok ? ok_code : bad_code;

                for (const std::string& op : operators)
                {
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

TEST_F(DDSSQLFilterTests, type_compatibility_compare_operand_op_operand)
{
    // operand OP operand
    {
        std::vector<TestCase> test_cases;
        for (const auto& check1 : checks_compare)
        {
            for (auto& check2 : checks_compare)
            {
                for (const std::string& op : operators)
                {
                    // op1 OP op2
                    test_cases.emplace_back(TestCase{ check1.first + op + check2.first, {}, bad_code });
                    test_cases.emplace_back(TestCase{ check1.first + op + "%0", {check2.first}, bad_code });
                    test_cases.emplace_back(TestCase{ check1.first + op + "%1", {check2.first}, bad_code });
                    test_cases.emplace_back(TestCase{ check1.first + op + "%0", {}, bad_code });

                    // op2 OP op1
                    test_cases.emplace_back(TestCase{ check2.first + op + check1.first, {}, bad_code });
                    test_cases.emplace_back(TestCase{ "%0" + op + check1.first, {check2.first}, bad_code });
                    test_cases.emplace_back(TestCase{ "%1" + op + check1.first, {check2.first}, bad_code });
                    test_cases.emplace_back(TestCase{ "%0" + op + check1.first, {}, bad_code });
                }
            }
        }

        run(test_cases);
    }
}

/**
 * Singleton that holds the serialized payloads to be evaluated
 */
class DDSSQLFilterValueGlobalData
{

public:

    static const std::vector<std::unique_ptr<IContentFilter::SerializedPayload>>& values()
    {
        static DDSSQLFilterValueGlobalData the_instance;
        return the_instance.values_;
    }

    static const std::array<std::array<std::array<bool, 5>, 5>, 6>& results()
    {
        static std::array<std::array<std::array<bool, 5>, 5>, 6> the_results;
        static bool generated = false;

        if (!generated)
        {
            generated = true;

            // EQ
            the_results[0][0] = {true, false, false, false, false};
            the_results[0][1] = {false, true, false, false, false};
            the_results[0][2] = {false, false, true, false, false};
            the_results[0][3] = {false, false, false, true, false};
            the_results[0][4] = {false, false, false, false, true};
            // NE
            the_results[1][0] = {false, true, true, true, true};
            the_results[1][1] = {true, false, true, true, true};
            the_results[1][2] = {true, true, false, true, true};
            the_results[1][3] = {true, true, true, false, true};
            the_results[1][4] = {true, true, true, true, false};
            // LT
            the_results[2][0] = {false, false, false, false, false};
            the_results[2][1] = {true, false, false, false, false};
            the_results[2][2] = {true, true, false, false, false};
            the_results[2][3] = {true, true, true, false, false};
            the_results[2][4] = {true, true, true, true, false};
            // LE
            the_results[3][0] = {true, false, false, false, false};
            the_results[3][1] = {true, true, false, false, false};
            the_results[3][2] = {true, true, true, false, false};
            the_results[3][3] = {true, true, true, true, false};
            the_results[3][4] = {true, true, true, true, true};
            // GT
            the_results[4][0] = {false, true, true, true, true};
            the_results[4][1] = {false, false, true, true, true};
            the_results[4][2] = {false, false, false, true, true};
            the_results[4][3] = {false, false, false, false, true};
            the_results[4][4] = {false, false, false, false, false};
            // GE
            the_results[5][0] = {true, true, true, true, true};
            the_results[5][1] = {false, true, true, true, true};
            the_results[5][2] = {false, false, true, true, true};
            the_results[5][3] = {false, false, false, true, true};
            the_results[5][4] = {false, false, false, false, true};
        }

        return the_results;
    }

    static const std::array<std::pair<std::string, std::string>, 6>& ops()
    {
        static const std::array < std::pair<std::string, std::string>, 6 > the_ops =
        {
            std::pair<std::string, std::string>{"=",  "eq"},
            std::pair<std::string, std::string>{"<>", "ne"},
            std::pair<std::string, std::string>{"<",  "lt"},
            std::pair<std::string, std::string>{"<=", "le"},
            std::pair<std::string, std::string>{">",  "gt"},
            std::pair<std::string, std::string>{">=", "ge"}
        };

        return the_ops;
    }

private:

    std::vector<std::unique_ptr<IContentFilter::SerializedPayload>> values_;

    DDSSQLFilterValueGlobalData()
    {
        std::array<ContentFilterTestType, 5> data;

        for (ContentFilterTestType& d : data)
        {
            d.bounded_sequence_struct_field().emplace_back();
            d.unbounded_sequence_struct_field().emplace_back();
        }

        add_char_values(data);
        add_uint8_values(data);
        add_int16_values(data);
        add_uint16_values(data);
        add_int32_values(data);
        add_uint32_values(data);
        add_int64_values(data);
        add_uint64_values(data);
        add_float_values(data);
        add_double_values(data);
        add_long_double_values(data);
        add_bool_values(data);
        add_string_values(data);
        add_enum_values(data);
        add_enum2_values(data);

        for (const ContentFilterTestType& d : data)
        {
            add_value(d);
        }
    }

    void add_value(
            const ContentFilterTestType& data)
    {
        static ContentFilterTestTypePubSubType type_support;
        auto data_ptr = const_cast<ContentFilterTestType*>(&data);
        auto data_size = type_support.calculate_serialized_size(data_ptr, fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        auto payload = new IContentFilter::SerializedPayload(data_size);
        values_.emplace_back(payload);
        type_support.serialize(data_ptr, *payload, fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    void add_char_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        std::array<char, 5> values{ ' ', 'A', 'Z', 'a', 'z' };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].char_field(values[i]);
            data[i].struct_field().char_field(values[i]);
            data[i].array_struct_field()[0].char_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].char_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].char_field(values[i]);
            data[i].array_char_field()[0] = values[i];
            data[i].bounded_sequence_char_field().push_back(values[i]);
            data[i].unbounded_sequence_char_field().push_back(values[i]);
        }
    }

    void add_uint8_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr uint8_t min = std::numeric_limits<uint8_t>::lowest();
        constexpr uint8_t max = std::numeric_limits<uint8_t>::max();

        std::array<uint8_t, 5> values{ min, max / 4, max / 3, max / 2, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].uint8_field(values[i]);
            data[i].struct_field().uint8_field(values[i]);
            data[i].array_struct_field()[0].uint8_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].uint8_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].uint8_field(values[i]);
            data[i].array_uint8_field()[0] = values[i];
            data[i].bounded_sequence_uint8_field().push_back(values[i]);
            data[i].unbounded_sequence_uint8_field().push_back(values[i]);
        }
    }

    void add_int16_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr int16_t min = std::numeric_limits<int16_t>::lowest();
        constexpr int16_t max = std::numeric_limits<int16_t>::max();

        std::array<int16_t, 5> values{ min, -100, 0, 100, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].int16_field(values[i]);
            data[i].struct_field().int16_field(values[i]);
            data[i].array_struct_field()[0].int16_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].int16_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].int16_field(values[i]);
            data[i].array_int16_field()[0] = values[i];
            data[i].bounded_sequence_int16_field().push_back(values[i]);
            data[i].unbounded_sequence_int16_field().push_back(values[i]);
        }
    }

    void add_uint16_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr uint16_t min = std::numeric_limits<uint16_t>::lowest();
        constexpr uint16_t max = std::numeric_limits<uint16_t>::max();

        std::array<uint16_t, 5> values{ min, max / 4, max / 3, max / 2, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].uint16_field(values[i]);
            data[i].struct_field().uint16_field(values[i]);
            data[i].array_struct_field()[0].uint16_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].uint16_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].uint16_field(values[i]);
            data[i].array_uint16_field()[0] = values[i];
            data[i].bounded_sequence_uint16_field().push_back(values[i]);
            data[i].unbounded_sequence_uint16_field().push_back(values[i]);
        }
    }

    void add_int32_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr int32_t min = std::numeric_limits<int32_t>::lowest();
        constexpr int32_t max = std::numeric_limits<int32_t>::max();

        std::array<int32_t, 5> values{ min, -100, 0, 100, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].int32_field(values[i]);
            data[i].struct_field().int32_field(values[i]);
            data[i].array_struct_field()[0].int32_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].int32_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].int32_field(values[i]);
            data[i].array_int32_field()[0] = values[i];
            data[i].bounded_sequence_int32_field().push_back(values[i]);
            data[i].unbounded_sequence_int32_field().push_back(values[i]);
        }
    }

    void add_uint32_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr uint32_t min = std::numeric_limits<uint32_t>::lowest();
        constexpr uint32_t max = (std::numeric_limits<uint32_t>::max)();

        std::array<uint32_t, 5> values{ min, max / 4, max / 3, max / 2, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].uint32_field(values[i]);
            data[i].struct_field().uint32_field(values[i]);
            data[i].array_struct_field()[0].uint32_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].uint32_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].uint32_field(values[i]);
            data[i].array_uint32_field()[0] = values[i];
            data[i].bounded_sequence_uint32_field().push_back(values[i]);
            data[i].unbounded_sequence_uint32_field().push_back(values[i]);
        }
    }

    void add_int64_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr int64_t min = std::numeric_limits<int64_t>::lowest();
        constexpr int64_t max = std::numeric_limits<int64_t>::max();

        std::array<int64_t, 5> values{ min, -100, 0, 100, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].int64_field(values[i]);
            data[i].struct_field().int64_field(values[i]);
            data[i].array_struct_field()[0].int64_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].int64_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].int64_field(values[i]);
            data[i].array_int64_field()[0] = values[i];
            data[i].bounded_sequence_int64_field().push_back(values[i]);
            data[i].unbounded_sequence_int64_field().push_back(values[i]);
        }
    }

    void add_uint64_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr uint64_t min = std::numeric_limits<uint64_t>::lowest();
        constexpr uint64_t max = std::numeric_limits<uint64_t>::max();

        std::array<uint64_t, 5> values{ min, max / 4, max / 3, max / 2, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].uint64_field(values[i]);
            data[i].struct_field().uint64_field(values[i]);
            data[i].array_struct_field()[0].uint64_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].uint64_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].uint64_field(values[i]);
            data[i].array_uint64_field()[0] = values[i];
            data[i].bounded_sequence_uint64_field().push_back(values[i]);
            data[i].unbounded_sequence_uint64_field().push_back(values[i]);
        }
    }

    void add_float_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr float min = std::numeric_limits<float>::lowest();
        constexpr float max = std::numeric_limits<float>::max();

        std::array<float, 5> values{ min, -3.14159f, 0.0f, 3.14159f, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].float_field(values[i]);
            data[i].struct_field().float_field(values[i]);
            data[i].array_struct_field()[0].float_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].float_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].float_field(values[i]);
            data[i].array_float_field()[0] = values[i];
            data[i].bounded_sequence_float_field().push_back(values[i]);
            data[i].unbounded_sequence_float_field().push_back(values[i]);
        }
    }

    void add_double_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr double min = std::numeric_limits<double>::lowest();
        constexpr double max = std::numeric_limits<double>::max();

        std::array<double, 5> values{ min, -3.14159, 0.0, 3.14159, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].double_field(values[i]);
            data[i].struct_field().double_field(values[i]);
            data[i].array_struct_field()[0].double_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].double_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].double_field(values[i]);
            data[i].array_double_field()[0] = values[i];
            data[i].bounded_sequence_double_field().push_back(values[i]);
            data[i].unbounded_sequence_double_field().push_back(values[i]);
        }
    }

    void add_long_double_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        constexpr long double min = std::numeric_limits<long double>::lowest();
        constexpr long double max = std::numeric_limits<long double>::max();

        std::array<long double, 5> values{ min, -3.14159l, 0.0, 3.14159l, max };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].long_double_field(values[i]);
            data[i].struct_field().long_double_field(values[i]);
            data[i].array_struct_field()[0].long_double_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].long_double_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].long_double_field(values[i]);
            data[i].array_long_double_field()[0] = values[i];
            data[i].bounded_sequence_long_double_field().push_back(values[i]);
            data[i].unbounded_sequence_long_double_field().push_back(values[i]);
        }
    }

    void add_bool_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        std::array<bool, 5> values{ false, false, true, true, true };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].bool_field(values[i]);
            data[i].struct_field().bool_field(values[i]);
            data[i].array_struct_field()[0].bool_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].bool_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].bool_field(values[i]);
            data[i].array_bool_field()[0] = values[i];
            data[i].bounded_sequence_bool_field().push_back(values[i]);
            data[i].unbounded_sequence_bool_field().push_back(values[i]);
        }
    }

    void add_string_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        std::array<std::string, 5> values{ "", "   ", " AA", " AZ", "ZZZ"};

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].string_field(values[i]);
            data[i].struct_field().string_field(values[i]);
            data[i].array_struct_field()[0].string_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].string_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].string_field(values[i]);
            data[i].array_string_field()[0] = values[i];
            data[i].bounded_sequence_string_field().push_back(values[i]);
            data[i].unbounded_sequence_string_field().push_back(values[i]);
        }
    }

    void add_enum_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        std::array<Color, 5> values{ Color::RED, Color::GREEN, Color::BLUE, Color::YELLOW, Color::MAGENTA };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].enum_field(values[i]);
            data[i].struct_field().enum_field(values[i]);
            data[i].array_struct_field()[0].enum_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].enum_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].enum_field(values[i]);
            data[i].array_enum_field()[0] = values[i];
            data[i].bounded_sequence_enum_field().push_back(values[i]);
            data[i].unbounded_sequence_enum_field().push_back(values[i]);
        }
    }

    void add_enum2_values(
            std::array<ContentFilterTestType, 5>& data)
    {
        std::array<Material, 5> values
        {
            Material::WOOD, Material::PLASTIC, Material::METAL, Material::CONCRETE, Material::STONE
        };

        for (size_t i = 0; i < values.size(); ++i)
        {
            data[i].enum2_field(values[i]);
            data[i].struct_field().enum2_field(values[i]);
            data[i].array_struct_field()[0].enum2_field(values[i]);
            data[i].bounded_sequence_struct_field()[0].enum2_field(values[i]);
            data[i].unbounded_sequence_struct_field()[0].enum2_field(values[i]);
            data[i].array_enum2_field()[0] = values[i];
            data[i].bounded_sequence_enum2_field().push_back(values[i]);
            data[i].unbounded_sequence_enum2_field().push_back(values[i]);
        }
    }

};

struct DDSSQLFilterValueParams
{
    std::string test_case_name;
    std::string expression;
    std::vector<std::string> params;
    std::vector<bool> samples_filtered;
};

class DDSSQLFilterValueTests : public testing::TestWithParam<DDSSQLFilterValueParams>
{
public:

    struct PrintToStringParamName
    {
        template<class ParamType>
        std::string operator ()(
                const ::testing::TestParamInfo<ParamType>& info) const
        {
            const auto& test_params = static_cast<const DDSSQLFilterValueParams&>(info.param);
            return test_params.test_case_name;
        }

    };

protected:

    void SetUp() override
    {
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair type_ids;
        register_ContentFilterTestType_type_identifier(type_ids);
        eprosima::fastdds::dds::Log::ClearConsumers();
    }

    DDSFilterFactory uut;
    ContentFilterTestTypePubSubType type_support;

    template<typename T>
    void perform_basic_check(
            const IContentFilter* filter_instance,
            const T& results,
            const std::vector<std::unique_ptr<IContentFilter::SerializedPayload>>& values)
    {
        for (size_t i = 0; i < values.size(); ++i)
        {
            IContentFilter::FilterSampleInfo info;
            IContentFilter::GUID_t guid;
            EXPECT_EQ(results[i], filter_instance->evaluate(*values[i], info, guid)) << "with i = " << i;
        }
    }

};

TEST_P(DDSSQLFilterValueTests, test_filtered_value)
{
    class ErrorLogChecker : public LogConsumer
    {
        void Consume(
                const Log::Entry& entry) override
        {
            EXPECT_NE(Log::Kind::Error, entry.kind);
        }

    };

    std::unique_ptr<ErrorLogChecker> consumer(new ErrorLogChecker());
    Log::RegisterConsumer(std::move(consumer));

    const auto& input = GetParam();
    const auto& values = DDSSQLFilterValueGlobalData::values();
    const auto& results = input.samples_filtered;
    ASSERT_EQ(results.size(), values.size());

    IContentFilter* filter_instance = nullptr;
    auto ret = create_content_filter(uut, input.expression, input.params, &type_support, filter_instance);
    EXPECT_EQ(RETCODE_OK, ret);
    ASSERT_NE(nullptr, filter_instance);

    perform_basic_check(filter_instance, results, values);

    ret = uut.delete_content_filter("DDSSQL", filter_instance);
    EXPECT_EQ(RETCODE_OK, ret);

    Log::Flush();
    Log::ClearConsumers();
}

TEST_F(DDSSQLFilterValueTests, test_compound_not)
{
    static const std::string expressions[2] =
    {
        "NOT (float_field = %0)",
        "not (float_field = %0)"
    };

    static const std::array<std::string, 5> param_values =
    {
        std::to_string(std::numeric_limits<float>::lowest()),
        "-3.14159",
        "0",
        "3.14159",
        std::to_string(std::numeric_limits<float>::max())
    };

    for (const std::string& expression : expressions)
    {
        IContentFilter* filter = nullptr;
        auto ret = create_content_filter(uut, expression, { param_values.back() }, &type_support, filter);
        EXPECT_EQ(RETCODE_OK, ret);
        ASSERT_NE(nullptr, filter);

        const auto& values = DDSSQLFilterValueGlobalData::values();
        std::array<bool, 5> results;
        StackAllocatedSequence<const char*, 1> params;
        params.length(1);

        ASSERT_EQ(results.size(), values.size());

        for (size_t i = 0; i < param_values.size(); ++i)
        {
            // Update parameter value
            params[0] = param_values[i].c_str();
            ret = uut.create_content_filter("DDSSQL", "ContentFilterTestType", &type_support, nullptr, params, filter);
            EXPECT_EQ(RETCODE_OK, ret);
            ASSERT_NE(nullptr, filter);

            // Update expected results
            results.fill(true);
            results[i] = false;
            for (size_t j = 0; j < values.size(); ++j)
            {
                IContentFilter::FilterSampleInfo info;
                IContentFilter::GUID_t guid;
                EXPECT_EQ(results[j], filter->evaluate(*values[j], info, guid)) << "with i = " << i << ", j = " << j;
            }
        }

        ret = uut.delete_content_filter("DDSSQL", filter);
        EXPECT_EQ(RETCODE_OK, ret);
    }
}

TEST_F(DDSSQLFilterValueTests, test_compound_and)
{
    static const std::string expressions[2] =
    {
        "float_field BETWEEN %0 AND %1 AND int16_field < 0",
        "float_field between %0 and %1 and int16_field < 0"
    };

    for (const std::string& expression : expressions)
    {
        IContentFilter* filter = nullptr;
        auto ret = create_content_filter(uut, expression, { "-3.14159", "3.14159" }, &type_support, filter);
        EXPECT_EQ(RETCODE_OK, ret);
        ASSERT_NE(nullptr, filter);

        const auto& values = DDSSQLFilterValueGlobalData::values();
        std::array<bool, 5> results{false, true, false, false, false};

        ASSERT_EQ(results.size(), values.size());
        perform_basic_check(filter, results, values);

        ret = uut.delete_content_filter("DDSSQL", filter);
        EXPECT_EQ(RETCODE_OK, ret);
    }
}

TEST_F(DDSSQLFilterValueTests, test_compound_or)
{
    static const std::string expressions[2] =
    {
        "float_field NOT BETWEEN %0 AND %1 OR int16_field > 0",
        "float_field not between %0 and %1 or int16_field > 0"
    };

    for (const std::string& expression : expressions)
    {
        IContentFilter* filter = nullptr;
        auto ret = create_content_filter(uut, expression, { "-3.14159", "3.14159" }, &type_support, filter);
        EXPECT_EQ(RETCODE_OK, ret);
        ASSERT_NE(nullptr, filter);

        const auto& values = DDSSQLFilterValueGlobalData::values();
        std::array<bool, 5> results{ true, false, false, true, true };

        ASSERT_EQ(results.size(), values.size());
        perform_basic_check(filter, results, values);

        ret = uut.delete_content_filter("DDSSQL", filter);
        EXPECT_EQ(RETCODE_OK, ret);
    }
}

TEST_F(DDSSQLFilterValueTests, test_update_params)
{
    static const std::string expression = "string_field MATCH %0 OR string_field LIKE %1";

    IContentFilter* filter = nullptr;
    auto ret = create_content_filter(uut, expression, { "'BBB'", "'X'" }, &type_support, filter);
    EXPECT_EQ(RETCODE_OK, ret);
    ASSERT_NE(nullptr, filter);

    const auto& values = DDSSQLFilterValueGlobalData::values();
    std::array<bool, 5> results{false, false, false, false, false};

    ASSERT_EQ(results.size(), values.size());
    perform_basic_check(filter, results, values);

    StackAllocatedSequence<const char*, 2> params;
    params.length(2);

    // Change %0 to a wrong parameter should preserve filter state and results
    params[0] = "'Z??"; // Wrong (missing ending quote)
    params[1] = "'X'";  // Unchanged
    ret = uut.create_content_filter("DDSSQL", "ContentFilterTestType", &type_support, nullptr, params, filter);
    EXPECT_EQ(RETCODE_BAD_PARAMETER, ret);
    perform_basic_check(filter, results, values);

    // Change %0 to a wrong parameter should preserve filter state and results
    params[0] = "'Z??"; // Wrong (missing ending quote)
    params[1] = "'%'";   // Changed
    ret = uut.create_content_filter("DDSSQL", "ContentFilterTestType", &type_support, nullptr, params, filter);
    EXPECT_EQ(RETCODE_BAD_PARAMETER, ret);
    perform_basic_check(filter, results, values);

    // Change %1 to a wrong parameter should preserve filter state and results
    params[0] = "'BBB'"; // Unchanged
    params[1] = "'";  // Wrong (missing ending quote)
    ret = uut.create_content_filter("DDSSQL", "ContentFilterTestType", &type_support, nullptr, params, filter);
    EXPECT_EQ(RETCODE_BAD_PARAMETER, ret);
    perform_basic_check(filter, results, values);

    // Change %1 to a wrong parameter should preserve filter state and results
    params[0] = "'.*'"; // Changed
    params[1] = "'";  // Wrong (missing ending quote)
    ret = uut.create_content_filter("DDSSQL", "ContentFilterTestType", &type_support, nullptr, params, filter);
    EXPECT_EQ(RETCODE_BAD_PARAMETER, ret);
    perform_basic_check(filter, results, values);

    // Correctly changing both parameters should change results
    params[0] = "'Z..'"; // Only last value matches
    params[1] = "''";  // Only first value matches
    results[0] = results[4] = true;
    ret = uut.create_content_filter("DDSSQL", "ContentFilterTestType", &type_support, nullptr, params, filter);
    EXPECT_EQ(RETCODE_OK, ret);
    perform_basic_check(filter, results, values);

    ret = uut.delete_content_filter("DDSSQL", filter);
    EXPECT_EQ(RETCODE_OK, ret);
}

static void add_test_filtered_value_inputs(
        const std::string& test_prefix,
        const std::string& field_name,
        const std::array<std::pair<std::string, std::string>, 5>& values,
        std::vector<DDSSQLFilterValueParams>& inputs,
        const std::array<std::array<std::array<bool, 5>, 5>, 6>& results)
{
    auto& ops = DDSSQLFilterValueGlobalData::ops();
    for (size_t i = 0; i < ops.size(); ++i)
    {
        auto& op = ops[i];
        for (size_t j = 0; j < values.size(); ++j)
        {
            auto& results_row = results[i][j];
            DDSSQLFilterValueParams input
            {
                test_prefix + "_" + op.second + "_" + values[j].second,
                field_name + " " + op.first + " " + values[j].first,
                {},
                {}
            };
            input.samples_filtered.assign(results_row.begin(), results_row.end());
            inputs.emplace_back(input);

            input.test_case_name += "_P0";
            input.expression = field_name + " " + op.first + " %0";
            input.params.push_back(values[j].first);
            inputs.emplace_back(input);
        }
    }
}

static void add_negative_test_filtered_value_inputs(
        const std::string& test_prefix,
        const std::string& field_name,
        const std::array<std::pair<std::string, std::string>, 5>& values,
        std::vector<DDSSQLFilterValueParams>& inputs)
{
    auto& ops = DDSSQLFilterValueGlobalData::ops();
    for (size_t i = 0; i < ops.size(); ++i)
    {
        auto& op = ops[i];
        for (size_t j = 0; j < values.size(); ++j)
        {
            DDSSQLFilterValueParams input
            {
                test_prefix + "_" + op.second + "_" + values[j].second,
                field_name + " " + op.first + " " + values[j].first,
                {},
                {}
            };
            input.samples_filtered.assign(5, false);
            inputs.emplace_back(input);

            input.test_case_name += "_P0";
            input.expression = field_name + " " + op.first + " %0";
            input.params.push_back(values[j].first);
            inputs.emplace_back(input);
        }
    }
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_inputs_given_values_and_results(
        const std::string& field_name,
        const std::array<std::pair<std::string, std::string>, 5>& values,
        const std::array<std::array<std::array<bool, 5>, 5>, 6>& results = DDSSQLFilterValueGlobalData::results())
{
    std::string bounded_seq_name = "bounded_sequence_" + field_name;
    std::string unbounded_seq_name = "unbounded_sequence_" + field_name;
    std::string array_struct_name = "array_struct_field[0]." + field_name;
    std::string bounded_seq_struct_name = "bounded_sequence_struct_field[0]." + field_name;
    std::string unbounded_seq_struct_name = "unbounded_sequence_struct_field[0]." + field_name;
    std::string neg_bounded_seq_struct_name = "bounded_sequence_struct_field[2]." + field_name;
    std::string neg_unbounded_seq_struct_name = "unbounded_sequence_struct_field[2]." + field_name;

    std::vector<DDSSQLFilterValueParams> inputs;
    add_test_filtered_value_inputs("plain_field", field_name, values, inputs, results);
    add_test_filtered_value_inputs("in_struct", "struct_field." + field_name, values, inputs, results);
    add_test_filtered_value_inputs("in_array_struct", array_struct_name, values, inputs, results);
    add_test_filtered_value_inputs("in_bounded_sequence_struct", bounded_seq_struct_name, values, inputs, results);
    add_negative_test_filtered_value_inputs("neg_bounded_seq_struct", neg_bounded_seq_struct_name, values, inputs);
    add_test_filtered_value_inputs("in_unbounded_sequence_struct", unbounded_seq_struct_name, values, inputs, results);
    add_negative_test_filtered_value_inputs("neg_unbounded_seq_struct", neg_unbounded_seq_struct_name, values, inputs);
    add_test_filtered_value_inputs("array", "array_" + field_name + "[0]", values, inputs, results);
    add_test_filtered_value_inputs("bounded_sequence", bounded_seq_name + "[0]", values, inputs, results);
    add_negative_test_filtered_value_inputs("neg_bounded_sequence", bounded_seq_name + "[2]", values, inputs);
    add_test_filtered_value_inputs("unbounded_sequence", unbounded_seq_name + "[0]", values, inputs, results);
    add_negative_test_filtered_value_inputs("neg_unbounded_sequence", unbounded_seq_name + "[2]", values, inputs);
    return inputs;
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_char_inputs()
{
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{"' '", "minus_2"},
        std::pair<std::string, std::string>{"'A'", "minus_1"},
        std::pair<std::string, std::string>{"'Z'", "0"},
        std::pair<std::string, std::string>{"'a'", "plus_1"},
        std::pair<std::string, std::string>{"'z'", "plus_2"}
    };

    return get_test_filtered_value_inputs_given_values_and_results("char_field", values);
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_string_inputs()
{
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{"''", "minus_2"},
        std::pair<std::string, std::string>{"'   '", "minus_1"},
        std::pair<std::string, std::string>{"' AA'", "0"},
        std::pair<std::string, std::string>{"' AZ'", "plus_1"},
        std::pair<std::string, std::string>{"'ZZZ'", "plus_2"}
    };

    // Adding standard tests
    std::vector<DDSSQLFilterValueParams> inputs;
    inputs = get_test_filtered_value_inputs_given_values_and_results("string_field", values);

    // Adding tests for LIKE operator
    DDSSQLFilterValueParams input;
    input.test_case_name = "like_any_percent";
    input.expression = "string_field LIKE '%'";
    input.samples_filtered.assign(5, true);
    inputs.push_back(input);

    input.test_case_name = "like_any_star";
    input.expression = "string_field LIKE '*'";
    input.samples_filtered.assign(5, true);
    inputs.push_back(input);

    input.test_case_name = "like_space_percent";
    input.expression = "string_field LIKE ' %'";
    input.samples_filtered.assign({ false, true, true, true, false });
    inputs.push_back(input);

    input.test_case_name = "like_space_star";
    input.expression = "string_field LIKE ' *'";
    input.samples_filtered.assign({ false, true, true, true, false });
    inputs.push_back(input);

    input.test_case_name = "like_A_question";
    input.expression = "string_field LIKE '?A?'";
    input.samples_filtered.assign({ false, false, true, true, false });
    inputs.push_back(input);

    input.test_case_name = "like_A_underscore";
    input.expression = "string_field LIKE '_A_'";
    input.samples_filtered.assign({ false, false, true, true, false });
    inputs.push_back(input);

    input.test_case_name = "like_exact_empty";
    input.expression = "string_field LIKE ''";
    input.samples_filtered.assign({ true, false, false, false, false });
    inputs.push_back(input);

    input.test_case_name = "like_exact_ZZZ";
    input.expression = "string_field LIKE 'ZZZ'";
    input.samples_filtered.assign({ false, false, false, false, true });
    inputs.push_back(input);

    input.test_case_name = "like_exact_none";
    input.expression = "string_field LIKE 'BBB'";
    input.samples_filtered.assign({ false, false, false, false, false });
    inputs.push_back(input);

    // Adding tests for MATCH operator
    input.test_case_name = "match_any";
    input.expression = "string_field match '.*'";
    input.samples_filtered.assign(5, true);
    inputs.push_back(input);

    input.test_case_name = "match_space";
    input.expression = "string_field match ' .*'";
    input.samples_filtered.assign({ false, true, true, true, false });
    inputs.push_back(input);

    input.test_case_name = "match_A";
    input.expression = "string_field match '.A.'";
    input.samples_filtered.assign({ false, false, true, true, false });
    inputs.push_back(input);

    input.test_case_name = "match_exact_empty";
    input.expression = "string_field match ''";
    input.samples_filtered.assign({ true, false, false, false, false });
    inputs.push_back(input);

    input.test_case_name = "match_exact_ZZZ";
    input.expression = "string_field match 'ZZZ'";
    input.samples_filtered.assign({ false, false, false, false, true });
    inputs.push_back(input);

    input.test_case_name = "match_exact_none";
    input.expression = "string_field match 'BBB'";
    input.samples_filtered.assign({ false, false, false, false, false });
    inputs.push_back(input);

    input.test_case_name = "match_range";
    input.expression = "string_field match '([A-Z])+'";
    input.samples_filtered.assign({ false, false, false, false, true });
    inputs.push_back(input);

    input.test_case_name = "match_space_and_range";
    input.expression = "string_field match ' ([A-Z])+'";
    input.samples_filtered.assign({ false, false, true, true, false });
    inputs.push_back(input);

    return inputs;
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_boolean_inputs()
{
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{"FALSE", "minus_2"},
        std::pair<std::string, std::string>{"FALSE", "minus_1"},
        std::pair<std::string, std::string>{"TRUE", "0"},
        std::pair<std::string, std::string>{"TRUE", "plus_1"},
        std::pair<std::string, std::string>{"TRUE", "plus_2"}
    };

    std::array<std::array<std::array<bool, 5>, 5>, 6> results;
    // EQ
    results[0][0] = { true, true, false, false, false };
    results[0][1] = { true, true, false, false, false };
    results[0][2] = { false, false, true, true, true };
    results[0][3] = { false, false, true, true, true };
    results[0][4] = { false, false, true, true, true };
    // NE
    results[1][0] = { false, false, true, true, true };
    results[1][1] = { false, false, true, true, true };
    results[1][2] = { true, true, false, false, false };
    results[1][3] = { true, true, false, false, false };
    results[1][4] = { true, true, false, false, false };
    // LT
    results[2][0] = { false, false, false, false, false };
    results[2][1] = { false, false, false, false, false };
    results[2][2] = { true, true, false, false, false };
    results[2][3] = { true, true, false, false, false };
    results[2][4] = { true, true, false, false, false };
    // LE
    results[3][0] = { true, true, false, false, false };
    results[3][1] = { true, true, false, false, false };
    results[3][2] = { true, true, true, true, true };
    results[3][3] = { true, true, true, true, true };
    results[3][4] = { true, true, true, true, true };
    // GT
    results[4][0] = { false, false, true, true, true };
    results[4][1] = { false, false, true, true, true };
    results[4][2] = { false, false, false, false, false };
    results[4][3] = { false, false, false, false, false };
    results[4][4] = { false, false, false, false, false };
    // GE
    results[5][0] = { true, true, true, true, true };
    results[5][1] = { true, true, true, true, true };
    results[5][2] = { false, false, true, true, true };
    results[5][3] = { false, false, true, true, true };
    results[5][4] = { false, false, true, true, true };

    return get_test_filtered_value_inputs_given_values_and_results("bool_field", values, results);
}

template<typename T>
static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_unsigned_integer_inputs(
        const std::string& field_name)
{
    constexpr T max = std::numeric_limits<T>::max();
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{std::to_string(std::numeric_limits<T>::lowest()), "minus_2"},
        std::pair<std::string, std::string>{std::to_string(max / 4), "minus_1"},
        std::pair<std::string, std::string>{std::to_string(max / 3), "0"},
        std::pair<std::string, std::string>{std::to_string(max / 2), "plus_1"},
        std::pair<std::string, std::string>{std::to_string(max), "plus_2"}
    };

    return get_test_filtered_value_inputs_given_values_and_results(field_name, values);
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_uint8_inputs()
{
    return get_test_filtered_value_unsigned_integer_inputs<uint8_t>("uint8_field");
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_uint16_inputs()
{
    return get_test_filtered_value_unsigned_integer_inputs<uint16_t>("uint16_field");
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_uint32_inputs()
{
    return get_test_filtered_value_unsigned_integer_inputs<uint32_t>("uint32_field");
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_uint64_inputs()
{
    return get_test_filtered_value_unsigned_integer_inputs<uint64_t>("uint64_field");
}

template<typename T>
static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_signed_integer_inputs(
        const std::string& field_name)
{
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{std::to_string(std::numeric_limits<T>::lowest()), "minus_2"},
        std::pair<std::string, std::string>{"-100", "minus_1"},
        std::pair<std::string, std::string>{"0", "0"},
        std::pair<std::string, std::string>{"100", "plus_1"},
        std::pair<std::string, std::string>{std::to_string(std::numeric_limits<T>::max()), "plus_2"}
    };

    return get_test_filtered_value_inputs_given_values_and_results(field_name, values);
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_int16_inputs()
{
    return get_test_filtered_value_signed_integer_inputs<int16_t>("int16_field");
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_int32_inputs()
{
    return get_test_filtered_value_signed_integer_inputs<int32_t>("int32_field");
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_int64_inputs()
{
    return get_test_filtered_value_signed_integer_inputs<int64_t>("int64_field");
}

template<typename T>
static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_floating_point_inputs(
        const std::string& field_name)
{
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{std::to_string(std::numeric_limits<T>::lowest()), "minus_2"},
        std::pair<std::string, std::string>{"-3.14159", "minus_1"},
        std::pair<std::string, std::string>{"0", "0"},
        std::pair<std::string, std::string>{"3.14159", "plus_1"},
        std::pair<std::string, std::string>{std::to_string(std::numeric_limits<T>::max()), "plus_2"}
    };

    return get_test_filtered_value_inputs_given_values_and_results(field_name, values);
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_float_inputs()
{
    return get_test_filtered_value_floating_point_inputs<float>("float_field");
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_double_inputs()
{
    return get_test_filtered_value_floating_point_inputs<double>("double_field");
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_long_double_inputs()
{
    return get_test_filtered_value_floating_point_inputs<long double>("long_double_field");
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_enum_inputs()
{
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{"'RED'", "minus_2"},
        std::pair<std::string, std::string>{"'GREEN'", "minus_1"},
        std::pair<std::string, std::string>{"'BLUE'", "0"},
        std::pair<std::string, std::string>{"'YELLOW'", "plus_1"},
        std::pair<std::string, std::string>{"'MAGENTA'", "plus_2"}
    };

    return get_test_filtered_value_inputs_given_values_and_results("enum_field", values);
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_enum2_inputs()
{
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{"'WOOD'", "minus_2"},
        std::pair<std::string, std::string>{"'PLASTIC'", "minus_1"},
        std::pair<std::string, std::string>{"'METAL'", "0"},
        std::pair<std::string, std::string>{"'CONCRETE'", "plus_1"},
        std::pair<std::string, std::string>{"'STONE'", "plus_2"}
    };

    return get_test_filtered_value_inputs_given_values_and_results("enum2_field", values);
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_promotion_inputs()
{
    std::vector<DDSSQLFilterValueParams> inputs;

    DDSSQLFilterValueParams input;

    // BOOLEAN promotions
    input.test_case_name = "bool_field_int16_field";
    input.expression = "bool_field > int16_field";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "bool_field_int32_field";
    input.expression = "bool_field > int32_field";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "bool_field_int64_field";
    input.expression = "bool_field > int64_field";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "bool_field_signed_int_constant";
    input.expression = "bool_field > -1";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "bool_field_signed_hex_constant";
    input.expression = "bool_field > -0x1";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "bool_field_uint8_field";
    input.expression = "bool_field < uint8_field";
    input.samples_filtered.assign({false, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "bool_field_uint16_field";
    input.expression = "bool_field < uint16_field";
    input.samples_filtered.assign({false, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "bool_field_uint32_field";
    input.expression = "bool_field < uint32_field";
    input.samples_filtered.assign({false, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "bool_field_uint64_field";
    input.expression = "bool_field < uint64_field";
    input.samples_filtered.assign({false, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "bool_field_unsigned_int_constant";
    input.expression = "bool_field < 1";
    input.samples_filtered.assign({true, true, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "bool_field_unsigned_hex_constant";
    input.expression = "bool_field < 0x1";
    input.samples_filtered.assign({true, true, false, false, false});
    inputs.push_back(input);

    // INT - INT promotions
    input.test_case_name = "int16_field_int32_field";
    input.expression = "int16_field > int32_field";
    input.samples_filtered.assign({true, false, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "int16_field_int64_field";
    input.expression = "int16_field > int64_field";
    input.samples_filtered.assign({true, false, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "int32_field_int64_field";
    input.expression = "int32_field > int64_field";
    input.samples_filtered.assign({true, false, false, false, false});
    inputs.push_back(input);

    // INT - UINT promotions
    input.test_case_name = "int16_field_uint8_field";
    input.expression = "int16_field < uint8_field";
    input.samples_filtered.assign({true, true, true, true, false});
    inputs.push_back(input);

    input.test_case_name = "int16_field_uint16_field";
    input.expression = "int16_field < uint16_field";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "int16_field_uint32_field";
    input.expression = "int16_field < uint32_field";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "int16_field_uint64_field";
    input.expression = "int16_field < uint64_field";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "int32_field_uint8_field";
    input.expression = "int32_field < uint8_field";
    input.samples_filtered.assign({true, true, true, true, false});
    inputs.push_back(input);

    input.test_case_name = "int32_field_uint16_field";
    input.expression = "int32_field < uint16_field";
    input.samples_filtered.assign({true, true, true, true, false});
    inputs.push_back(input);

    input.test_case_name = "int32_field_uint32_field";
    input.expression = "int32_field < uint32_field";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "int32_field_uint64_field";
    input.expression = "int32_field < uint64_field";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "int64_field_uint8_field";
    input.expression = "int64_field < uint8_field";
    input.samples_filtered.assign({true, true, true, true, false});
    inputs.push_back(input);

    input.test_case_name = "int64_field_uint16_field";
    input.expression = "int64_field < uint16_field";
    input.samples_filtered.assign({true, true, true, true, false});
    inputs.push_back(input);

    input.test_case_name = "int64_field_uint32_field";
    input.expression = "int64_field < uint32_field";
    input.samples_filtered.assign({true, true, true, true, false});
    inputs.push_back(input);

    input.test_case_name = "int64_field_uint64_field";
    input.expression = "int64_field < uint64_field";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "int16_field_unsigned_int_constant";
    input.expression = "int16_field < 1";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "int16_field_unsigned_hex_constant";
    input.expression = "int16_field < 0x1";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "int32_field_unsigned_hex_constant";
    input.expression = "int32_field < 0x1";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "int64_field_unsigned_int_constant";
    input.expression = "int64_field < 1";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "int64_field_unsigned_hex_constant";
    input.expression = "int64_field < 0x1";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    // INT - FLOAT promotions
    input.test_case_name = "int16_field_float_field";
    input.expression = "int16_field < float_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int32_field_float_field";
    input.expression = "int32_field < float_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int64_field_float_field";
    input.expression = "int64_field < float_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int16_field_double_field";
    input.expression = "int16_field < double_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int32_field_double_field";
    input.expression = "int32_field < double_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int64_field_double_field";
    input.expression = "int64_field < double_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int16_field_long_double_field";
    input.expression = "int16_field < long_double_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int32_field_long_double_field";
    input.expression = "int32_field < long_double_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int64_field_long_double_field";
    input.expression = "int64_field < long_double_field";
    input.samples_filtered.assign({false, true, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "int16_field_float_constant";
    input.expression = "int16_field < -0.5";
    input.samples_filtered.assign({true, true, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "int32_field_float_constant";
    input.expression = "int32_field < -0.5";
    input.samples_filtered.assign({true, true, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "int64_field_float_constant";
    input.expression = "int64_field < -0.5";
    input.samples_filtered.assign({true, true, false, false, false});
    inputs.push_back(input);

    // UINT - FLOAT promotions
    input.test_case_name = "uint8_field_float_field";
    input.expression = "uint8_field < float_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint16_field_float_field";
    input.expression = "uint16_field < float_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint32_field_float_field";
    input.expression = "uint32_field < float_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint64_field_float_field";
    input.expression = "uint64_field < float_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint16_field_double_field";
    input.expression = "uint16_field < double_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint32_field_double_field";
    input.expression = "uint32_field < double_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint64_field_double_field";
    input.expression = "uint64_field < double_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint16_field_long_double_field";
    input.expression = "uint16_field < long_double_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint32_field_long_double_field";
    input.expression = "uint32_field < long_double_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint64_field_long_double_field";
    input.expression = "uint64_field < long_double_field";
    input.samples_filtered.assign({false, false, false, false, true});
    inputs.push_back(input);

    input.test_case_name = "uint16_field_float_constant";
    input.expression = "uint16_field > 3E4";
    input.samples_filtered.assign({false, false, false, true, true});
    inputs.push_back(input);

    input.test_case_name = "uint32_field_float_constant";
    input.expression = "uint32_field > 2E9";
    input.samples_filtered.assign({false, false, false, true, true});
    inputs.push_back(input);

    input.test_case_name = "uint64_field_float_constant";
    input.expression = "uint64_field > 8e18";
    input.samples_filtered.assign({false, false, false, true, true});
    inputs.push_back(input);

    // ENUM - INT promotions
    input.test_case_name = "enum_field_int16_field";
    input.expression = "enum_field > int16_field";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "enum_field_int32_field";
    input.expression = "enum_field > int32_field";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "enum_field_int64_field";
    input.expression = "enum_field > int64_field";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "enum_field_signed_int_constant";
    input.expression = "enum_field > -1";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "enum_field_signed_hex_constant";
    input.expression = "enum_field > -0x1";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    // ENUM - UINT promotions
    input.test_case_name = "enum_field_uint8_field";
    input.expression = "enum_field >= uint8_field";
    input.samples_filtered.assign({true, false, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "enum_field_uint16_field";
    input.expression = "enum_field >= uint16_field";
    input.samples_filtered.assign({true, false, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "enum_field_uint32_field";
    input.expression = "enum_field >= uint32_field";
    input.samples_filtered.assign({true, false, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "enum_field_uint64_field";
    input.expression = "enum_field >= uint64_field";
    input.samples_filtered.assign({true, false, false, false, false});
    inputs.push_back(input);

    input.test_case_name = "enum_field_unsigned_int_constant";
    input.expression = "enum_field > 1";
    input.samples_filtered.assign({false, false, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "enum_field_unsigned_hex_constant";
    input.expression = "enum_field > 0x1";
    input.samples_filtered.assign({false, false, true, true, true});
    inputs.push_back(input);

    // CHAR - STRING promotions
    input.test_case_name = "char_field_string_field";
    input.expression = "string_field < char_field";
    input.samples_filtered.assign({true, true, true, true, true});
    inputs.push_back(input);

    input.test_case_name = "char_field_string_constant";
    input.expression = "char_field < 'ZZ'";
    input.samples_filtered.assign({true, true, true, false, false});
    inputs.push_back(input);

    input.test_case_name = "string_field_char_constant";
    input.expression = "string_field < 'Z'";
    input.samples_filtered.assign({true, true, true, true, false});
    inputs.push_back(input);

    return inputs;
}

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsChar,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_char_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsString,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_string_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsBool,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_boolean_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsUInt8,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_uint8_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsUInt16,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_uint16_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsUInt32,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_uint32_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsUInt64,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_uint64_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsInt16,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_int16_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsInt32,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_int32_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsInt64,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_int64_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsFloat,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_float_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsDouble,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_double_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsLongDouble,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_long_double_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsEnum,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_enum_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsEnum2,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_enum2_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsPromotion,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_promotion_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

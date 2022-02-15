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
#include "fastdds/dds/log/Log.hpp"

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
    {"INT", {"BOOL", "INT", "FLOAT", "ENUM", "ENUM2"}},
    {"FLOAT", {"INT", "FLOAT"}},
    {"CHAR", {"CHAR", "STRING", "ENUM_STR", "ENUM2_STR"}},
    {"STRING", {"CHAR", "STRING", "ENUM_STR", "ENUM2_STR"}},
    {"ENUM", {"INT", "ENUM", "ENUM_STR"}},
    {"ENUM2", {"INT", "ENUM2", "ENUM2_STR"}},
    {"ENUM_STR", {"ENUM", "CHAR", "STRING"}},
    {"ENUM2_STR", {"ENUM2", "CHAR", "STRING"}}
};

static bool are_types_compatible(
        const std::string& type1,
        const std::string& type2)
{
    return type_compatibility_matrix.at(type1).count(type2) > 0;
}

using DDSFilterFactory = DDSSQLFilter::DDSFilterFactory;
using ReturnCode_t = DDSFilterFactory::ReturnCode_t;

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
        for (const TestCase& tc : test_cases)
        {
            run(tc);
        }
    }

};

TEST_F(DDSSQLFilterTests, empty_expression)
{
    TestCase empty{ "", {}, ReturnCode_t::RETCODE_OK };
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
    // operand OP operand
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
            {"'RED'", "ENUM_STR"},
            {"'GREEN'", "ENUM_STR"},
            {"'BLUE'", "ENUM_STR"},
            // Enum Material values
            {"'WOOD'", "ENUM2_STR"},
            {"'PLASTIC'", "ENUM2_STR"},
            {"'METAL'", "ENUM2_STR"},
            {"'CONCRETE'", "ENUM2_STR"},
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

        for (const auto& check1 : checks)
        {
            for (auto& check2 : checks)
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
struct DDSSQLFilterValueGlobalData
{
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
        create_sample_minus_2();
        create_sample_minus_1();
        create_sample_0();
        create_sample_plus_1();
        create_sample_plus_2();
    }

    void add_value(
            const ContentFilterTestType& data)
    {
        static ContentFilterTestTypePubSubType type_support;
        auto data_ptr = const_cast<ContentFilterTestType*>(&data);
        auto data_size = type_support.getSerializedSizeProvider(data_ptr)();
        auto payload = new IContentFilter::SerializedPayload(data_size);
        values_.emplace_back(payload);
        type_support.serialize(data_ptr, payload);
    }

    void create_sample_0()
    {
        ContentFilterTestType data;
        data.float_field(0.0f);
        data.struct_field().float_field(0.0f);
        data.array_float_field()[0] = 0.0f;
        data.bounded_sequence_float_field().push_back(0.0f);
        data.unbounded_sequence_float_field().push_back(0.0f);
        add_value(data);
    }

    void create_sample_minus_1()
    {
        ContentFilterTestType data;
        data.float_field(-3.14159f);
        data.struct_field().float_field(-3.14159f);
        data.array_float_field()[0] = -3.14159f;
        data.bounded_sequence_float_field().push_back(-3.14159f);
        data.unbounded_sequence_float_field().push_back(-3.14159f);
        add_value(data);
    }

    void create_sample_minus_2()
    {
        ContentFilterTestType data;
        data.float_field(-1e38f);
        data.struct_field().float_field(-1e38f);
        data.array_float_field()[0] = -1e38f;
        data.bounded_sequence_float_field().push_back(-1e38f);
        data.unbounded_sequence_float_field().push_back(-1e38f);
        add_value(data);
    }

    void create_sample_plus_1()
    {
        ContentFilterTestType data;
        data.float_field(3.14159f);
        data.struct_field().float_field(3.14159f);
        data.array_float_field()[0] = 3.14159f;
        data.bounded_sequence_float_field().push_back(3.14159f);
        data.unbounded_sequence_float_field().push_back(3.14159f);
        add_value(data);
    }

    void create_sample_plus_2()
    {
        ContentFilterTestType data;
        data.float_field(1e38f);
        data.struct_field().float_field(1e38f);
        data.array_float_field()[0] = 1e38f;
        data.bounded_sequence_float_field().push_back(1e38f);
        data.unbounded_sequence_float_field().push_back(1e38f);
        add_value(data);
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

    DDSFilterFactory uut;
    ContentFilterTestTypePubSubType type_support;
};

TEST_P(DDSSQLFilterValueTests, test_filtered_value)
{
    const auto& input = GetParam();
    const auto& values = DDSSQLFilterValueGlobalData::values();
    const auto& results = input.samples_filtered;
    ASSERT_EQ(results.size(), values.size());

    IContentFilter* filter_instance = nullptr;
    auto ret = create_content_filter(uut, input.expression, input.params, &type_support, filter_instance);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, ret);
    ASSERT_NE(nullptr, filter_instance);

    for (size_t i = 0; i < values.size(); ++i)
    {
        IContentFilter::FilterSampleInfo info;
        IContentFilter::GUID_t guid;
        EXPECT_EQ(results[i], filter_instance->evaluate(*values[i], info, guid)) << "with i = " << i;
    }

    ret = uut.delete_content_filter("DDSSQL", filter_instance);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, ret);
}

static void add_test_filtered_value_inputs(
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
            auto& results = DDSSQLFilterValueGlobalData::results()[i][j];
            DDSSQLFilterValueParams input
            {
                test_prefix + "_" + op.second + "_" + values[j].second,
                field_name + " " + op.first + " " + values[j].first,
                {},
                { results.begin(), results.end() }
            };
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
                { false, false, false, false, false }
            };
            inputs.emplace_back(input);

            input.test_case_name += "_P0";
            input.expression = field_name + " " + op.first + " %0";
            input.params.push_back(values[j].first);
            inputs.emplace_back(input);
        }
    }
}

static std::vector<DDSSQLFilterValueParams> get_test_filtered_value_float_inputs()
{
    static const std::array<std::pair<std::string, std::string>, 5> values =
    {
        std::pair<std::string, std::string>{"-1e38", "minus_2"},
        std::pair<std::string, std::string>{"-3.14159", "minus_1"},
        std::pair<std::string, std::string>{"0", "0"},
        std::pair<std::string, std::string>{"3.14159", "plus_1"},
        std::pair<std::string, std::string>{"1e38", "plus_2"}
    };

    std::vector<DDSSQLFilterValueParams> inputs;
    add_test_filtered_value_inputs("plain_field", "float_field", values, inputs);
    add_test_filtered_value_inputs("in_struct", "struct_field.float_field", values, inputs);
    add_test_filtered_value_inputs("array", "array_float_field[0]", values, inputs);
    add_test_filtered_value_inputs("bounded_sequence", "bounded_sequence_float_field[0]", values, inputs);
    add_negative_test_filtered_value_inputs("neg_bounded_sequence", "bounded_sequence_float_field[2]", values, inputs);
    add_test_filtered_value_inputs("unbounded_sequence", "unbounded_sequence_float_field[0]", values, inputs);
    add_negative_test_filtered_value_inputs("neg_unbounded_sequence", "unbounded_sequence_float_field[2]", values, inputs);
    return inputs;
}

INSTANTIATE_TEST_SUITE_P(
    DDSSQLFilterValueTestsFloats,
    DDSSQLFilterValueTests,
    ::testing::ValuesIn(get_test_filtered_value_float_inputs()),
    DDSSQLFilterValueTests::PrintToStringParamName());

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    registerContentFilterTestTypeTypes();
    eprosima::fastdds::dds::Log::ClearConsumers();
    return RUN_ALL_TESTS();
}

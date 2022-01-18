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
static const std::vector<std::string> primitive_fields
{
    "char_field",
    "uint8_field",
    "int16_field",
    "uint16_field",
    "int32_field",
    "uint32_field",
    "int64_field",
    "uint64_field",
    "float_field",
    "double_field",
    "long_double_field",
    "bool_field",
    "string_field",
    "enum_field",
    "enum2_field"
};

using DDSFilterFactory = DDSSQLFilter::DDSFilterFactory;
using ReturnCode_t = DDSFilterFactory::ReturnCode_t;

class DDSSQLFilterTests : public testing::Test
{
    DDSFilterFactory uut;
    ContentFilterTestTypePubSubType type_support;

protected:

    struct TestCase
    {
        std::string expression;
        std::vector<std::string> parameters{};
        ReturnCode_t result{};
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
        if (ret == ReturnCode_t::RETCODE_OK)
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
        {"other_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"@", ReturnCode_t::RETCODE_OK},
        {"@[0]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"@.other_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"struct_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"struct_field[0]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"struct_field.@", ReturnCode_t::RETCODE_OK},
        {"struct_field.other_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"struct_field.", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"array_@", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"array_@[0]", ReturnCode_t::RETCODE_OK},
        {"array_@[" + max_array_size_str + "]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"array_struct_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"array_struct_field.@", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"array_struct_field[0]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"array_struct_field[0].@", ReturnCode_t::RETCODE_OK},
        {"array_struct_field[0].other_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"array_struct_field[" + max_array_size_str + "]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"array_struct_field[" + max_array_size_str + "].@", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"bounded_sequence_@", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"bounded_sequence_@[0]", ReturnCode_t::RETCODE_OK},
        {"bounded_sequence_@[" + max_seq_size_str + "]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"bounded_sequence_struct_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"bounded_sequence_struct_field.@", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"bounded_sequence_struct_field[0]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"bounded_sequence_struct_field[0].@", ReturnCode_t::RETCODE_OK},
        {"bounded_sequence_struct_field[0].other_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"bounded_sequence_struct_field[" + max_seq_size_str + "]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"bounded_sequence_struct_field[" + max_seq_size_str + "].@", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"unbounded_sequence_@", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"unbounded_sequence_@[0]", ReturnCode_t::RETCODE_OK},
        {"unbounded_sequence_@[" + max_seq_size_str + "]", ReturnCode_t::RETCODE_OK},
        {"unbounded_sequence_struct_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"unbounded_sequence_struct_field.@", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"unbounded_sequence_struct_field[0]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"unbounded_sequence_struct_field[0].@", ReturnCode_t::RETCODE_OK},
        {"unbounded_sequence_struct_field[0].other_field", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"unbounded_sequence_struct_field[" + max_seq_size_str + "]", ReturnCode_t::RETCODE_BAD_PARAMETER},
        {"unbounded_sequence_struct_field[" + max_seq_size_str + "].@", ReturnCode_t::RETCODE_OK}
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
            for (const std::string& field : primitive_fields)
            {
                std::string s = item.first;
                s.replace(pos, 1, field);
                s = s + " = " + s;
                test_cases.emplace_back(TestCase{ s, {}, item.second });
            }
        }
    }

    run(test_cases);
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

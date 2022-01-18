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

using DDSFilterFactory = DDSSQLFilter::DDSFilterFactory;
using ReturnCode_t = DDSFilterFactory::ReturnCode_t;

class DDSSQLFilterTests : public testing::Test
{
    DDSFilterFactory uut;
    ContentFilterTestTypePubSubType type_support;

protected:

    struct TestCase
    {
        const char* expression = nullptr;
        std::vector<const char*> parameters{};
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
            params[n] = test.parameters[n];
        }

        auto ret = uut.create_content_filter("DDSSQL", "ContentFilterTestType", &type_support,
                        test.expression, params, filter_instance);
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

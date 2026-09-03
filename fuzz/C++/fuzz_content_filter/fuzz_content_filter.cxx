// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * Fuzzer for the DDS SQL content filter (DDSSQLFilter).
 *
 * A DataReader may attach a content filter to its subscription. The filter is
 * propagated to the remote DataWriter inside the reader's discovery data --
 * ReaderProxyData reads it with
 * ParameterSerializer<ContentFilterProperty>::read_from_cdr_message() -- and
 * the *writer* then parses and evaluates it in order to filter samples before
 * sending them. Both the expression and its parameters therefore arrive from a
 * remote participant, and both are fuzzed here.
 *
 * fuzz_proxy_cdr already deserializes ReaderProxyData, so the expression
 * *string* reaches the writer today, but nothing exercises the PEGTL grammar
 * and the semantic conversion that consume it.
 *
 * Coverage targets:
 * - DDSFilterFactory::validate_filter_expression() and the expression pool
 * - parser::parse_filter_expression() (DDSFilterGrammar, PEGTL)
 * - convert_tree(): DDSFilterField, DDSFilterValue, DDSFilterCompoundCondition,
 *   DDSFilterPredicate
 * - parameter binding via DDSFilterValue::set_value()
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "fastdds/topic/DDSSQLFilter/DDSFilterFactory.hpp"

#include "fastdds/dds/core/StackAllocatedSequence.hpp"
#include "fastdds/dds/log/Log.hpp"

#include "data_types/ContentFilterTestType.hpp"
#include "data_types/ContentFilterTestTypePubSubTypes.hpp"
#include "data_types/ContentFilterTestTypeTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;
using DDSFilterFactory = DDSSQLFilter::DDSFilterFactory;

namespace {

// The expression is capped well below DEFAULT_MAX_EXPRESSION_LENGTH (16 KiB);
// anything longer is rejected by validate_filter_expression() without being
// parsed, so it would only slow the fuzzer down.
constexpr size_t kMaxInputSize = 4096;

// Matches the sequence used by the upstream DDSSQLFilter unit test.
constexpr size_t kMaxParameters = 10;

ContentFilterTestTypePubSubType* g_type_support = nullptr;

} // namespace

extern "C" int LLVMFuzzerInitialize(
        int* argc,
        char*** argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    xtypes::TypeIdentifierPair type_ids;
    register_ContentFilterTestType_type_identifier(type_ids);

    Log::ClearConsumers();

    g_type_support = new ContentFilterTestTypePubSubType();

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    if (size == 0 || size > kMaxInputSize)
    {
        return 0;
    }

    const std::string input(reinterpret_cast<const char*>(data), size);
    std::vector<std::string> parameters;
    std::string expression;

    size_t start = input.find('\n');
    if (std::string::npos == start)
    {
        expression = input;
    }
    else
    {
        expression = input.substr(0, start);
        ++start;
        while (start <= input.size() && parameters.size() < kMaxParameters)
        {
            const size_t nl = input.find('\n', start);
            if (std::string::npos == nl)
            {
                parameters.emplace_back(input.substr(start));
                break;
            }
            parameters.emplace_back(input.substr(start, nl - start));
            start = nl + 1;
        }
    }

    StackAllocatedSequence<const char*, kMaxParameters> params;
    const LoanableCollection::size_type n_params =
            static_cast<LoanableCollection::size_type>(parameters.size());
    params.length(n_params);
    for (LoanableCollection::size_type n = 0; n < n_params; ++n)
    {
        params[n] = parameters[n].c_str();
    }

    DDSFilterFactory factory{ DDSFilterFactory::DEFAULT_MAX_SUBEXPRESSIONS,
                              DDSFilterFactory::DEFAULT_MAX_EXPRESSION_LENGTH };

    IContentFilter* filter_instance = nullptr;
    if (RETCODE_OK == factory.create_content_filter(
                "DDSSQL", "ContentFilterTestType", g_type_support,
                expression.c_str(), params, filter_instance))
    {
        factory.delete_content_filter("DDSSQL", filter_instance);
    }

    return 0;
}

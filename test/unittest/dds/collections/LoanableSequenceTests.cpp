// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <array>
#include <atomic>
#include <memory>
#include <string>

#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using test_size_type = LoanableCollection::size_type;

static constexpr test_size_type num_test_elements = 10u;
static const std::array<int, num_test_elements> result_values =
{
    2, 2, 4, 6, 10, 16, 26, 42, 68, 110
};

template<typename T, test_size_type num_items = num_test_elements>
struct StackAllocatedBuffer
{
    constexpr test_size_type size() const
    {
        return num_items;
    }

    StackAllocatedBuffer()
    {
        for (test_size_type n = 0; n < num_items; ++n)
        {
            buffer[n] = &elems[n];
        }
    }

    std::array<T, num_items> elems;
    void* buffer[num_items];
};

// Declare test sequence using declaration macro
FASTDDS_SEQUENCE(TestSeq, int);

void clear_values(
        TestSeq& seq)
{
    // Keep current length for later
    test_size_type len = seq.length();
    // Set length to current maximum
    seq.length(seq.maximum());
    // Clear all values
    for (test_size_type n = seq.length(); n > 0; )
    {
        seq[--n] = 0;
    }
    // Restore original length
    seq.length(len);
}

void set_result_values(
        TestSeq& seq)
{
    ASSERT_TRUE(seq.length(num_test_elements));
    test_size_type n = 0;
    for (int v : result_values)
    {
        seq[n++] = v;
    }
}

void check_result(
        const TestSeq& result,
        test_size_type num_elems = num_test_elements)
{
    EXPECT_EQ(num_elems, result.length());
    for (test_size_type n = 0; n < num_elems; ++n)
    {
        EXPECT_EQ(result_values[n], result[n]);
    }
}

TEST(LoanableSequenceTests, construct)
{
    // Check post-conditions of default constructor
    {
        TestSeq uut;
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }

    // Check post-conditions of constructor with maximum
    {
        TestSeq uut(num_test_elements);
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }

    // Check maximum of 0 behaves as default
    {
        TestSeq uut(0u);
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }
}

TEST(LoanableSequenceTests, copy_construct)
{
    // Helper loaned sequence (max = num_test_elements, len = 0)
    StackAllocatedBuffer<int> stack;
    TestSeq loaned;
    loaned.loan(stack.buffer, stack.size(), 0);

    // Helper owned sequence (max = num_test_elements, len = 0)
    TestSeq owned(num_test_elements);

    // Copy-constructing an empty sequence behaves as default constructor
    {
        TestSeq empty;
        TestSeq uut(empty);
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }

    // Copy-constructing an empty allocated sequence behaves as default constructor
    {
        TestSeq uut(owned);
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }

    // Copy-constructing an empty loaned sequence behaves as default constructor
    {
        TestSeq uut(loaned);
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }

    // Give length and values to sequences
    set_result_values(owned);
    set_result_values(loaned);

    // Copy-constructing a non-empty allocated sequence allocates and copies
    {
        TestSeq uut(owned);
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_NE(owned.buffer(), uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        check_result(uut);
    }

    // Copy-constructing a non-empty loaned sequence allocates and copies
    {
        TestSeq uut(loaned);
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_NE(loaned.buffer(), uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        check_result(uut);
    }

    // Reduce length of sequences
    EXPECT_TRUE(owned.length(1u));
    EXPECT_GT(owned.maximum(), owned.length());
    EXPECT_TRUE(loaned.length(1u));
    EXPECT_GT(loaned.maximum(), loaned.length());

    // Copy-constructing a non-empty allocated sequence with max > len allocates and copies
    {
        TestSeq uut(owned);
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_NE(owned.buffer(), uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(1u, uut.maximum());
        check_result(uut, 1u);
    }

    // Copy-constructing a non-empty loaned sequence with max > len allocates and copies
    {
        TestSeq uut(loaned);
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_NE(loaned.buffer(), uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(1u, uut.maximum());
        check_result(uut, 1u);
    }

    // Return loan to avoid warning on destructor
    loaned.unloan();
}

TEST(LoanableSequenceTests, copy_assign)
{
    // Helper loaned sequence (max = num_test_elements, len = 0)
    StackAllocatedBuffer<int> stack;
    TestSeq loaned;
    loaned.loan(stack.buffer, stack.size(), 0);

    // Helper owned sequence (max = num_test_elements, len = 0)
    TestSeq owned(num_test_elements);

    // Copying an empty sequence behaves as default constructor
    {
        TestSeq empty;
        TestSeq uut = empty;
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }

    // Copying an empty allocated sequence behaves as default constructor
    {
        TestSeq uut = owned;
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }

    // Copying an empty loaned sequence behaves as default constructor
    {
        TestSeq uut = loaned;
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
    }

    // Give length and values to sequences
    set_result_values(owned);
    set_result_values(loaned);

    // Copying a non-empty allocated sequence allocates and copies
    {
        TestSeq uut = owned;
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_NE(owned.buffer(), uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        check_result(uut);
    }

    // Copying a non-empty loaned sequence allocates and copies
    {
        TestSeq uut = loaned;
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_NE(loaned.buffer(), uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        check_result(uut);
    }

    // Reduce length of sequences
    EXPECT_TRUE(owned.length(1u));
    EXPECT_GT(owned.maximum(), owned.length());
    EXPECT_TRUE(loaned.length(1u));
    EXPECT_GT(loaned.maximum(), loaned.length());

    // Copying a non-empty allocated sequence with max > len allocates and copies
    {
        TestSeq uut = owned;
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_NE(owned.buffer(), uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(1u, uut.maximum());
        check_result(uut, 1u);
    }

    // Copying a non-empty loaned sequence with max > len allocates and copies
    {
        TestSeq uut = loaned;
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_NE(loaned.buffer(), uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(1u, uut.maximum());
        check_result(uut, 1u);
    }

    // Copying loaned into allocated changes length
    EXPECT_TRUE(loaned.length(num_test_elements));
    EXPECT_NE(loaned.length(), owned.length());
    clear_values(owned);
    owned = loaned;
    EXPECT_EQ(loaned.length(), owned.length());
    EXPECT_NE(loaned.buffer(), owned.buffer());
    EXPECT_TRUE(owned.has_ownership());
    check_result(owned, num_test_elements);

    // Copying allocated into loaned releases loan
    EXPECT_TRUE(owned.length(1u));
    loaned = owned;
    EXPECT_EQ(loaned.length(), owned.length());
    EXPECT_NE(loaned.buffer(), owned.buffer());
    EXPECT_TRUE(loaned.has_ownership());
    EXPECT_EQ(1u, loaned.maximum());
    check_result(loaned, 1u);

    // Copying a bigger collection makes collection grow
    EXPECT_TRUE(owned.length(num_test_elements));
    loaned = owned;
    EXPECT_EQ(loaned.length(), owned.length());
    EXPECT_NE(loaned.buffer(), owned.buffer());
    EXPECT_TRUE(loaned.has_ownership());
    EXPECT_EQ(num_test_elements, loaned.maximum());
    check_result(loaned, num_test_elements);
}

TEST(LoanableSequenceTests, loan_unloan)
{
    StackAllocatedBuffer<int> stack;
    test_size_type max = 0u, len = 0u;
    void** result_buffer;

    {
        // Create default sequence
        TestSeq uut;
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.maximum());
        EXPECT_EQ(0u, uut.length());

        // Check that loan cannot be returned
        EXPECT_EQ(nullptr, uut.unloan());
        EXPECT_EQ(nullptr, uut.unloan(max, len));

        // Allocate data
        EXPECT_TRUE(uut.length(num_test_elements));
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        EXPECT_EQ(num_test_elements, uut.length());

        // Check that loan cannot be returned
        EXPECT_EQ(nullptr, uut.unloan());
        EXPECT_EQ(nullptr, uut.unloan(max, len));

        // Check that loan cannot be performed
        EXPECT_FALSE(uut.loan(stack.buffer, num_test_elements, 0u));
    }

    {
        // Create allocated sequence
        TestSeq uut(num_test_elements);
        EXPECT_NE(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        EXPECT_EQ(0u, uut.length());

        // Check that loan cannot be returned
        EXPECT_EQ(nullptr, uut.unloan());
        EXPECT_EQ(nullptr, uut.unloan(max, len));

        // Check that loan cannot be performed
        EXPECT_FALSE(uut.loan(stack.buffer, num_test_elements, 0u));
    }

    // Note: When uut is deleted upon exiting its scope, a warning log will be produced.
    // We will check the generation of that warning with a custom consumer.
    Log::SetVerbosity(Log::Kind::Warning);
    std::atomic_bool log_has_been_detected;
    log_has_been_detected.store(false);

    class CustomLogConsumer : public LogConsumer
    {
        const char* log_function = "eprosima::fastdds::dds::LoanableSequence<int>::~LoanableSequence";

    public:

        CustomLogConsumer(
                std::atomic_bool* log_detected_ptr)
            : log_detected_(log_detected_ptr)
        {
        }

        void Consume(
                const Log::Entry& entry) override
        {
            if ((Log::Kind::Warning == entry.kind) &&
                    (std::string(entry.context.function) == log_function))
            {
                log_detected_->store(true, std::memory_order::memory_order_seq_cst);
            }
        }

        std::atomic_bool* log_detected_;
    };
    std::unique_ptr<CustomLogConsumer> consumer(new CustomLogConsumer(&log_has_been_detected));
    Log::RegisterConsumer(std::move(consumer));

    {
        // Create a loaned sequence and check postconditions
        TestSeq uut;
        EXPECT_TRUE(uut.loan(stack.buffer, num_test_elements, 0));
        EXPECT_FALSE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        EXPECT_EQ(0u, uut.length());
        EXPECT_EQ(stack.buffer, uut.buffer());
        EXPECT_TRUE(uut.length(num_test_elements));
        EXPECT_EQ(num_test_elements, uut.length());

        // Check unloan
        result_buffer = uut.unloan(max, len);
        EXPECT_EQ(stack.buffer, result_buffer);
        EXPECT_EQ(num_test_elements, max);
        EXPECT_EQ(num_test_elements, len);

        // Check unloan postconditions
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.length());
        EXPECT_EQ(0u, uut.maximum());

        // Loan again
        EXPECT_TRUE(uut.loan(stack.buffer, num_test_elements, 0));
        EXPECT_FALSE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        EXPECT_EQ(0u, uut.length());
        EXPECT_EQ(stack.buffer, uut.buffer());

        // Check other version of unloan
        result_buffer = uut.unloan();
        EXPECT_EQ(stack.buffer, result_buffer);
        EXPECT_EQ(num_test_elements, max);
        EXPECT_EQ(num_test_elements, len);

        // Check unloan postconditions
        EXPECT_EQ(nullptr, uut.buffer());
        EXPECT_TRUE(uut.has_ownership());
        EXPECT_EQ(0u, uut.length());
        EXPECT_EQ(0u, uut.maximum());

        // Check with wrong parameters
        EXPECT_FALSE(uut.loan(stack.buffer, 0u, 0u));
        EXPECT_FALSE(uut.loan(stack.buffer, 1u, 2u));
        EXPECT_FALSE(uut.loan(nullptr, 1u, 1u));

        // Check that we can loan more than once
        EXPECT_TRUE(uut.loan(stack.buffer, num_test_elements, 0u));
        EXPECT_FALSE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        EXPECT_EQ(0u, uut.length());
        EXPECT_EQ(stack.buffer, uut.buffer());
        EXPECT_TRUE(uut.loan(stack.buffer, num_test_elements, num_test_elements));
        EXPECT_FALSE(uut.has_ownership());
        EXPECT_EQ(num_test_elements, uut.maximum());
        EXPECT_EQ(num_test_elements, uut.length());
        EXPECT_EQ(stack.buffer, uut.buffer());

        // Now loan a different buffer
        StackAllocatedBuffer<int, 1u> stack2;
        EXPECT_TRUE(uut.loan(stack2.buffer, 1u, 0u));
        EXPECT_FALSE(uut.has_ownership());
        EXPECT_EQ(1u, uut.maximum());
        EXPECT_EQ(0u, uut.length());
        EXPECT_EQ(stack2.buffer, uut.buffer());

        // Check that loaned buffer cannot grow above maximum
        EXPECT_TRUE(uut.length(1u));
        EXPECT_FALSE(uut.length(10u));
        EXPECT_FALSE(uut.has_ownership());
        EXPECT_EQ(1u, uut.maximum());
        EXPECT_EQ(1u, uut.length());
        EXPECT_EQ(stack2.buffer, uut.buffer());
    }

    Log::Flush();
    EXPECT_TRUE(log_has_been_detected.load(std::memory_order::memory_order_seq_cst));
}

void perform_accessors_test_step(
        TestSeq& uut,
        const TestSeq& c_uut)
{
    int n = 1000;
    test_size_type len = uut.length();

    // Accessing past last element should throw
    for (test_size_type i = 0; i < num_test_elements; ++i)
    {
        EXPECT_THROW(uut[len + i] = n, std::out_of_range);
        EXPECT_THROW(n = c_uut[len + i], std::out_of_range);
    }

    // Accessing from len-1 to 0 should not throw
    while (len > 0)
    {
        --len;
        EXPECT_NO_THROW(uut[len] = n);
        EXPECT_NO_THROW(n = c_uut[len]);
    }
}

void perform_accessors_tests(
        TestSeq& uut)
{
    const TestSeq& c_uut = uut;

    // Perform test on empty sequence
    perform_accessors_test_step(uut, c_uut);

    // Perform test on sequence with values
    set_result_values(uut);
    perform_accessors_test_step(uut, c_uut);
}

TEST(LoanableSequenceTests, accessors)
{
    TestSeq uut;

    // Perform test on loaned sequence
    StackAllocatedBuffer<int> stack;
    uut.loan(stack.buffer, stack.size(), 0);
    perform_accessors_tests(uut);

    // Perform test on owned sequence
    uut.unloan();
    perform_accessors_tests(uut);
}

template<typename T>
void sum_collections(
        LoanableSequence<T>& out,
        const LoanableSequence<T>& in1,
        const LoanableSequence<T>& in2)
{
    test_size_type length = std::min(in1.length(), in2.length());
    ASSERT_TRUE(out.length(length));
    for (test_size_type n = 0; n < length; ++n)
    {
        out[n] = in1[n] + in2[n];
    }
}

TEST(LoanableSequenceTests, sum_collections)
{
    TestSeq fibonacci;
    fibonacci.length(num_test_elements);
    fibonacci[0] = 1;
    fibonacci[1] = 1;
    for (test_size_type n = 2u; n < num_test_elements; ++n)
    {
        fibonacci[n] = fibonacci[n - 1] + fibonacci[n - 2];
    }

    // Check non-loaned version
    {
        TestSeq result;
        sum_collections(result, fibonacci, fibonacci);
        check_result(result);
        EXPECT_TRUE(result.has_ownership());
        EXPECT_EQ(num_test_elements, result.maximum());
    }

    // Check with loans
    {
        // Create a stack-allocated buffer
        StackAllocatedBuffer<int> stack;

        // Create a loaned sequence and check postconditions
        TestSeq result;
        EXPECT_TRUE(result.loan(stack.buffer, num_test_elements, 0));
        EXPECT_FALSE(result.has_ownership());
        EXPECT_EQ(num_test_elements, result.maximum());
        EXPECT_EQ(0u, result.length());
        EXPECT_EQ(stack.buffer, result.buffer());

        // Test increasing length and accessors
        sum_collections(result, fibonacci, fibonacci);
        check_result(result);
        EXPECT_FALSE(result.has_ownership());
        EXPECT_EQ(num_test_elements, result.maximum());
        EXPECT_EQ(stack.buffer, result.buffer());

        // Test unloan.
        test_size_type max, len;
        void** result_buffer = result.unloan(max, len);
        EXPECT_EQ(stack.buffer, result_buffer);
        EXPECT_EQ(num_test_elements, max);
        EXPECT_EQ(num_test_elements, len);

        // Check unloan postconditions
        EXPECT_EQ(nullptr, result.buffer());
        EXPECT_TRUE(result.has_ownership());
        EXPECT_EQ(0u, result.length());
        EXPECT_EQ(0u, result.maximum());
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

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

#include <climits>

#include <gtest/gtest.h>

#include <fastdds/rtps/common/SequenceNumber.hpp>

using namespace eprosima::fastdds::rtps;

/*!
 * @fn TEST(SequenceNumber, IncrementalOperator)
 * @brief This test checks the incremental operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, IncrementalOperator)
{
    SequenceNumber_t seq(0, UINT32_MAX);

    ++seq;

    SequenceNumber_t expected_seq(1, 0);

    ASSERT_EQ(seq, expected_seq);

    ++seq;

    expected_seq.low = 1;

    ASSERT_EQ(seq, expected_seq);
}

/*!
 * @fn TEST(SequenceNumber, AdditionAssignmentOperator)
 * @brief This test checks the addition assignment operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, AdditionAssignmentOperator)
{
    SequenceNumber_t seq(3, UINT32_MAX - 3);

    seq += 7;

    SequenceNumber_t expected_seq(4, 3);

    ASSERT_EQ(seq, expected_seq);

    seq += INT_MAX;

    expected_seq.low = (uint32_t)INT32_MAX;
    expected_seq.low += 3;

    ASSERT_EQ(seq, expected_seq);

    seq += INT_MAX;

    expected_seq.high = 5;
    expected_seq.low = 1;

    ASSERT_EQ(seq, expected_seq);

    seq.high = INT32_MAX - 1;
    seq.low = 0;

    seq += INT_MAX;

    expected_seq.high = INT32_MAX - 1;
    expected_seq.low = (uint32_t)INT32_MAX;

    ASSERT_EQ(seq, expected_seq);

    seq += INT_MAX;

    expected_seq.low = UINT32_MAX - 1;

    ASSERT_EQ(seq, expected_seq);

    seq += INT_MAX;

    expected_seq.high = INT32_MAX;
    expected_seq.low = (uint32_t)INT32_MAX - 2;

    ASSERT_EQ(seq, expected_seq);
}

/*!
 * @fn TEST(SequenceNumber, EqualOperator)
 * @brief This test checks the equal operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, EqualOperator)
{
    SequenceNumber_t seq(10, 4356), seq2(10, 4356);

    ASSERT_TRUE(seq == seq2);

    seq.high = 345;

    ASSERT_FALSE(seq == seq2);

    seq.high = 1;

    ASSERT_FALSE(seq == seq2);

    seq.high = 10;
    seq.low = 60000;

    ASSERT_FALSE(seq == seq2);

    seq.low = 100;

    ASSERT_FALSE(seq == seq2);
}

/*!
 * @fn TEST(SequenceNumber, NotEqualOperator)
 * @brief This test checks the not equal operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, NotEqualOperator)
{
    SequenceNumber_t seq(10, 4356), seq2(10, 4356);

    ASSERT_FALSE(seq != seq2);

    seq.high = 345;

    ASSERT_TRUE(seq != seq2);

    seq.high = 1;

    ASSERT_TRUE(seq != seq2);

    seq.high = 10;
    seq.low = 60000;

    ASSERT_TRUE(seq != seq2);

    seq.low = 100;

    ASSERT_TRUE(seq != seq2);
}

/*!
 * @fn TEST(SequenceNumber, GreaterThanOperator)
 * @brief This test checks the greater than operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, GreaterThanOperator)
{
    SequenceNumber_t seq(10, 4356), seq2(10, 4356);

    ASSERT_FALSE(seq > seq2);

    seq.high = 345;

    ASSERT_TRUE(seq > seq2);

    seq.high = 1;

    ASSERT_FALSE(seq > seq2);

    seq.high = 10;
    seq.low = 60000;

    ASSERT_TRUE(seq > seq2);

    seq.low = 100;

    ASSERT_FALSE(seq > seq2);
}

/*!
 * @fn TEST(SequenceNumber, LessThanOperator)
 * @brief This test checks the less than operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, LessThanOperator)
{
    SequenceNumber_t seq(10, 4356), seq2(10, 4356);

    ASSERT_FALSE(seq < seq2);

    seq.high = 345;

    ASSERT_FALSE(seq < seq2);

    seq.high = 1;

    ASSERT_TRUE(seq < seq2);

    seq.high = 10;
    seq.low = 60000;

    ASSERT_FALSE(seq < seq2);

    seq.low = 100;

    ASSERT_TRUE(seq < seq2);
}

/*!
 * @fn TEST(SequenceNumber, GreaterThanOrEqualOperator)
 * @brief This test checks the greater than or equal operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, GreaterThanOrEqualOperator)
{
    SequenceNumber_t seq(10, 4356), seq2(10, 4356);

    ASSERT_TRUE(seq >= seq2);

    seq.high = 345;

    ASSERT_TRUE(seq >= seq2);

    seq.high = 1;

    ASSERT_FALSE(seq >= seq2);

    seq.high = 10;
    seq.low = 60000;

    ASSERT_TRUE(seq >= seq2);

    seq.low = 100;

    ASSERT_FALSE(seq >= seq2);
}

/*!
 * @fn TEST(SequenceNumber, LessThanOrEqualOperator)
 * @brief This test checks the less than or equal operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, LessThanOrEqualOperator)
{
    SequenceNumber_t seq(10, 4356), seq2(10, 4356);

    ASSERT_TRUE(seq <= seq2);

    seq.high = 345;

    ASSERT_FALSE(seq <= seq2);

    seq.high = 1;

    ASSERT_TRUE(seq <= seq2);

    seq.high = 10;
    seq.low = 60000;

    ASSERT_FALSE(seq <= seq2);

    seq.low = 100;

    ASSERT_TRUE(seq <= seq2);
}

/*!
 * @fn TEST(SequenceNumber, SubtractionOperator)
 * @brief This test checks the subtraction operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, SubtractionOperator)
{
    SequenceNumber_t seq(4, 3);

    seq = seq - 7;

    SequenceNumber_t expected_seq(3, UINT32_MAX - 3);

    ASSERT_EQ(seq, expected_seq);

    seq.high = INT32_MAX;
    seq.low = UINT32_MAX - 1;

    seq = seq - (uint32_t)INT32_MAX;

    expected_seq.high = INT32_MAX;
    expected_seq.low = (uint32_t)INT32_MAX;

    ASSERT_EQ(seq, expected_seq);

    seq.high = 25;
    seq.low = UINT32_MAX;

    seq = seq - UINT32_MAX;

    expected_seq.high = 25;
    expected_seq.low = 0;

    ASSERT_EQ(seq, expected_seq);

    seq = seq - UINT32_MAX;

    expected_seq.high = 24;
    expected_seq.low = 1;

    ASSERT_EQ(seq, expected_seq);
}

/*!
 * @fn TEST(SequenceNumber, AdditionOperator)
 * @brief This test checks the addition operator for type SequenceNumber_t.
 */
TEST(SequenceNumber, AdditionOperator)
{
    SequenceNumber_t seq(3, UINT32_MAX - 3);

    seq = seq + 7;

    SequenceNumber_t expected_seq(4, 3);

    ASSERT_EQ(seq, expected_seq);

    seq = seq + (uint32_t)INT32_MAX;

    expected_seq.low = (uint32_t)INT32_MAX;
    expected_seq.low += 3;

    ASSERT_EQ(seq, expected_seq);

    seq = seq + (uint32_t)INT32_MAX;

    expected_seq.high = 5;
    expected_seq.low = 1;

    ASSERT_EQ(seq, expected_seq);

    seq.high = INT32_MAX - 1;
    seq.low = 0;

    seq = seq +  (uint32_t)INT32_MAX;

    expected_seq.high = INT32_MAX - 1;
    expected_seq.low = (uint32_t)INT32_MAX;

    ASSERT_EQ(seq, expected_seq);

    seq = seq + (uint32_t)INT32_MAX;

    expected_seq.low = UINT32_MAX - 1;

    ASSERT_EQ(seq, expected_seq);

    seq = seq + (uint32_t)INT32_MAX;

    expected_seq.high = INT32_MAX;
    expected_seq.low = (uint32_t)INT32_MAX - 2;

    ASSERT_EQ(seq, expected_seq);

    seq.high = 24;
    seq.low = 1;

    seq = seq + UINT32_MAX;

    expected_seq.high = 25;
    expected_seq.low = 0;

    ASSERT_EQ(seq, expected_seq);

    seq = seq + UINT32_MAX;

    expected_seq.low = UINT32_MAX;

    ASSERT_EQ(seq, expected_seq);
}

/*!
 * @fn TEST(SequenceNumber, SubtractionBetweenSesOperator)
 * @brief This test checks the subtraction operator between SequenceNumber_t.
 * This operation has a limit: minuend has to be greater than or equal to subtrahend.
 */
TEST(SequenceNumber, SubtractionBetweenSesOperator)
{
    SequenceNumber_t minuend(4, 3);
    SequenceNumber_t subtrahend(0, 7);

    SequenceNumber_t res = minuend - subtrahend;

    SequenceNumber_t expected_seq(3, UINT32_MAX - 3);

    ASSERT_EQ(res, expected_seq);

    minuend.high = INT32_MAX;
    minuend.low = UINT32_MAX - 1;

    subtrahend.high = 0;
    subtrahend.low = (uint32_t)INT32_MAX;

    res = minuend - subtrahend;

    expected_seq.high = INT32_MAX;
    expected_seq.low = (uint32_t)INT32_MAX;

    ASSERT_EQ(res, expected_seq);

    minuend.high = 25;
    minuend.low = UINT32_MAX;

    subtrahend.high = 0;
    subtrahend.low = UINT32_MAX;

    res = minuend - subtrahend;

    expected_seq.high = 25;
    expected_seq.low = 0;

    ASSERT_EQ(res, expected_seq);

    res = res - subtrahend;

    expected_seq.high = 24;
    expected_seq.low = 1;

    ASSERT_EQ(res, expected_seq);
}

/*!
 * @fn TEST(SequenceNumberSet, AddOperation)
 * @brief This test checks the addition operator of SequenceNumber_t data on type SequenceNumberSet_t.
 */
TEST(SequenceNumberSet, AddOperation)
{
    SequenceNumber_t seq(10, UINT32_MAX - 1);

    SequenceNumberSet_t set(seq);

    ASSERT_TRUE(set.add(seq));

    ++seq;

    ASSERT_TRUE(set.add(seq));

    ++seq;

    ASSERT_TRUE(set.add(seq));

    seq += 252;

    ASSERT_TRUE(set.add(seq));

    seq += 3;

    ASSERT_FALSE(set.add(seq));

}

/*!
 * @fn TEST(SequenceNumberSet, GetMaxSeqNumOperation)
 * @brief This test checks the function that return the max sequence number.
 */
TEST(SequenceNumberSet, GetMaxSeqNumOperation)
{

    SequenceNumber_t seq(10, UINT32_MAX - 3);

    SequenceNumberSet_t set(seq);

    seq.low = UINT32_MAX - 1;

    ASSERT_TRUE(set.add(seq));

    seq.high = 11;
    seq.low = 20;

    ASSERT_TRUE(set.add(seq));

    seq.high = 10;
    seq.low = UINT32_MAX - 3;

    ASSERT_TRUE(set.add(seq));

    seq.high = 11;
    seq.low = 10;

    ASSERT_TRUE(set.add(seq));

    seq.high = 10;
    seq.low = UINT32_MAX;

    ASSERT_TRUE(set.add(seq));

    SequenceNumber_t expected_seq(11, 20);

    ASSERT_EQ(set.max(), expected_seq);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

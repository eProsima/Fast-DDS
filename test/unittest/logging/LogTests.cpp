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

#include <fastrtps/log/NewLog.h>
#include <fastrtps/log/StdoutConsumer.h>
#include "mock/MockConsumer.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>

using namespace eprosima::fastrtps;
using namespace std;

class LogTests: public ::testing::Test 
{
   public:
   LogTests() 
   {
      std::unique_ptr<MockConsumer> consumer(new MockConsumer);
      std::unique_ptr<StdoutConsumer> defaultConsumer(new StdoutConsumer);
      mockConsumer = consumer.get();
      Log::RegisterConsumer(std::move(consumer));
      Log::RegisterConsumer(std::move(defaultConsumer));
      Log::StartLogging();
   }

   ~LogTests()
   {
      Log::Reset();
   }

   MockConsumer* mockConsumer;

   const uint32_t AsyncTries = 5;
   const uint32_t AsyncWaitMs = 50;

   std::vector<Log::Entry> HELPER_WaitForEntries(uint32_t amount);
};

TEST_F(LogTests, asynchronous_logging)
{
   logError(SampleCategory, "Sample error message");
   logWarning(SampleCategory, "Sample warning message");
   logWarning(DifferentCategory, "Sample warning message in another category");

   auto consumedEntries = HELPER_WaitForEntries(3);
   ASSERT_EQ(3, consumedEntries.size());
}

TEST_F(LogTests, reporting_options)
{
   // moving away from the defaults
   Log::ReportFilenames(true);
   Log::ReportFunctions(false);
   
   logError(Reporting, "Error with different reporting options");
   auto consumedEntries = HELPER_WaitForEntries(1);
   ASSERT_EQ(1, consumedEntries.size());
   
   auto entry = consumedEntries.back();
   ASSERT_NE(entry.context.filename, nullptr);
   ASSERT_EQ(entry.context.function, nullptr);
}

TEST_F(LogTests, multithreaded_logging)
{
   vector<unique_ptr<thread>> threads;
   for (int i = 0; i != 5; i++)
   {
      threads.emplace_back(new thread([i]{
         std::stringstream ss;
         ss << "I'm thread " << i << "!";
         logWarning(Multithread, ss.str());
      }));
   }

   for (auto& thread: threads) {
      thread->join();
   }

   auto consumedEntries = HELPER_WaitForEntries(5);
   ASSERT_EQ(5, consumedEntries.size());
}

TEST_F(LogTests, regex_category_filtering)
{
   Log::SetRegexFilter(std::regex("(Good)"));
   logError(GoodCategory, "This should be logged");
   logError(BadCategory, "If you're seeing this, something went wrong");
   logWarning(EvenMoreGoodCategory, "This should be logged too!");
   auto consumedEntries = HELPER_WaitForEntries(3);
   ASSERT_EQ(2, consumedEntries.size());
}

TEST_F(LogTests, logging_resilient_to_bad_uses)
{
   Log::StopLogging();
   logError(SampleCategory, "This shouldn't be logged");
   logWarning(SampleCategory, "This shouldn't be logged");

   Log::StopLogging();
   Log::StopLogging();
   Log::StartLogging();
   Log::StopLogging();
   Log::StartLogging();
   logError(SampleCategory, "This should be logged");

   auto consumedEntries = HELPER_WaitForEntries(3);
   ASSERT_EQ(1, consumedEntries.size());

}

std::vector<Log::Entry> LogTests::HELPER_WaitForEntries(uint32_t amount)
{
   uint32_t entries = 0;
   for (uint32_t i = 0; i != AsyncTries; i++)
   {
      entries |= mockConsumer->ConsumedEntries().size();
      if (entries == amount) break;
      this_thread::sleep_for(chrono::milliseconds(AsyncWaitMs));
   }

   return mockConsumer->ConsumedEntries();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

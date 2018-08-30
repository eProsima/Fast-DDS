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
//

#include <fastrtps/log/Log.h>
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
      Log::SetVerbosity(Log::Info);
   }

   ~LogTests()
   {
      Log::Reset();
      Log::KillThread();
   }

   MockConsumer* mockConsumer;

   const uint32_t AsyncTries = 5;
   const uint32_t AsyncWaitMs = 25;

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
         logWarning(Multithread, "I'm thread " << i);
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
   Log::SetCategoryFilter(std::regex("(Good)"));
   logError(GoodCategory, "This should be logged because my regex filter allows for it");
   logError(BadCategory, "If you're seeing this, something went wrong");
   logWarning(EvenMoreGoodCategory, "This should be logged too!");
   auto consumedEntries = HELPER_WaitForEntries(3);
   ASSERT_EQ(2, consumedEntries.size());
}

TEST_F(LogTests, multi_criteria_filtering_with_regex)
{
   Log::SetCategoryFilter(std::regex("(Good)"));
   Log::SetFilenameFilter(std::regex("(LogTests)", std::regex_constants::icase));
   Log::SetErrorStringFilter(std::regex("(Good)"));
   Log::ReportFilenames(true); // For clarity, not necessary.

   logError(GoodCategory, "This should be logged because it contains the word \"Good\" in the "\
      "error string and the category, and is in the right filename");
   logError(BadCategory,  "Despite the word \"Good\" being here, this shouldn't be logged");
   logError(GoodCategory, "And neither should this.");
   auto consumedEntries = HELPER_WaitForEntries(3);
   ASSERT_EQ(1, consumedEntries.size());

   Log::SetFilenameFilter(std::regex("(we shouldn't find this ever)"));
   logError(GoodCategory,  "Despite the word \"Good\" being here, this shouldn't be logged because "\
                           "the filename is all wrong");

   consumedEntries = HELPER_WaitForEntries(2);
   ASSERT_EQ(1, consumedEntries.size());
}

TEST_F(LogTests, multiple_verbosity_levels)
{
   Log::SetVerbosity(Log::Warning);
   logError(VerbosityChecks, "This should be logged");
   logWarning(VerbosityChecks, "This should be logged too!");
   logInfo(VerbosityChecks, "If you're seeing this, something went wrong");
   auto consumedEntries = HELPER_WaitForEntries(3);
   ASSERT_EQ(2, consumedEntries.size());

   Log::SetVerbosity(Log::Error);
   logError(VerbosityChecks, "This should be logged");
   logWarning(VerbosityChecks, "If you're seeing this, something went wrong");
   logInfo(VerbosityChecks, "If you're seeing this, something went wrong");

   consumedEntries = HELPER_WaitForEntries(5);
   ASSERT_EQ(3, consumedEntries.size());
}

std::vector<Log::Entry> LogTests::HELPER_WaitForEntries(uint32_t amount)
{
   size_t entries = 0;
   for (uint32_t i = 0; i != AsyncTries; i++)
   {
      entries = mockConsumer->ConsumedEntries().size();
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

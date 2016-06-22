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
#include "mock/MockConsumer.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>

//using namespace std;
//using namespace eprosima::fastrtps::rtps;

using namespace eprosima::fastrtps;
using namespace std;

class LogTests: public ::testing::Test 
{
   public:
   LogTests() 
   {
      std::unique_ptr<MockConsumer> consumer(new MockConsumer);
      mockConsumer = consumer.get();
      Log::RegisterConsumer(std::move(consumer));
      Log::StartLogging();
   }

   ~LogTests()
   {
      Log::StopLogging();
   }

   MockConsumer* mockConsumer;

   const uint32_t AsyncTries = 5;
   const uint32_t AsyncWaitMs = 50;
};

TEST_F(LogTests, asynchronous_logging)
{
   logError("I'm logging this, asynchronously");

   bool logged = false;
   for (uint32_t i = 0; i != AsyncTries; i++)
   {
      logged |= !mockConsumer->ConsumedEntries().empty();
      if (logged) break;
      this_thread::sleep_for(chrono::milliseconds(AsyncWaitMs));
   }

   ASSERT_TRUE(logged);
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

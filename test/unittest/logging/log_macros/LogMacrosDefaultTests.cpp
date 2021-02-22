
// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/log/Log.hpp>
#include "LogMacros.hpp"
#include <gtest/gtest.h>

/* WARNING - This test will fail with any LOG_NO_ CMake option set different than default configuration
 * Check all log levels are active in debug mode, or INFO is not active in Release mode */
TEST_F(LogMacrosTests, default_macros_test)
{
    logError(SampleCategory, "Sample error message");
    logWarning(SampleCategory, "Sample warning message");
    logInfo(SampleCategory, "Sample info message");

    unsigned int expected_result = 3;

#if !(__DEBUG || _DEBUG)
    --expected_result;
#endif // CMAKE_BUILD_TYPE == DEBUG_TYPE

    auto consumedEntries = HELPER_WaitForEntries(expected_result);
    ASSERT_EQ(expected_result, consumedEntries.size());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

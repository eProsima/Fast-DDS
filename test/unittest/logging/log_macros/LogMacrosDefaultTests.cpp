
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

#define log_str(x) #x
#define macro_print(mname) std::cout << #mname << " = " << \
        (std::string(#mname) == log_str(mname) ? "" : log_str(mname)) << std::endl

/* WARNING - This test will fail with any LOG_NO_ CMake option set different than default configuration
 * Check all log levels are active in debug mode, or INFO is not active in Release mode */
TEST_F(LogMacrosTests, default_macros_test)
{
    std::cout << std::endl << "EPROSIMA_LOG_INFO #define'd related constants:" << std::endl;
    macro_print(HAVE_LOG_NO_INFO);
#if !defined(_MSC_VER)
    macro_print(FASTDDS_ENFORCE_LOG_INFO);
#endif // Visual Studio seems to have issues with macro expansion
    macro_print(__INTERNALDEBUG);
    macro_print(_INTERNALDEBUG);
    macro_print(_DEBUG);
    macro_print(__DEBUG);
    macro_print(NDEBUG);
    std::cout << std::endl;

    EPROSIMA_LOG_ERROR(SampleCategory, "Sample error message");
    EPROSIMA_LOG_WARNING(SampleCategory, "Sample warning message");
    EPROSIMA_LOG_INFO(SampleCategory, "Sample info message");

#if defined(NDEBUG) && !HAVE_LOG_NO_INFO
# if !defined(_MSC_VER )
    GTEST_SKIP() << "Unexpected default values for NDEBUG and HAVE_LOG_NO_INFO";
# endif  // Visual Studio specific behavior
#endif  // Check default macro values

    // Result depends on the log configuration
#if !HAVE_LOG_NO_INFO &&  \
    (defined(FASTDDS_ENFORCE_LOG_INFO) || \
    ((defined(__INTERNALDEBUG) || defined(_INTERNALDEBUG)) && (defined(_DEBUG) || defined(__DEBUG) || \
    !defined(NDEBUG))))
    constexpr unsigned int expected_result = 3;
#else
    constexpr unsigned int expected_result = 2;
#endif // debug macros check

    // Warning and error should always be logged.
    // Info might be logged depending on the log configuration.
    // We always wait for 3 entries, though not all of them might be present.
    auto consumedEntries = HELPER_WaitForEntries(3u);
    ASSERT_EQ(expected_result, consumedEntries.size());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

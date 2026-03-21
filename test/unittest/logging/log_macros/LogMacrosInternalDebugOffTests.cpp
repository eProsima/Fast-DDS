
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

#define FASTDDS_ENFORCE_LOG_INFO
#ifdef HAVE_LOG_NO_INFO
#undef HAVE_LOG_NO_INFO
#endif // HAVE_LOG_NO_INFO
#define HAVE_LOG_NO_INFO 1

#ifdef HAVE_LOG_NO_WARNING
#undef HAVE_LOG_NO_WARNING
#endif // HAVE_LOG_NO_WARNING
#define HAVE_LOG_NO_WARNING 1

#ifdef HAVE_LOG_NO_ERROR
#undef HAVE_LOG_NO_ERROR
#endif // HAVE_LOG_NO_ERROR
#define HAVE_LOG_NO_ERROR 1

#ifdef __INTERNALDEBUG
#undef __INTERNALDEBUG
#endif // __INTERNALDEBUG
#define __INTERNALDEBUG 0

#include <fastdds/dds/log/Log.hpp>
#include "LogMacros.hpp"
#include <gtest/gtest.h>

/*
 * WARNING: If this test is not properly working, the expected behaviour would be a compilation failure
 * This test try to send in a macro a value (void) that is not convertible to string, so the compilation
 * must fail if INTERNAL_DEBUG is ON or any HAVE_LOG_NO_... is OFF
 */
TEST_F(LogMacrosTests, internal_debug_off)
{
    int n = 0;
    EPROSIMA_LOG_ERROR(SampleCategory, non_valid_function(n));
    EPROSIMA_LOG_WARNING(SampleCategory, non_valid_function(n));
    EPROSIMA_LOG_INFO(SampleCategory, non_valid_function(n));

    auto consumedEntries = HELPER_WaitForEntries(0);
    // No logs must be shown
    ASSERT_EQ(0u, consumedEntries.size());
    // n must not increase as INTERNAL_DEBUG does not force code to execute, just to compile
    ASSERT_EQ(n, 0);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

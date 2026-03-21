
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

/*************************************
* Force old logs to NOT be compiled *
*************************************/
// Enforce to NOT compile old macros
#ifdef ENABLE_OLD_LOG_MACROS_
#undef ENABLE_OLD_LOG_MACROS_
#endif  // ENABLE_OLD_LOG_MACROS_
#define ENABLE_OLD_LOG_MACROS_ 0

// Enforce log info, warning and error to compile
#define FASTDDS_ENFORCE_LOG_INFO
#ifdef HAVE_LOG_NO_INFO
#undef HAVE_LOG_NO_INFO
#endif // HAVE_LOG_NO_INFO
#define HAVE_LOG_NO_INFO 0

#ifdef HAVE_LOG_NO_WARNING
#undef HAVE_LOG_NO_WARNING
#endif // HAVE_LOG_NO_WARNING
#define HAVE_LOG_NO_WARNING 0

#ifdef HAVE_LOG_NO_ERROR
#undef HAVE_LOG_NO_ERROR
#endif // HAVE_LOG_NO_ERROR
#define HAVE_LOG_NO_ERROR 0

#include <fastdds/dds/log/Log.hpp>
#include "LogMacros.hpp"
#include <gtest/gtest.h>

// Check that logInfo logWarning and logError and its private definitions does not exist.
TEST_F(LogMacrosTests, check_old_not_compiled)
{

#if defined(logInfo) || defined(logInfo_)
    ASSERT_TRUE(0) << "logInfo exists even disabling ENABLE_OLD_LOG_MACROS_";
#endif  // logInfo || logInfo_

#if defined(logWarning) || defined(logWarning_)
    ASSERT_TRUE(0) << "logWarning exists even disabling ENABLE_OLD_LOG_MACROS_";
#endif  // logWarning || logWarning_

#if defined(logError) || defined(logError_)
    ASSERT_TRUE(0) << "logError exists even disabling ENABLE_OLD_LOG_MACROS_";
#endif  // logError || logError_

}

TEST_F(LogMacrosTests, all_active_new_ones_old_disable)
{
    EPROSIMA_LOG_ERROR(SampleCategory, "Sample error message");
    EPROSIMA_LOG_WARNING(SampleCategory, "Sample warning message");
    EPROSIMA_LOG_INFO(SampleCategory, "Sample info message");

    auto consumedEntries = HELPER_WaitForEntries(3);
    ASSERT_EQ(3u, consumedEntries.size());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

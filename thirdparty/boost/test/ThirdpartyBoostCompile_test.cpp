// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <boostconfig.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/thread/thread_time.hpp>

/**
 * This is a temporary class intented to check the correct compilation of the
 * third party boost classes that will be used in the shared-memory transport
 * implementation
 */
class ThirdpartyBoostCompileTest
{
public:

    ThirdpartyBoostCompileTest()
    {
        boost::interprocess::named_mutex::remove("foo");
        boost::interprocess::interprocess_semaphore sem {0};

        try
        {
            boost::interprocess::managed_shared_memory foo(boost::interprocess::open_only, "foo");
        }
        catch (const std::exception&)
        {
        }

        try
        {
            boost::interprocess::interprocess_mutex mutex;
            boost::interprocess::interprocess_condition cv;
            boost::get_system_time();
        }
        catch (const std::exception&)
        {
        }
    }

};

int main()
{
    try
    {
        ThirdpartyBoostCompileTest compile_test;
    }
    catch (const std::exception&)
    {
        return -1;
    }

    return 0;
}

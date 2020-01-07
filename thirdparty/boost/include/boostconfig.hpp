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

#ifndef _FASTDDS_THIRDPARTYBOOST_BOOSTCONFIG_H_
#define _FASTDDS_THIRDPARTYBOOST_BOOSTCONFIG_H_

#define BOOST_DATE_TIME_NO_LIB
#define BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING
#define BOOST_INTERPROCESS_TIMEOUT_WHEN_LOCKING_DURATION_MS 1000

#ifdef _MSC_VER

#include <stdlib.h>

//#define BOOST_INTERPROCESS_BOOTSTAMP_IS_LASTBOOTUPTIME
#define BOOST_INTERPROCESS_SHARED_DIR_PATH "C:\\ProgramData\\fastrtps_interprocess"

#include <boost/interprocess/detail/workaround.hpp>
#if defined(BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION)
#error "BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION must be disabled in boost/interprocess/detail/workarround.hpp"
#endif

class SharedDirCreator
{
public:

    SharedDirCreator();
    void forzeConstruct();
    static void clean() {
        system("del " BOOST_INTERPROCESS_SHARED_DIR_PATH "\\*.* /Q");
    }

private:

    bool created_;
};

extern SharedDirCreator g_shared_dir_creator;
#define BOOST_INTERPROCESS_USE_WINDOWS
//#define BOOST_INTERPROCESS_BOOTSTAMP_IS_SESSION_MANAGER_BASED

#endif // _MSC_VER_

#endif // _FASTDDS_THIRDPARTYBOOST_BOOSTCONFIG_H_

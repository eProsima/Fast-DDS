// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file thread_logging.hpp
 */

#ifndef THREAD_LOGGING_HPP
#define THREAD_LOGGING_HPP

#define THREAD_EPROSIMA_LOG_ERROR(thread_name, msg)                         \
    do{                                                                     \
        if (strcmp(thread_name, "dds.log") == 0)                            \
        {                                                                   \
            std::cerr << msg << std::endl;                                  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            EPROSIMA_LOG_ERROR(SYSTEM, msg);                                \
        }                                                                   \
    } while (0)

#endif  // THREAD_LOGGING_HPP

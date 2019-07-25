/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef EPROSIMA_DDS_CORE_DETAIL_MACROS_HPP_
#define EPROSIMA_DDS_CORE_DETAIL_MACROS_HPP_

#include <iostream>

// Constants
#define OMG_DDS_DEFAULT_STATE_BIT_COUNT_DETAIL size_t(16)
#define OMG_DDS_DEFAULT_STATUS_COUNT_DETAIL    size_t(16)
// ==========================================================================

// Static Assert
#define OMG_DDS_STATIC_ASSERT_DETAIL(...) static_assert(__VA_ARGS__, #__VA_ARGS__)
// ==========================================================================

// Logging Macros
#define OMG_DDS_LOG_DETAIL(kind, msg) \
    std::cout << "[" << kind << "]: " << msg << std::endl;
// ==========================================================================


// DLL Export Macros
#ifdef _WIN32 // This is defined for 32/64 bit Windows
#define OMG_DDS_API_DETAIL __declspec(dllexport)
#else
#define OMG_DDS_API_DETAIL
#endif
// ==========================================================================


#endif //EPROSIMA_DDS_CORE_DETAIL_MACROS_HPP_

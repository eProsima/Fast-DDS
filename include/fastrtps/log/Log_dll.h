/*************************************************************************
 * Copyright (c) 201 4 eProsima. All rights reserved.
 *
 * This copy of eProsima Log is licensed to you under the terms described in the
 * *_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef _LOG_LOG_DLL_H_
#define _LOG_LOG_DLL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS
// normalize macros
#if !defined(LOG_DYN_LINK) && !defined(LOG_STATIC_LINK) \
    && !defined(EPROSIMA_ALL_DYN_LINK) && !defined(EPROSIMA_ALL_STATIC_LINK)
#define LOG_STATIC_LINK
#endif

#if defined(EPROSIMA_ALL_DYN_LINK) && !defined(LOG_DYN_LINK)
#define LOG_DYN_LINK
#endif

#if defined(LOG_DYN_LINK) && defined(LOG_STATIC_LINK)
#error Must not define both LOG_DYN_LINK and LOG_STATIC_LINK
#endif

#if defined(EPROSIMA_ALL_NO_LIB) && !defined(LOG_NO_LIB)
#define LOG_NO_LIB
#endif

// enable dynamic linking

#if defined(_WIN32)
#if defined(EPROSIMA_ALL_DYN_LINK) || defined(LOG_DYN_LINK)
#if defined(LOG_SOURCE)
#define LOG_DllAPI __declspec( dllexport )
#else
#define LOG_DllAPI __declspec( dllimport )
#endif // LOG_SOURCE
#else
#define LOG_DllAPI
#endif
#else
#define LOG_DllAPI
#endif // _WIN32

// Auto linking.

#if !defined(LOG_SOURCE) && !defined(EPROSIMA_ALL_NO_LIB) \
    && !defined(LOG_NO_LIB)


// Set properties.
//#define EPROSIMA_LIB_NAME LOG

#if defined(EPROSIMA_ALL_DYN_LINK) || defined(LOG_DYN_LINK)
#define EPROSIMA_DYN_LINK
#endif

//#include "eProsima_auto_link.h"
#endif // auto-linking disabled

#endif //
#endif

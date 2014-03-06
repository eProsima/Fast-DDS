/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of EprosimaRTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef _EPROSIMARTPS_EPROSIMARTPS_DLL_H_
#define _EPROSIMARTPS_EPROSIMARTPS_DLL_H_

// normalize macros
#if !defined(EPROSIMARTPS_DYN_LINK) && !defined(EPROSIMARTPS_STATIC_LINK) \
    && !defined(EPROSIMA_ALL_DYN_LINK) && !defined(EPROSIMA_ALL_STATIC_LINK)
#define EPROSIMARTPS_STATIC_LINK
#endif

#if defined(EPROSIMA_ALL_DYN_LINK) && !defined(EPROSIMARTPS_DYN_LINK)
#define EPROSIMARTPS_DYN_LINK
#endif

#if defined(EPROSIMARTPS_DYN_LINK) && defined(EPROSIMARTPS_STATIC_LINK)
#error Must not define both EPROSIMARTPS_DYN_LINK and EPROSIMARTPS_STATIC_LINK
#endif

#if defined(EPROSIMA_ALL_NO_LIB) && !defined(EPROSIMARTPS_NO_LIB)
#define EPROSIMARTPS_NO_LIB
#endif

// enable dynamic linking

#if defined(_WIN32)
#if defined(EPROSIMA_ALL_DYN_LINK) || defined(EPROSIMARTPS_DYN_LINK)
#if defined(EPROSIMARTPS_SOURCE)
#define RTPS_DllAPI __declspec( dllexport )
#else
#define RTPS_DllAPI __declspec( dllimport )
#endif // EPROSIMARTPS_SOURCE
#else
#define RTPS_DllAPI
#endif
#else
#define RTPS_DllAPI
#endif // _WIN32

// Auto linking.

#if !defined(EPROSIMARTPS_SOURCE) && !defined(EPROSIMA_ALL_NO_LIB) \
    && !defined(EPROSIMARTPS_NO_LIB)

#include "eprosimartps/eprosimartps_version.h"

// Set properties.
#define EPROSIMA_LIB_NAME eprosimartps

#if defined(EPROSIMA_ALL_DYN_LINK) || defined(EPROSIMARTPS_DYN_LINK)
#define EPROSIMA_DYN_LINK
#endif

#include "eprosimartps/eProsima_auto_link.h"
#endif // auto-linking disabled

#endif // _EPROSIMARTPS_EPROSIMARTPS_DLL_H_

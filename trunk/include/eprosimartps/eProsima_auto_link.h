/*
 Expected defines.

 - EPROSIMA_LIB_NAME
 - VERSION_STR
 - VERSION_MAJOR
 - VERSION_MINOR
 - VERSION_RELEA
*/

#if defined(_MSC_VER)
    #define EPROSIMA_STRINGIZE(X) EPROSIMA_DO_STRINGIZE(X)
    #define EPROSIMA_DO_STRINGIZE(X) #X

    #if defined(_DEBUG)
        #define EPROSIMA_LIB_DEBUG_TAG "d"
    #else
        #define EPROSIMA_LIB_DEBUG_TAG
    #endif // _DEBUG

    // Select linkage option.
    #if (defined(_DLL) || defined(_RTLDLL)) && defined(EPROSIMA_DYN_LINK)
        #define EPROSIMA_LIB_PREFIX
    #elif defined(EPROSIMA_DYN_LINK)
        #error "Mixing a dll eprosima library with a static runtime is a bad idea"
    #else
        #define EPROSIMA_LIB_PREFIX "lib"
    #endif

    // Include library
    #if defined(EPROSIMA_LIB_NAME) \
	&& defined(EPROSIMA_LIB_PREFIX) \
	&& defined(EPROSIMA_LIB_DEBUG_TAG) \
	&& defined(VERSION_STR)
        #pragma comment(lib, EPROSIMA_LIB_PREFIX EPROSIMA_STRINGIZE(EPROSIMA_LIB_NAME) EPROSIMA_LIB_DEBUG_TAG "-" VERSION_STR ".lib")
    #else
    #error "Some required macros where not defined"
    #endif

#endif // _MSC_VER
    
// Undef macros
#ifdef EPROSIMA_LIB_PREFIX
#undef EPROSIMA_LIB_PREFIX
#endif

#ifdef EPROSIMA_LIB_NAME
#undef EPROSIMA_LIB_NAME
#endif

#ifdef EPROSIMA_LIB_DEBUG_TAG
#undef EPROSIMA_LIB_DEBUG_TAG
#endif

#ifdef VERSION_STR
#undef VERSION_STR
#endif

#ifdef VERSION_MAJOR
#undef VERSION_MAJOR
#endif

#ifdef VERSION_MINOR
#undef VERSION_MINOR
#endif

#ifdef VERSION_RELEA
#undef VERSION_RELEA
#endif


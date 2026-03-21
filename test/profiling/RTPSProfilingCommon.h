#ifndef _EPROSIMA_FASTDDS_PROFILING_COMMON_
#define _EPROSIMA_FASTDDS_PROFILING_COMMON_


#if defined(_WIN32)
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

#endif //_EPROSIMA_FASTDDS_PROFILING_COMMON_

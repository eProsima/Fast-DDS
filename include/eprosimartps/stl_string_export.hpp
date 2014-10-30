#if defined(_WIN32) && !defined(STL_STRING_EXPORT_APPLY)

#define STL_STRING_EXPORT(dllexport) \
    template class dllexport std::allocator<char>; \
    template class dllexport std::basic_string<char, std::char_traits<char>, std::allocator<char> >;
   
#define STL_STRING_EXPORT_APPLY 1

#else

#ifdef STL_STRING_EXPORT
#undef STL_STRING_EXPORT
#endif

#define STL_STRING_EXPORT(dllexport)

#endif //_WIN32

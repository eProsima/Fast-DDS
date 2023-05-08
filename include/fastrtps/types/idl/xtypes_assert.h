// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_IDL_XTYPES_ASSERT_H_
#define TYPES_IDL_XTYPES_ASSERT_H_

#include <iostream>
#include <sstream>

#if !defined(XTYPES_EXCEPTIONS)
#if !defined(NDEBUG)

#define xtypes_assert2_(cond, msg) xtypes_assert3_(cond, msg, false)

#ifdef WIN32

#ifndef _WINDOWS_
#   define NOMINMAX
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

#include <dbghelp.h>

#include <vector>
#include <iomanip>

namespace {
    // auxiliary structure
    struct symbol_desc : SYMBOL_INFO
    {
        CHAR _[1023]; // name buffer

        symbol_desc()
        {
            SizeOfStruct = sizeof(SYMBOL_INFO);
            MaxNameLen = 1024;
        }
    };
}

#define xtypes_assert3_(cond, msg, bt)                                                                              \
    {                                                                                                               \
        if (!(cond))                                                                                                \
        {                                                                                                           \
            using namespace std;                                                                                    \
            using tstring = basic_string<CHAR>;                                                                     \
                                                                                                                    \
            stringstream ss__;                                                                                      \
            ss__ << "[XTYPES]: ";                                                                                   \
            ss__ << __FILE__ << ":" << __LINE__ << " - ";                                                           \
            ss__ << "Assertion failed with message: ";                                                              \
            ss__ << msg << endl;                                                                                    \
            if (bt)                                                                                                 \
            {                                                                                                       \
                HANDLE hProcess = GetCurrentProcess();                                                              \
                                                                                                                    \
                if (SymInitialize(hProcess, NULL, TRUE))                                                            \
                {                                                                                                   \
                    symbol_desc symbol;                                                                             \
                    void* callstack[128];                                                                           \
                                                                                                                    \
                    int frames = CaptureStackBackTrace(0, 128, callstack, NULL);                                    \
                                                                                                                    \
                    for( int i = 0; i < frames; ++i)                                                                \
                    {                                                                                               \
                        SymFromAddr( hProcess, reinterpret_cast<DWORD64>(callstack[i]), 0, &symbol);                \
                        tstring name = symbol.Name;                                                                 \
                        if (name.empty())                                                                           \
                        {                                                                                           \
                            name = "unknown symbol";                                                                \
                        }                                                                                           \
                                                                                                                    \
                        ss__ << left << setw(4) << (frames -i -1) << showbase << hex                                \
                             << setw(19) << symbol.Address << right << name << dec << endl;                         \
                    }                                                                                               \
                                                                                                                    \
                    if (!SymCleanup(hProcess))                                                                      \
                    {                                                                                               \
                        DWORD error = GetLastError();                                                               \
                        ss__ << "SymCleanup returned error : " << error << endl;                                    \
                    }                                                                                               \
                }                                                                                                   \
                else                                                                                                \
                {                                                                                                   \
                    DWORD error = GetLastError();                                                                   \
                    ss__ << "SymInitialize returned error : " << error << endl;                                     \
                }                                                                                                   \
            }                                                                                                       \
            cerr << ss__.str() << endl;                                                                             \
            abort();                                                                                                \
        }                                                                                                           \
    }                                                                                                               \

#else // WIN32

#include <execinfo.h>

#define xtypes_assert3_(cond, msg, bt)                                                                              \
    {                                                                                                               \
        if (!(cond))                                                                                                \
        {                                                                                                           \
            std::stringstream ss__;                                                                                 \
            ss__ << "[XTYPES]: ";                                                                                   \
            ss__ << __FILE__ << ":" << __LINE__ << " - ";                                                           \
            ss__ << "Assertion failed with message: ";                                                              \
            ss__ << msg << std::endl;                                                                               \
            if (bt)                                                                                                 \
            {                                                                                                       \
                void* callstack[128];                                                                               \
                int frames = backtrace(callstack, 128);                                                             \
                char** symbols = backtrace_symbols(callstack, frames);                                              \
                ss__ << std::endl << "Backtrace:" << std::endl;                                                     \
                for (int i = 0; i < frames; ++i)                                                                    \
                {                                                                                                   \
                    ss__ << symbols[i] << std::endl;                                                                \
                }                                                                                                   \
                free(symbols);                                                                                      \
            }                                                                                                       \
            std::cerr << ss__.str() << std::endl;                                                                   \
            std::abort();                                                                                           \
        }                                                                                                           \
    }                                                                                                               \

#endif // WIN32

#else // NDEBUG

#define xtypes_assert2_(cond, msg)
#define xtypes_assert3_(cond, msg, bt)

#endif // NDEBUG

#else // XTYPES_EXCEPTIONS
#include <exception>
#define xtypes_assert2_(cond, msg) xtypes_assert3_(cond, msg, false)

#define xtypes_assert3_(cond, msg, bt)                                                                              \
    {                                                                                                               \
        if (!(cond))                                                                                                \
        {                                                                                                           \
            std::stringstream ss__;                                                                                 \
            ss__ << "[XTYPES]: ";                                                                                   \
            ss__ << __FILE__ << ":" << __LINE__ << " - ";                                                           \
            ss__ << "Assertion failed with message: ";                                                              \
            ss__ << msg << std::endl;                                                                               \
            throw std::runtime_error(ss__.str());                                                                   \
        }                                                                                                           \
    }                                                                                                               \

#endif // XTYPES_EXCEPTIONS

#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define xtypes_assert(...) GET_MACRO(__VA_ARGS__, xtypes_assert3_, xtypes_assert2_)(__VA_ARGS__)

namespace {

[[noreturn]] inline void unreachable()
{
#    ifdef __GNUC__ // GCC, Clang, ICC
    __builtin_unreachable();
#    elif defined(_MSC_VER) // MSVC
    __assume(false);
#     endif
}

} // namespace

#endif // TYPES_IDL_XTYPES_ASSERT_H_

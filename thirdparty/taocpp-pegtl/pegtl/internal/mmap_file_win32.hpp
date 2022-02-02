// Copyright (c) 2014-2022 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_MMAP_FILE_WIN32_HPP
#define TAO_PEGTL_INTERNAL_MMAP_FILE_WIN32_HPP

#if !defined( NOMINMAX )
#define NOMINMAX
#define TAO_PEGTL_NOMINMAX_WAS_DEFINED
#endif

#if !defined( WIN32_LEAN_AND_MEAN )
#define WIN32_LEAN_AND_MEAN
#define TAO_PEGTL_WIN32_LEAN_AND_MEAN_WAS_DEFINED
#endif

#include <windows.h>

#if defined( TAO_PEGTL_NOMINMAX_WAS_DEFINED )
#undef NOMINMAX
#undef TAO_PEGTL_NOMINMAX_WAS_DEFINED
#endif

#if defined( TAO_PEGTL_WIN32_LEAN_AND_MEAN_WAS_DEFINED )
#undef WIN32_LEAN_AND_MEAN
#undef TAO_PEGTL_WIN32_LEAN_AND_MEAN_WAS_DEFINED
#endif

#if !defined( __cpp_exceptions )
#include <cstdio>
#include <exception>
#endif

#include "filesystem.hpp"

namespace tao::pegtl::internal
{
   struct mmap_file_open
   {
      explicit mmap_file_open( const internal::filesystem::path& path )
         : m_path( path ),
           m_handle( open() )
      {}

      mmap_file_open( const mmap_file_open& ) = delete;
      mmap_file_open( mmap_file_open&& ) = delete;

      ~mmap_file_open()
      {
         ::CloseHandle( m_handle );
      }

      mmap_file_open& operator=( const mmap_file_open& ) = delete;
      mmap_file_open& operator=( mmap_file_open&& ) = delete;

      [[nodiscard]] std::size_t size() const
      {
         LARGE_INTEGER size;
         if( !::GetFileSizeEx( m_handle, &size ) ) {
#if defined( __cpp_exceptions )
            internal::error_code ec( ::GetLastError(), internal::system_category() );
            throw internal::filesystem::filesystem_error( "GetFileSizeEx() failed", m_path, ec );
#else
            std::perror( "GetFileSizeEx() failed" );
            std::terminate();
#endif
         }
         return std::size_t( size.QuadPart );
      }

      const internal::filesystem::path m_path;
      const HANDLE m_handle;

   private:
      [[nodiscard]] HANDLE open() const
      {
         SetLastError( 0 );
#if( _WIN32_WINNT >= 0x0602 )
         const HANDLE handle = ::CreateFile2( m_path.c_str(),
                                              GENERIC_READ,
                                              FILE_SHARE_READ,
                                              OPEN_EXISTING,
                                              nullptr );
         if( handle != INVALID_HANDLE_VALUE ) {
            return handle;
         }
#if defined( __cpp_exceptions )
         internal::error_code ec( ::GetLastError(), internal::system_category() );
         throw internal::filesystem::filesystem_error( "CreateFile2() failed", m_path, ec );
#else
         std::perror( "CreateFile2() failed" );
         std::terminate();
#endif
#else
         const HANDLE handle = ::CreateFileW( m_path.c_str(),
                                              GENERIC_READ,
                                              FILE_SHARE_READ,
                                              nullptr,
                                              OPEN_EXISTING,
                                              FILE_ATTRIBUTE_NORMAL,
                                              nullptr );
         if( handle != INVALID_HANDLE_VALUE ) {
            return handle;
         }
#if defined( __cpp_exceptions )
         internal::error_code ec( ::GetLastError(), internal::system_category() );
         throw internal::filesystem::filesystem_error( "CreateFileW()", m_path, ec );
#else
         std::perror( "CreateFileW() failed" );
         std::terminate();
#endif
#endif
      }
   };

   struct mmap_file_mmap
   {
      explicit mmap_file_mmap( const internal::filesystem::path& path )
         : mmap_file_mmap( mmap_file_open( path ) )
      {}

      explicit mmap_file_mmap( const mmap_file_open& reader )
         : m_size( reader.size() ),
           m_handle( open( reader ) )
      {}

      mmap_file_mmap( const mmap_file_mmap& ) = delete;
      mmap_file_mmap( mmap_file_mmap&& ) = delete;

      ~mmap_file_mmap()
      {
         ::CloseHandle( m_handle );
      }

      mmap_file_mmap& operator=( const mmap_file_mmap& ) = delete;
      mmap_file_mmap& operator=( mmap_file_mmap&& ) = delete;

      const size_t m_size;
      const HANDLE m_handle;

   private:
      [[nodiscard]] HANDLE open( const mmap_file_open& reader ) const
      {
         const uint64_t file_size = reader.size();
         SetLastError( 0 );
         // Use `CreateFileMappingW` because a) we're not specifying a
         // mapping name, so the character type is of no consequence, and
         // b) it's defined in `memoryapi.h`, unlike
         // `CreateFileMappingA`(?!)
         const HANDLE handle = ::CreateFileMappingW( reader.m_handle,
                                                     nullptr,
                                                     PAGE_READONLY,
                                                     DWORD( file_size >> 32 ),
                                                     DWORD( file_size & 0xffffffff ),
                                                     nullptr );
         if( handle != NULL || file_size == 0 ) {
            return handle;
         }
#if defined( __cpp_exceptions )
         internal::error_code ec( ::GetLastError(), internal::system_category() );
         throw internal::filesystem::filesystem_error( "CreateFileMappingW() failed", reader.m_path, ec );
#else
         std::perror( "CreateFileMappingW() failed" );
         std::terminate();
#endif
      }
   };

   class mmap_file_win32
   {
   public:
      explicit mmap_file_win32( const internal::filesystem::path& path )
         : mmap_file_win32( mmap_file_mmap( path ) )
      {}

      explicit mmap_file_win32( const mmap_file_mmap& mapper )
         : m_size( mapper.m_size ),
           m_data( static_cast< const char* >( ::MapViewOfFile( mapper.m_handle,
                                                                FILE_MAP_READ,
                                                                0,
                                                                0,
                                                                0 ) ) )
      {
         if( ( m_size != 0 ) && ( intptr_t( m_data ) == 0 ) ) {
#if defined( __cpp_exceptions )
            internal::error_code ec( ::GetLastError(), internal::system_category() );
            throw internal::filesystem::filesystem_error( "MapViewOfFile() failed", ec );
#else
            std::perror( "MapViewOfFile() failed" );
            std::terminate();
#endif
         }
      }

      mmap_file_win32( const mmap_file_win32& ) = delete;
      mmap_file_win32( mmap_file_win32&& ) = delete;

      ~mmap_file_win32()
      {
         ::UnmapViewOfFile( LPCVOID( m_data ) );
      }

      mmap_file_win32& operator=( const mmap_file_win32& ) = delete;
      mmap_file_win32& operator=( mmap_file_win32&& ) = delete;

      [[nodiscard]] bool empty() const noexcept
      {
         return m_size == 0;
      }

      [[nodiscard]] std::size_t size() const noexcept
      {
         return m_size;
      }

      using iterator = const char*;
      using const_iterator = const char*;

      [[nodiscard]] iterator data() const noexcept
      {
         return m_data;
      }

      [[nodiscard]] iterator begin() const noexcept
      {
         return m_data;
      }

      [[nodiscard]] iterator end() const noexcept
      {
         return m_data + m_size;
      }

   private:
      const std::size_t m_size;
      const char* const m_data;
   };

   using mmap_file_impl = mmap_file_win32;

}  // namespace tao::pegtl::internal

#endif

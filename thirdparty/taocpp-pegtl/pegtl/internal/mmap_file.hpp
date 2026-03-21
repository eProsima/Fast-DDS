// Copyright (c) 2022 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_MMAP_FILE_HPP
#define TAO_PEGTL_INTERNAL_MMAP_FILE_HPP

#if defined( __unix__ ) || ( defined( __APPLE__ ) && defined( __MACH__ ) )
#include <unistd.h>  // Required for _POSIX_MAPPED_FILES
#endif

#if defined( _POSIX_MAPPED_FILES )
#include "mmap_file_posix.hpp"
#elif defined( _WIN32 )
#include "mmap_file_win32.hpp"
#else
#endif

#include "filesystem.hpp"

namespace tao::pegtl::internal
{
   struct mmap_file
   {
      const mmap_file_impl data;

      explicit mmap_file( const internal::filesystem::path& path )
         : data( path )
      {}

      mmap_file( const mmap_file& ) = delete;
      mmap_file( mmap_file&& ) = delete;

      ~mmap_file() = default;

      mmap_file& operator=( const mmap_file& ) = delete;
      mmap_file& operator=( mmap_file&& ) = delete;
   };

}  // namespace tao::pegtl::internal

#endif

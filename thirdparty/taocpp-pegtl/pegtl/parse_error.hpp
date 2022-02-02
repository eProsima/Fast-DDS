// Copyright (c) 2014-2020 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#ifndef TAO_PEGTL_PARSE_ERROR_HPP
#define TAO_PEGTL_PARSE_ERROR_HPP

#include <stdexcept>
#include <vector>

#include "config.hpp"
#include "position.hpp"

namespace tao
{
   namespace TAO_PEGTL_NAMESPACE
   {
      struct parse_error
         : public std::runtime_error
      {
         parse_error( const std::string& msg, std::vector< position >&& in_positions )
            : std::runtime_error( msg ),
              positions( std::move( in_positions ) )
         {
         }

         template< typename Input >
         parse_error( const std::string& msg, const Input& in )
            : parse_error( msg, in.position() )
         {
         }

         parse_error( const std::string& msg, const position& pos )
            : std::runtime_error( to_string( pos ) + ": " + msg ),
              positions( 1, pos )
         {
         }

         parse_error( const std::string& msg, position&& pos )
            : std::runtime_error( to_string( pos ) + ": " + msg )
         {
            positions.emplace_back( std::move( pos ) );
         }

         std::vector< position > positions;
      };

   }  // namespace TAO_PEGTL_NAMESPACE

}  // namespace tao

#endif

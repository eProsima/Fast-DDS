// Copyright (c) 2014-2020 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#ifndef TAO_PEGTL_ANALYSIS_GRAMMAR_INFO_HPP
#define TAO_PEGTL_ANALYSIS_GRAMMAR_INFO_HPP

#include <map>
#include <string>
#include <utility>

#include "../config.hpp"
#include "../internal/demangle.hpp"

#include "rule_info.hpp"

namespace tao
{
   namespace TAO_PEGTL_NAMESPACE
   {
      namespace analysis
      {
         struct grammar_info
         {
            using map_t = std::map< std::string, rule_info >;
            map_t map;

            template< typename Name >
            std::pair< map_t::iterator, bool > insert( const rule_type type )
            {
               return map.insert( map_t::value_type( internal::demangle< Name >(), rule_info( type ) ) );
            }
         };

      }  // namespace analysis

   }  // namespace TAO_PEGTL_NAMESPACE

}  // namespace tao

#endif

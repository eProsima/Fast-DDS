// Copyright (c) 2014-2020 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#ifndef TAO_PEGTL_INTERNAL_OPT_HPP
#define TAO_PEGTL_INTERNAL_OPT_HPP

#include <type_traits>

#include "../config.hpp"

#include "seq.hpp"
#include "skip_control.hpp"
#include "trivial.hpp"

#include "../apply_mode.hpp"
#include "../rewind_mode.hpp"

#include "../analysis/generic.hpp"

namespace tao
{
   namespace TAO_PEGTL_NAMESPACE
   {
      namespace internal
      {
         template< typename... Rules >
         struct opt
            : opt< seq< Rules... > >
         {
         };

         template<>
         struct opt<>
            : trivial< true >
         {
         };

         template< typename Rule >
         struct opt< Rule >
         {
            using analyze_t = analysis::generic< analysis::rule_type::opt, Rule >;

            template< apply_mode A,
                      rewind_mode,
                      template< typename... >
                      class Action,
                      template< typename... >
                      class Control,
                      typename Input,
                      typename... States >
            static bool match( Input& in, States&&... st )
            {
               Control< Rule >::template match< A, rewind_mode::required, Action, Control >( in, st... );
               return true;
            }
         };

         template< typename... Rules >
         struct skip_control< opt< Rules... > > : std::true_type
         {
         };

      }  // namespace internal

   }  // namespace TAO_PEGTL_NAMESPACE

}  // namespace tao

#endif

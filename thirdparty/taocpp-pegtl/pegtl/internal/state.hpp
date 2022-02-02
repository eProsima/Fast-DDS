// Copyright (c) 2014-2020 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#ifndef TAO_PEGTL_INTERNAL_STATE_HPP
#define TAO_PEGTL_INTERNAL_STATE_HPP

#include "../config.hpp"

#include "seq.hpp"
#include "skip_control.hpp"

#include "../apply_mode.hpp"
#include "../rewind_mode.hpp"

#include "../analysis/generic.hpp"

namespace tao
{
   namespace TAO_PEGTL_NAMESPACE
   {
      namespace internal
      {
         template< typename State, typename... Rules >
         struct state
            : state< State, seq< Rules... > >
         {
         };

         template< typename State, typename Rule >
         struct state< State, Rule >
         {
            using analyze_t = analysis::generic< analysis::rule_type::seq, Rule >;

            template< apply_mode A,
                      rewind_mode M,
                      template< typename... >
                      class Action,
                      template< typename... >
                      class Control,
                      typename Input,
                      typename... States >
            static auto success( State& s, const Input& in, States&&... st )
               -> decltype( s.template success< A, M, Action, Control >( in, st... ), void() )
            {
               s.template success< A, M, Action, Control >( in, st... );
            }

            // NOTE: The additional "int = 0" is a work-around for missing expression SFINAE in VS2015.

            template< apply_mode,
                      rewind_mode,
                      template< typename... >
                      class Action,
                      template< typename... >
                      class Control,
                      typename Input,
                      typename... States,
                      int = 0 >
            static auto success( State& s, const Input& in, States&&... st )
               -> decltype( s.success( in, st... ), void() )
            {
               s.success( in, st... );
            }

            template< apply_mode A,
                      rewind_mode M,
                      template< typename... >
                      class Action,
                      template< typename... >
                      class Control,
                      typename Input,
                      typename... States >
            static bool match( Input& in, States&&... st )
            {
               State s( static_cast< const Input& >( in ), st... );

               if( Control< Rule >::template match< A, M, Action, Control >( in, s ) ) {
                  success< A, M, Action, Control >( s, in, st... );
                  return true;
               }
               return false;
            }
         };

         template< typename State, typename... Rules >
         struct skip_control< state< State, Rules... > > : std::true_type
         {
         };

      }  // namespace internal

   }  // namespace TAO_PEGTL_NAMESPACE

}  // namespace tao

#endif

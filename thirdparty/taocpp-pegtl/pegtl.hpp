// Copyright (c) 2014-2022 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_HPP
#define TAO_PEGTL_HPP

#include "pegtl/internal/demangle.hpp"
#include "pegtl/parse.hpp"
#include "pegtl/version.hpp"

#include "pegtl/ascii.hpp"
#include "pegtl/rules.hpp"
#include "pegtl/utf8.hpp"

#include "pegtl/argv_input.hpp"
#include "pegtl/buffer_input.hpp"
#include "pegtl/cstream_input.hpp"
#include "pegtl/istream_input.hpp"
#include "pegtl/memory_input.hpp"
#include "pegtl/read_input.hpp"
#include "pegtl/string_input.hpp"

// This has to be included *after* the above inputs,
// otherwise the amalgamated header will not work!
#include "pegtl/file_input.hpp"

#include "pegtl/change_action.hpp"
#include "pegtl/change_action_and_state.hpp"
#include "pegtl/change_action_and_states.hpp"
#include "pegtl/change_control.hpp"
#include "pegtl/change_state.hpp"
#include "pegtl/change_states.hpp"

#include "pegtl/disable_action.hpp"
#include "pegtl/enable_action.hpp"

#include "pegtl/discard_input.hpp"
#include "pegtl/discard_input_on_failure.hpp"
#include "pegtl/discard_input_on_success.hpp"

#if defined( __cpp_exceptions )
#include "pegtl/internal/if_must.hpp"
#endif

#endif

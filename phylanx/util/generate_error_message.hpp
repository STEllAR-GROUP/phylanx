//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_GENERATE_ERROR_MESSAGE_JUL_20_2018_0102PM)
#define PHYLANX_UTIL_GENERATE_ERROR_MESSAGE_JUL_20_2018_0102PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>

#include <string>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::string generate_error_message(std::string const& msg,
        std::string const& name = "", std::string const& codename = "");

    PHYLANX_EXPORT std::string generate_error_message(std::string const& msg,
        execution_tree::compiler::primitive_name_parts const& parts,
        std::string const& codename = "");
}}

#endif

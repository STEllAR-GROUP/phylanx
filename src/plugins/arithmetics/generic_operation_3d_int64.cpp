// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <phylanx/plugins/arithmetics/generic_operation.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_3d.hpp>

#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    template generic_operation::matrix_vector_function_ptr<std::int64_t>
    generic_operation::get_3d_function(std::string const& funcname,
        std::string const& name, std::string const& codename);
}}}

#endif

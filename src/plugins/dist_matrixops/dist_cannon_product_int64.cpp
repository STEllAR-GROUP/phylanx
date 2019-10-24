//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/dist_matrixops/dist_cannon_product.hpp>
#include <phylanx/plugins/dist_matrixops/dist_cannon_product_impl.hpp>

#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    // explicitly instantiate the required functions

    ///////////////////////////////////////////////////////////////////////////

    template execution_tree::primitive_argument_type dist_cannon_product::dot2d2d(
        ir::node_data<std::int64_t>&&, ir::node_data<std::int64_t>&&,
        execution_tree::localities_information&& lhs_localities,
        execution_tree::localities_information const& rhs_localities) const;

}}}

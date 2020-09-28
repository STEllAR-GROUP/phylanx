//  Copyright (c) 2017-2020 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/plugins/common/statistics_nd.hpp>
#include <phylanx/plugins/common/statistics_nd_impl.hpp>
#include <phylanx/plugins/common/statistics_operations.hpp>

#include <hpx/datastructures/optional.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// explicitly instantiate the required functions
namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    statisticsnd<statistics_max_op>(
        execution_tree::primitive_argument_type&& arg, ir::range&& axes,
        bool keepdims, execution_tree::primitive_argument_type&& initial,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx);

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    statisticsnd<statistics_max_op>(
        execution_tree::primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        execution_tree::primitive_argument_type&& initial,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx);

}}    // namespace phylanx::common

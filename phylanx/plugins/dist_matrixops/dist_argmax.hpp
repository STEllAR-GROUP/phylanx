// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_ARGMAX)
#define PHYLANX_PRIMITIVES_DIST_ARGMAX

#include <phylanx/config.hpp>
#include <phylanx/plugins/common/argminmax_operations.hpp>
#include <phylanx/plugins/dist_matrixops/dist_argminmax.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives {

    class dist_argmax : public dist_argminmax<common::argmax_op, dist_argmax>
    {
        using base_type = dist_argminmax<common::argmax_op, dist_argmax>;

    public:
        static execution_tree::match_pattern_type const match_data;

        dist_argmax() = default;

        dist_argmax(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline execution_tree::primitive create_dist_argmax(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return execution_tree::create_primitive_component(
            locality, "argmax_d", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::dist_matrixops::primitives

#endif

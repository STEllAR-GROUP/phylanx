//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018-2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_ARGMIN)
#define PHYLANX_PRIMITIVES_DIST_ARGMIN

#include <phylanx/config.hpp>
#include <phylanx/plugins/common/argminmax_operations.hpp>
#include <phylanx/plugins/dist_matrixops/dist_argminmax.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives {

    class dist_argmin : public dist_argminmax<common::argmin_op, dist_argmin>
    {
        using base_type = dist_argminmax<common::argmin_op, dist_argmin>;

    public:
        static execution_tree::match_pattern_type const match_data;

        dist_argmin() = default;

        dist_argmin(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline execution_tree::primitive create_dist_argmin(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return execution_tree::create_primitive_component(
            locality, "argmin_d", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::dist_matrixops::primitives

#endif

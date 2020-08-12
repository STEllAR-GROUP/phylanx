// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_STATISTICS_MAX_D_OPERATION)
#define PHYLANX_STATISTICS_MAX_D_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/common/statistics_operations.hpp>
#include <phylanx/plugins/dist_statistics/dist_statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Calculates the maximum of an array or maximum along an axis.
    /// \param a         The scalar, vector, or matrix to perform max over
    /// \param axis      Optional. If provided, max is calculated along the
    ///                  provided axis and a vector of results is returned.
    /// \param keep_dims Optional. If true the max value has to have the same
    ///                  number of dimensions as a. Otherwise, the axes with
    ///                  size one will be reduced.
    class max_d_operation
      : public dist_statistics_base<common::statistics_max_op, max_d_operation>
    {
        using base_type =
            dist_statistics_base<common::statistics_max_op, max_d_operation>;

    public:
        static match_pattern_type const match_data;

        max_d_operation() = default;

        max_d_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_amax_d_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "amax_d", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::execution_tree::primitives

#endif

// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_STATISTICS_MIN_OPERATION)
#define PHYLANX_STATISTICS_MIN_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/common/statistics_operations.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Calculates the minimum of an array or minimum along an axis.
    /// \param a         The scalar, vector, or matrix to perform min over
    /// \param axis      Optional. If provided, min is calculated along the
    ///                  provided axis and a vector of results is returned.
    /// \param keep_dims Optional. If true the min value has to have the same
    ///                  number of dimensions as a. Otherwise, the axes with
    ///                  size one will be reduced.
    class min_operation
      : public statistics_base<common::statistics_min_op, min_operation>
    {
        using base_type =
            statistics_base<common::statistics_min_op, min_operation>;

    public:
        static match_pattern_type const match_data;

        min_operation() = default;

        min_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_amin_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "amin", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::execution_tree::primitives

#endif

// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_STD)
#define PHYLANX_PRIMITIVES_STD

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/common/statistics_operations.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    /// This implementation is intended to behave like [NumPy implementation of std]
    /// (https://docs.scipy.org/doc/numpy-1.15.1/reference/generated/numpy.std.html).
    class std_operation
      : public statistics_base<common::statistics_stddev_op, std_operation>
    {
        using base_type =
            statistics_base<common::statistics_stddev_op, std_operation>;

    public:
        static match_pattern_type const match_data;

        std_operation() = default;

        std_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_std_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "std", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::execution_tree::primitives

#endif

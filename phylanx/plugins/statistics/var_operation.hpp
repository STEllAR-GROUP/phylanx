// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_VAR)
#define PHYLANX_PRIMITIVES_VAR

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        struct statistics_var_op;
    }

    /// This implementation is intended to behave like [NumPy implementation of var]
    /// (https://docs.scipy.org/doc/numpy-1.15.1/reference/generated/numpy.var.html).
    class var_operation
      : public statistics<detail::statistics_var_op, var_operation>
    {
        using base_type =
            statistics<detail::statistics_var_op, var_operation>;

    public:
        static match_pattern_type const match_data;

        var_operation() = default;

        var_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_var_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "var", std::move(operands), name, codename);
    }
}}}

#endif

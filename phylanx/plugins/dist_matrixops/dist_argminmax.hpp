// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_ARGMINMAX_2020_MAY_21_0502PM)
#define PHYLANX_PRIMITIVES_DIST_ARGMINMAX_2020_MAY_21_0502PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives {

    template <typename Op, typename Derived>
    class dist_argminmax
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<Derived>
    {
    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

    public:
        dist_argminmax() = default;

        dist_argminmax(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        execution_tree::primitive_argument_type argminmax0d(
            execution_tree::primitive_arguments_type&& args) const;
        execution_tree::primitive_argument_type argminmax1d(
            execution_tree::primitive_arguments_type&& args) const;
        execution_tree::primitive_argument_type argminmax2d(
            execution_tree::primitive_arguments_type&& args) const;
        execution_tree::primitive_argument_type argminmax3d(
            execution_tree::primitive_arguments_type&& args) const;
    };
}}}    // namespace phylanx::dist_matrixops::primitives

#endif

// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_dist_argminmax_IMPL_2020_MAY_21_0503PM)
#define PHYLANX_PRIMITIVES_DIST_dist_argminmax_IMPL_2020_MAY_21_0503PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/common/argminmax_nd.hpp>
#include <phylanx/plugins/dist_matrixops/dist_argminmax.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#include <phylanx/util/tensor_iterators.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    dist_argminmax<Op, Derived>::dist_argminmax(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    execution_tree::primitive_argument_type
    dist_argminmax<Op, Derived>::argminmax0d(
        execution_tree::primitive_arguments_type&& args) const
    {
        return common::argminmax0d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    execution_tree::primitive_argument_type
    dist_argminmax<Op, Derived>::argminmax1d(
        execution_tree::primitive_arguments_type&& args) const
    {
        return common::argminmax1d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    execution_tree::primitive_argument_type
    dist_argminmax<Op, Derived>::argminmax2d(
        execution_tree::primitive_arguments_type&& args) const
    {
        return common::argminmax2d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    execution_tree::primitive_argument_type
    dist_argminmax<Op, Derived>::argminmax3d(
        execution_tree::primitive_arguments_type&& args) const
    {
        return common::argminmax3d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    hpx::future<execution_tree::primitive_argument_type>
    dist_argminmax<Op, Derived>::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_argminmax<Op, Derived>::eval",
                generate_error_message(
                    "the dist_argminmax primitive requires exactly one or "
                    "two operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dist_argminmax<Op, Derived>::eval",
                    generate_error_message(
                        "the dist_argminmax primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    execution_tree::primitive_arguments_type&& args)
                    -> execution_tree::primitive_argument_type {

                    std::size_t a_dims =
                        execution_tree::extract_numeric_value_dimension(
                            args[0], this_->name_, this_->codename_);
                    switch (a_dims)
                    {
                    case 0:
                        return this_->argminmax0d(std::move(args));

                    case 1:
                        return this_->argminmax1d(std::move(args));

                    case 2:
                        return this_->argminmax2d(std::move(args));

                    case 3:
                        return this_->argminmax3d(std::move(args));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_argminmax<Op, Derived>::eval",
                            this_->generate_error_message(
                                "operand a has an invalid number of "
                                "dimensions"));
                    }
                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}    // namespace phylanx::dist_matrixops::primitives

#endif

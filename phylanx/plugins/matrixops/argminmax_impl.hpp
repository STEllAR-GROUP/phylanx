// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARGMINMAX_IMPL_2019_01_19_1130AM)
#define PHYLANX_PRIMITIVES_ARGMINMAX_IMPL_2019_01_19_1130AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/common/argminmax_nd.hpp>
#include <phylanx/plugins/matrixops/argminmax.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#include <phylanx/util/tensor_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

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
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    argminmax<Op, Derived>::argminmax(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    primitive_argument_type argminmax<Op, Derived>::argminmax0d(
        primitive_arguments_type&& args) const
    {
        return common::argminmax0d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    primitive_argument_type argminmax<Op, Derived>::argminmax1d(
        primitive_arguments_type&& args) const
    {
        return common::argminmax1d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    primitive_argument_type argminmax<Op, Derived>::argminmax2d(
        primitive_arguments_type&& args) const
    {
        return common::argminmax2d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    primitive_argument_type argminmax<Op, Derived>::argminmax3d(
        primitive_arguments_type&& args) const
    {
        return common::argminmax3d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    hpx::future<primitive_argument_type> argminmax<Op, Derived>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::eval",
                generate_error_message(
                    "the argminmax primitive requires exactly one or "
                    "two operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argminmax<Op, Derived>::eval",
                    generate_error_message(
                        "the argminmax primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                std::size_t a_dims = extract_numeric_value_dimension(
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
                        "argminmax<Op, Derived>::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

#endif

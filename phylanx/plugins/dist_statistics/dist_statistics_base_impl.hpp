// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVE_DIST_STATISTICS_IMPL_2020_JUN_19_1229PM)
#define PHYLANX_PRIMITIVE_DIST_STATISTICS_IMPL_2020_JUN_19_1229PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/statistics_nd.hpp>
#include <phylanx/plugins/dist_statistics/dist_statistics_base.hpp>

#include <hpx/assert.hpp>
#include <hpx/datastructures/optional.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    dist_statistics_base<Op, Derived>::dist_statistics_base(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statistics0d(
        primitive_argument_type&& arg, ir::range&& axes, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        return common::statisticsnd<Op>(std::move(arg), std::move(axes),
            keepdims, std::move(initial), dtype, name_, codename_,
            std::move(ctx));
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statistics1d(
        primitive_argument_type&& arg, ir::range&& axes, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        return common::statisticsnd<Op>(std::move(arg), std::move(axes),
            keepdims, std::move(initial), dtype, name_, codename_,
            std::move(ctx));
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statistics2d(
        primitive_argument_type&& arg, ir::range&& axes, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        return common::statisticsnd<Op>(std::move(arg), std::move(axes),
            keepdims, std::move(initial), dtype, name_, codename_,
            std::move(ctx));
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statistics3d(
        primitive_argument_type&& arg, ir::range&& axes, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        return common::statisticsnd<Op>(std::move(arg), std::move(axes),
            keepdims, std::move(initial), dtype, name_, codename_,
            std::move(ctx));
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statisticsnd(
        primitive_argument_type&& arg, ir::range&& axes, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        if (!arg.has_annotation())
        {
            return common::statisticsnd<Op>(std::move(arg), std::move(axes),
                keepdims, std::move(initial), dtype, name_, codename_,
                std::move(ctx));
        }

        std::size_t a_dims =
            extract_numeric_value_dimension(arg, name_, codename_);
        switch (a_dims)
        {
        case 0:
            return statistics0d(std::move(arg), std::move(axes), keepdims,
                std::move(initial), dtype, std::move(ctx));

        case 1:
            return statistics1d(std::move(arg), std::move(axes), keepdims,
                std::move(initial), dtype, std::move(ctx));

        case 2:
            return statistics2d(std::move(arg), std::move(axes), keepdims,
                std::move(initial), dtype, std::move(ctx));

        case 3:
            return statistics3d(std::move(arg), std::move(axes), keepdims,
                std::move(initial), dtype, std::move(ctx));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_argminmax<Op, Derived>::eval",
                generate_error_message(
                    "operand a has an invalid number of dimensions",
                    std::move(ctx)));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statistics0d(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        return common::statisticsnd<Op>(std::move(arg), axis, keepdims,
            std::move(initial), dtype, name_, codename_, std::move(ctx));
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statistics1d(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        return common::statisticsnd<Op>(std::move(arg), axis, keepdims,
            std::move(initial), dtype, name_, codename_, std::move(ctx));
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statistics2d(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        return common::statisticsnd<Op>(std::move(arg), axis, keepdims,
            std::move(initial), dtype, name_, codename_, std::move(ctx));
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statistics3d(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        return common::statisticsnd<Op>(std::move(arg), axis, keepdims,
            std::move(initial), dtype, name_, codename_, std::move(ctx));
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type dist_statistics_base<Op, Derived>::statisticsnd(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        primitive_argument_type&& initial, node_data_type dtype,
        eval_context ctx) const
    {
        if (!arg.has_annotation())
        {
            return common::statisticsnd<Op>(std::move(arg), axis, keepdims,
                std::move(initial), dtype, name_, codename_, std::move(ctx));
        }

        std::size_t a_dims =
            extract_numeric_value_dimension(arg, name_, codename_);
        switch (a_dims)
        {
        case 0:
            return statistics0d(std::move(arg), axis, keepdims,
                std::move(initial), dtype, std::move(ctx));

        case 1:
            return statistics1d(std::move(arg), axis, keepdims,
                std::move(initial), dtype, std::move(ctx));

        case 2:
            return statistics2d(std::move(arg), axis, keepdims,
                std::move(initial), dtype, std::move(ctx));

        case 3:
            return statistics3d(std::move(arg), axis, keepdims,
                std::move(initial), dtype, std::move(ctx));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_argminmax<Op, Derived>::eval",
                generate_error_message(
                    "operand a has an invalid number of dimensions",
                    std::move(ctx)));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    hpx::future<primitive_argument_type>
    dist_statistics_base<Op, Derived>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "statistics::eval",
                generate_error_message(
                    "the statistics primitive requires between one and five "
                    "operands", std::move(ctx)));
        }

        auto ctx_copy = ctx;
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::unwrapping(
            [this_ = std::move(this_), ctx = std::move(ctx)](
                primitive_arguments_type&& args) mutable
            -> primitive_argument_type
            {
                // Extract axis and keepdims
                // Presence of axis changes behavior for >1d cases
                hpx::util::optional<std::int64_t> axis;
                bool keepdims = false;
                primitive_argument_type initial;
                node_data_type dtype = node_data_type_double;

                if (args.size() > 1)
                {
                    // keepdims is (optional) argument #3
                    if (args.size() > 2 && valid(args[2]))
                    {
                        keepdims = extract_scalar_boolean_value(
                            args[2], this_->name_, this_->codename_);
                    }

                    // initial is (optional) argument #4
                    if (args.size() > 3)
                    {
                        initial = std::move(args[3]);
                    }

                    // dtype is (optional) argument #5
                    if (args.size() > 4 && valid(args[4]))
                    {
                        dtype =
                            map_dtype(extract_string_value(std::move(args[4]),
                                this_->name_, this_->codename_));
                    }

                    // axis is (optional) argument #2
                    if (valid(args[1]) && !is_explicit_nil(args[1]))
                    {
                        // the second argument is either a list of integers...
                        if (is_list_operand_strict(args[1]))
                        {
                            return this_->statisticsnd(std::move(args[0]),
                                extract_list_value_strict(std::move(args[1]),
                                    this_->name_, this_->codename_),
                                keepdims, std::move(initial), dtype,
                                std::move(ctx));
                        }

                        // ... or a single integer
                        axis = extract_scalar_integer_value_strict(
                            args[1], this_->name_, this_->codename_);
                    }
                }

                return this_->statisticsnd(std::move(args[0]), axis, keepdims,
                    std::move(initial), dtype, std::move(ctx));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx_copy)));
    }
}}}    // namespace phylanx::execution_tree::primitives

#endif

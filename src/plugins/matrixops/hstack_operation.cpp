// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/hstack_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const hstack_operation::match_data =
    {
        match_pattern_type{
            "hstack",
            std::vector<std::string>{"hstack(__1)"},
            &create_hstack_operation, &create_primitive<hstack_operation>, R"(
            args
            Args:

                *args (list of numbers, optional) : a list of numbers

            Returns:

            A list of numbers.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    hstack_operation::hstack_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    std::size_t hstack_operation::get_vecsize(
        primitive_arguments_type const& args) const
    {
        std::size_t vec_size = 0;

        for (std::size_t i = 0; i != args.size(); ++i)
        {
            std::size_t num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);

            if (num_dims != 0 && num_dims != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "hstack_operation::get_vecsize",
                    generate_error_message(
                        "for 0d/1d stacking, the hstack_operation primitive "
                        "requires the input be either a vector or a scalar"));
            }

            if (num_dims == 0)
            {
                ++vec_size;
            }
            else
            {
                vec_size += extract_numeric_value_dimensions(args[i])[1];
            }
        }

        return vec_size;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type hstack_operation::hstack0d1d_helper(
        primitive_arguments_type&& args) const
    {
        blaze::DynamicVector<T> result(get_vecsize(args));

        auto iter = result.begin();
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));
            std::size_t num_d = val.num_dimensions();
            if (num_d == 0)
            {
                *iter++ = val.scalar();
            }
            else
            {
                std::copy(val.vector().begin(), val.vector().end(), iter);
                iter += val.size();
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type hstack_operation::hstack0d1d(
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return hstack0d1d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return hstack0d1d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return hstack0d1d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "hstack_operation::hstack0d1d",
            generate_error_message(
                "the hstack_operation primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type hstack_operation::hstack2d_helper(
        primitive_arguments_type&& args) const
    {
        std::size_t args_size = args.size();

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
            extract_numeric_value_dimensions(args[0], name_, codename_);
        std::size_t total_cols = 0;

        for (std::size_t i = 0; i != args_size; ++i)
        {
            if (extract_numeric_value_dimension(args[i], name_, codename_) != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "hstack_operation::hstack2d_helper",
                    generate_error_message(
                        "for 2d stacking, the hstack_operation primitive "
                            "requires all the inputs be a matrices"));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            if (i != 0 && prevdim[0] != dim[0])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "hstack_operation::hstack2d_helper",
                    generate_error_message(
                        "the hstack_operation primitive requires the "
                            "number of rows to be equal for all matrices "
                            "being stacked"));
            }

            total_cols += dim[1];
            prevdim = dim;
        }

        blaze::DynamicMatrix<T> result(prevdim[0], total_cols);

        std::size_t step = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));

            std::size_t num_cols = val.dimension(1);
            for (std::size_t j = 0; j != num_cols; ++j)
            {
                blaze::column(result, j + step) = blaze::column(val.matrix(), j);
            }

            step += num_cols;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type hstack_operation::hstack2d(
        primitive_arguments_type&& args) const
    {
        switch (extract_common_type(args))
        {
        case node_data_type_bool:
            return hstack2d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return hstack2d_helper<std::int64_t>(std::move(args));

        case node_data_type_double:
            return hstack2d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "hstack_operation::hstack2d",
            generate_error_message(
                "the hstack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    hpx::future<primitive_argument_type> hstack_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty())
        {
            // hstack() without arguments returns an empty 1D vector
            using storage1d_type = ir::node_data<std::int64_t>::storage1d_type;
            return hpx::make_ready_future(primitive_argument_type{
                ir::node_data<std::int64_t>{storage1d_type(0)}});
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "hstack_operation::eval",
                generate_error_message(
                    "the hstack_operation primitive requires that "
                        "the arguments given by the operands array "
                        "are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                std::size_t matrix_dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);
                switch (matrix_dims)
                {
                case 0: HPX_FALLTHROUGH;
                case 1:
                    return this_->hstack0d1d(std::move(args));

                case 2:
                    return this_->hstack2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "hstack_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

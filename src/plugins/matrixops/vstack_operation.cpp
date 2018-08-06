//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/vstack_operation.hpp>

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
    match_pattern_type const vstack_operation::match_data =
    {
        hpx::util::make_tuple("vstack",
            std::vector<std::string>{"vstack(__1)"},
            &create_vstack_operation, &create_primitive<vstack_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    vstack_operation::vstack_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type vstack_operation::vstack0d_helper(
        std::vector<primitive_argument_type>&& args) const
    {
        std::size_t vec_size = args.size();

        blaze::DynamicMatrix<T> result(vec_size, 1);
        auto col = blaze::column(result, 0);

        std::size_t i = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));

            if (val.num_dimensions() != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "vstack_operation::vstack0d",
                    generate_error_message(
                        "the vstack_operation primitive requires all the "
                        "inputs be a scalar for 0d stacking"));
            }

            col[i++] = val.scalar();
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type vstack_operation::vstack0d(
        std::vector<primitive_argument_type>&& args) const
    {
        switch (extract_common_type(args))
        {
        case node_data_type_bool:
            return vstack0d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return vstack0d_helper<std::int64_t>(std::move(args));

        case node_data_type_double:
            return vstack0d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "hstack_operation::vstack0d",
            util::generate_error_message(
                "the vstack_operation primitive requires for all arguments to "
                    "be numeric data types",
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type vstack_operation::vstack1d2d_helper(
        std::vector<primitive_argument_type>&& args) const
    {
        std::size_t args_size = args.size();

        std::size_t num_dims_first =
            extract_numeric_value_dimension(args[0], name_, codename_);

        std::array<std::size_t, 2> prevdim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t total_rows = 1;
        std::size_t num_cols = prevdim[1];

        if (num_dims_first == 2)
        {
            total_rows = prevdim[0];
        }

        std::size_t first_size = num_cols;

        for (std::size_t i = 1; i != args_size; ++i)
        {
            std::size_t num_dims_second =
                extract_numeric_value_dimension(args[i], name_, codename_);

            if (num_dims_first == 0 || num_dims_second == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "vstack_operation::vstack2d",
                    util::generate_error_message(
                        "the vstack_operation primitive can not stack "
                        "matrices/vectors with a scalar",
                        name_, codename_));
            }

            std::array<std::size_t, 2> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            std::size_t second_size = dim[1];
            if (num_dims_second == 2)
            {
                total_rows += dim[0];
            }
            else
            {
                ++total_rows;
            }

            if (first_size != second_size)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "vstack_operation::vstack2d",
                    util::generate_error_message(
                        "the vstack_operation primitive requires for the "
                        "number of columns/size to be equal for all "
                        "matrices/vectors being stacked",
                        name_, codename_));
            }

            num_dims_first = num_dims_second;
            first_size = second_size;
        }

        blaze::DynamicMatrix<double> result(total_rows, num_cols);

        std::size_t step = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));
            if (val.num_dimensions() == 2)
            {
                std::size_t num_rows = val.dimension(0);
                for (std::size_t j = 0; j != num_rows; ++j)
                {
                    blaze::row(result, j + step) =
                        blaze::row(val.matrix(), j);
                }
                step += num_rows;
            }
            else
            {
                blaze::row(result, step) = blaze::trans(val.vector());
                ++step;
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type vstack_operation::vstack1d2d(
        std::vector<primitive_argument_type>&& args) const
    {
        switch (extract_common_type(args))
        {
        case node_data_type_bool:
            return vstack1d2d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return vstack1d2d_helper<std::int64_t>(std::move(args));

        case node_data_type_double:
            return vstack1d2d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "vstack_operation::vstack1d2d",
            util::generate_error_message(
                "the vstack_operation primitive requires for all arguments to "
                "be numeric data types",
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> vstack_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.empty())
        {
            // hstack() without arguments returns an empty column
            using storage2d_type = ir::node_data<std::int64_t>::storage2d_type;
            return hpx::make_ready_future(primitive_argument_type{
                ir::node_data<std::int64_t>{storage2d_type(0, 1)}});
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
                "vstack_operation::eval",
                util::generate_error_message(
                    "the vstack_operation primitive requires "
                        "that the arguments given by the operands "
                        "array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](std::vector<primitive_argument_type>&& args)
            -> primitive_argument_type
            {
                std::size_t matrix_dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);
                switch (matrix_dims)
                {
                case 0:
                    return this_->vstack0d(std::move(args));

                case 1: HPX_FALLTHROUGH;
                case 2:
                    return this_->vstack1d2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "vstack_operation::eval",
                        util::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> vstack_operation::eval(
        primitive_arguments_type const& args, eval_mode) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}

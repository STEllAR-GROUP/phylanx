//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/vstack_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
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
            &create_vstack_operation, &create_primitive<vstack_operation>,
            "args\n"
            "Args:\n"
            "\n"
            "  *args (list of number) : a list of numbers\n"
            "\n"
            "Returns:\n"
            "\n"
            "An N by 1 matrix of numbeers, where N is the quantity "
            "of numbers in `args`.")
    };

    ///////////////////////////////////////////////////////////////////////////
    vstack_operation::vstack_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type vstack_operation::vstack0d(args_type&& args) const
    {
        std::size_t vec_size = args.size();

        for (std::size_t i = 0; i < vec_size; ++i)
        {
            if (args[i].num_dimensions() != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "vstack_operation::vstack0d",
                    generate_error_message(
                        "the vstack_operation primitive requires all the "
                        "inputs be a scalar for 0d stacking"));
            }
        }

        blaze::DynamicMatrix<double> result(vec_size, 1);
        auto col = blaze::column(result, 0);

        for (std::size_t i = 0; i < vec_size; ++i)
        {
            col[i] = args[i].scalar();
        }

        return primitive_argument_type{
            ir::node_data<double>{storage2d_type{std::move(result)}}};
    }

    primitive_argument_type vstack_operation::vstack1d2d(args_type&& args) const
    {
        std::size_t args_size = args.size();
        std::size_t total_rows = args[0].dimension(0);
        std::size_t num_cols = args[0].dimension(1);
        std::size_t num_dims_first = args[0].num_dimensions();
        std::size_t num_dims_second = 0;
        std::size_t first_size = args[0].dimension(1);
        std::size_t second_size = 0;

        if (num_dims_first == 1)
        {
            total_rows = 1;
            num_cols = args[0].size();
            first_size = args[0].size();
        }

        for (std::size_t i = 1; i < args_size; ++i)
        {
            num_dims_second = args[i].num_dimensions();

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

            if (num_dims_second == 1)
            {
                second_size = args[i].dimension(0);
                total_rows++;
            }
            else
            {
                second_size = args[i].dimension(1);
                total_rows += args[i].dimension(0);
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

        blaze::DynamicMatrix<double> temp(total_rows, num_cols);

        std::size_t step = 0;

        for (std::size_t i = 0; i < args_size; ++i)
        {
            std::size_t num_rows = args[i].dimension(0);

            if (args[i].num_dimensions() == 2)
            {
                for (std::size_t j = 0; j < num_rows; ++j)
                {
                    blaze::row(temp, j + step) =
                        blaze::row(args[i].matrix(), j);
                }
                step = step + num_rows;
            }
            else
            {
                blaze::row(temp, step) = blaze::trans(args[i].vector());
                step++;
            }
        }

        return primitive_argument_type{
            ir::node_data<double>{storage2d_type{std::move(temp)}}};
    }

    hpx::future<primitive_argument_type> vstack_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.empty())
        {
            // hstack() without arguments returns an empty column
            return hpx::make_ready_future(primitive_argument_type{
                ir::node_data<double>{storage2d_type(0, 1)}});
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
            [this_](args_type&& args) -> primitive_argument_type
            {
                std::size_t matrix_dims = args[0].num_dimensions();
                switch (matrix_dims)
                {
                case 0:
                    return this_->vstack0d(std::move(args));

                case 1:
                    HPX_FALLTHROUGH;

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
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> vstack_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}

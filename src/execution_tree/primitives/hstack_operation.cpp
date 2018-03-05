// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/hstack_operation.hpp>
#include <phylanx/ir/node_data.hpp>

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
    primitive create_hstack_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("hstack");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const hstack_operation::match_data =
    {
        hpx::util::make_tuple("hstack",
            std::vector<std::string>{"hstack(_1, __2)"},
            &create_hstack_operation, &create_primitive<hstack_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    hstack_operation::hstack_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////

    std::size_t hstack_operation::get_vecsize(args_type& args) const
    {
        std::size_t vec_size = 0;
        std::size_t num_dims = 0;

        for (std::size_t i = 0; i < args.size(); ++i)
        {
            num_dims = args[i].num_dimensions();
            if (num_dims != 0 && num_dims != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "hstack_operation::is_stackable",
                    execution_tree::generate_error_message(
                        "the hstack_operation primitive requires the input "
                        "be either a vector or a scalar for 0d/1d stacking",
                        name_, codename_));
            }
            if (num_dims == 0)
            {
                vec_size = vec_size + 1;
            }
            else
            {
                vec_size = vec_size + args[i].size();
            }
        }

        return vec_size;
    }

    primitive_argument_type hstack_operation::hstack0d1d(args_type&& args) const
    {
        std::size_t vec_size = get_vecsize(args);
        std::size_t num_d = 0;
        blaze::DynamicVector<double> temp(vec_size);
        auto iter = temp.begin();

        for (std::size_t i = 0; i < args.size(); ++i)
        {
            num_d = args[i].num_dimensions();
            if (num_d == 0)
            {
                *iter++ = args[i].scalar();
            }
            else
            {
                std::copy(
                    args[i].vector().begin(), args[i].vector().end(), iter);
                iter += args[i].size();
            }
        }

        return primitive_argument_type{
            ir::node_data<double>{storage1d_type{std::move(temp)}}};
    }

    primitive_argument_type hstack_operation::hstack2d(args_type&& args) const
    {
        auto args_size = args.size();
        auto total_cols = args[0].dimension(1);

        for (std::size_t i = 0; i < args_size; ++i)
        {
            if (args[i].num_dimensions() != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "hstack_operation::hstack2d",
                    execution_tree::generate_error_message(
                        "the hstack_operation primitive requires all the "
                        "inputs be a matrix for 2d stacking",
                        name_,
                        codename_));
            }
        }
        for (std::size_t i = 1; i < args_size; ++i)
        {
            if (args[i - 1].dimension(0) != args[i].dimension(0))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "hstack_operation::hstack_operation",
                    execution_tree::generate_error_message(
                        "the hstack_operation primitive requires the "
                        "number of rows be equal for all matrix "
                        "being stacked",
                        name_, codename_));
            }

            total_cols += args[i].dimension(1);
        }

        blaze::DynamicMatrix<double> temp(args[0].dimension(0), total_cols);

        auto step = 0;

        for (std::size_t i = 0; i < args_size; ++i)
        {
            auto num_cols = args[i].dimension(1);
            for (std::size_t j = 0; j < num_cols; ++j)
            {
                blaze::column(temp, j + step) =
                    blaze::column(args[i].matrix(), j);
            }
            step = step + num_cols;
        }

        return primitive_argument_type{
            ir::node_data<double>{storage2d_type{std::move(temp)}}};
    }

    hpx::future<primitive_argument_type> hstack_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "hstack_operation::hstack_operation",
                execution_tree::generate_error_message(
                    "the hstack_operation primitive requires at least "
                        "two arguments",
                    name_, codename_));
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
                execution_tree::generate_error_message(
                    "the hstack_operation primitive requires that "
                        "the arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](args_type&& args) -> primitive_argument_type
            {
                std::size_t matrix_dims = args[0].num_dimensions();
                switch (matrix_dims)
                {
                case 0:
                    HPX_FALLTHROUGH;

                case 1:
                    return this_->hstack0d1d(std::move(args));

                case 2:
                    return this_->hstack2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "hstack_operation::eval",
                        execution_tree::generate_error_message(
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
    hpx::future<primitive_argument_type> hstack_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

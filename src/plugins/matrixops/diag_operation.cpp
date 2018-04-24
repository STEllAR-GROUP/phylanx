// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/diag_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cmath>
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
    match_pattern_type const diag_operation::match_data =
    {
        hpx::util::make_tuple("diag",
            std::vector<std::string>{"diag(_1, _2)"},
            &create_diag_operation, &create_primitive<diag_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    diag_operation::diag_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////

    primitive_argument_type diag_operation::diag0d(args_type&& args) const
    {
        return primitive_argument_type(std::move(args[0]));
    }

    primitive_argument_type diag_operation::diag1d(args_type&& args) const
    {
        auto vecsize = args[0].dimension(0);
        auto k = 0;
        auto incr = 0;

        if (args.size() == 2)
        {
            k = args[1].scalar();
            incr = std::abs(k);
        }

        blaze::DynamicMatrix<double> result(
            vecsize + incr, vecsize + incr, 0);

        blaze::Band<blaze::DynamicMatrix<double>> diag =
            blaze::band(result, k);

        diag = args[0].vector();

        return primitive_argument_type{ ir::node_data<double>{
            storage2d_type{std::move(result)}} };
    }

    primitive_argument_type diag_operation::diag2d(args_type&& args) const
    {
        auto k = 0;

        if (args.size() == 2)
        {
            k = args[1].scalar();
        }

        auto input_matrix = args[0].matrix();
        blaze::Band<blaze::CustomMatrix<double, true, true>> diag =
            blaze::band(input_matrix, k);

        blaze::DynamicVector<double> result(diag);

        return primitive_argument_type{ ir::node_data<double>{
            storage1d_type{std::move(result)}} };
    }

    hpx::future<primitive_argument_type> diag_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "diag_operation::diag_operation",
                execution_tree::generate_error_message(
                    "the diag_operation primitive requires "
                    "either one or two arguments",
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
                "diag_operation::eval",
                execution_tree::generate_error_message(
                    "the diag_operation primitive requires "
                    "that the arguments given by the operands "
                    "array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_](args_type&& args)
                -> primitive_argument_type {
            std::size_t matrix_dims = args[0].num_dimensions();
            switch (matrix_dims)
            {
            case 0:
                return this_->diag0d(std::move(args));

            case 1:
                return this_->diag1d(std::move(args));

            case 2:
                return this_->diag2d(std::move(args));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "diag_operation::eval",
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

    hpx::future<primitive_argument_type> diag_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

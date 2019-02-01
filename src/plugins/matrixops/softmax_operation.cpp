// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/softmax_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const softmax_operation::match_data =
    {
        hpx::util::make_tuple("softmax",
        std::vector<std::string>{"softmax(_1)","softmax(_1,_2)"},
        &create_softmax_operation, &create_primitive<softmax_operation>,
        R"(a, axis
        Args:

            a (array_like) : input array
            axis (optional, integer): an axis to softmax along. The
                default is the last axis (axis == -1) of an array. Axis
                is effective for >1d arrays.

        Returns:

        Returns an array of the same shape which is the normalized exponential
        function of the given array.  The resulting array consists of real
        values in the range (0..1], which add up to 1 in direction of the given axis)")
    };

    ///////////////////////////////////////////////////////////////////////////
    softmax_operation::softmax_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type softmax_operation::softmax0d() const
    {
        return primitive_argument_type{static_cast<double>(1.)};
    }

    primitive_argument_type softmax_operation::softmax1d(arg_type&& arg) const
    {
        auto v = arg.vector();
        return primitive_argument_type{blaze::softmax(v)};
    }

    primitive_argument_type softmax_operation::softmax2d_axis0(
        arg_type&& arg) const
    {
        auto m = arg.matrix();
        blaze::DynamicMatrix<double> result(m.rows(), m.columns());
        for (std::size_t i = 0; i < m.columns(); ++i)
        {
            blaze::column(result, i) = blaze::softmax(blaze::column(m, i));
        } // sum(result,axis=0) is a vector containing m.columns() ones
        return primitive_argument_type{result};
    }

    primitive_argument_type softmax_operation::softmax2d_axis1(
        arg_type&& arg) const
    {
        auto m = arg.matrix();
        blaze::DynamicMatrix<double> result(m.rows(), m.columns());
        for (std::size_t i = 0; i < m.rows(); ++i)
        {
            blaze::row(result, i) = blaze::softmax(blaze::row(m, i));
        } // sum(result,axis=1) is a vector containing m.rows() ones
        return primitive_argument_type{result};
    }

    primitive_argument_type softmax_operation::softmax2d(
        arg_type&& arg, std::int64_t axis) const
    {
        switch (axis)
        {
        case 0:
            return softmax2d_axis0(std::move(arg));

        case -1: HPX_FALLTHROUGH;
        case 1:
            return softmax2d_axis1(std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "softmax_operation::softmax2d",
                generate_error_message(
                    "the softmax_operation primitive requires operand axis "
                    "to be between -1 and 1 for matrices."));
        }
    }
    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> softmax_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "softmax_operation::eval",
                util::generate_error_message(
                    "the softmax_operation primitive requires exactly one, or "
                    "two operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "softmax_operation::eval",
                    util::generate_error_message(
                        "the softmax_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {
                // Extract axis
                std::int64_t axis =
                    static_cast<std::int64_t>(-1);    // column-wise operation

                // axis is the second argument
                if (args.size() > 1)
                {
                    if (valid(args[1]))
                        axis = execution_tree::extract_scalar_integer_value_strict(
                            args[1], this_->name_, this_->codename_);
                }

                // Extract the matrix
                arg_type a = extract_numeric_value(
                    args[0], this_->name_, this_->codename_);

                std::size_t a_dims = a.num_dimensions();

                switch (a_dims)
                {
                case 0:
                    return this_->softmax0d();
                case 1:
                    return this_->softmax1d(std::move(a));
                case 2:
                    return this_->softmax2d(std::move(a),axis);
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "softmax_operation::eval",
                        util::generate_error_message("operand a has an invalid "
                                                        "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}


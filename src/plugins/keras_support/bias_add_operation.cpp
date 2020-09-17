// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/bias_add_operation.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const bias_add_operation::match_data =
    {
        hpx::make_tuple("bias_add",
        std::vector<std::string>{R"(bias_add(_1, _2))"},
        &create_bias_add_operation,
        &create_primitive<bias_add_operation>,
        R"(x, bias
        Args:

            x (array_like) : input array of at least rank 2
            bias (array_like): a bias vector or an array of x_dims-1 dimensions

        Returns:

        Adds a bias array to an array)")
    };

    ///////////////////////////////////////////////////////////////////////////
    bias_add_operation::bias_add_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type bias_add_operation::bias_add2d(
        ir::node_data<double>&& arg,ir::node_data<double>&& bias) const
    {
        auto m = arg.matrix();
        std::size_t rows  = m.rows();
        std::size_t columns = m.columns();
        ir::node_data<double> b = extract_value_matrix<double>(
            std::move(bias), rows, columns, name_, codename_);

        if (!arg.is_ref())
        {
            arg = m + b.matrix();
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicMatrix<double> result(rows, columns);
            result = m + b.matrix();
            return primitive_argument_type{std::move(result)};
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type bias_add_operation::bias_add3d(
        ir::node_data<double>&& arg,ir::node_data<double>&& bias) const
    {
        auto t = arg.tensor();
        std::size_t pages = t.pages();
        std::size_t rows  = t.rows();
        std::size_t columns = t.columns();
        ir::node_data<double> b = extract_value_tensor<double>(
            std::move(bias), pages, rows, columns, name_, codename_);

        if (!arg.is_ref())
        {
            arg = t + b.tensor();
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicTensor<double> result(pages, rows, columns);
            result = t + b.tensor();
            return primitive_argument_type{std::move(result)};
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type bias_add_operation::bias_add4d(
        ir::node_data<double>&& arg,ir::node_data<double>&& bias) const
    {
        auto q = arg.quatern();
        std::size_t quats = q.quats();
        std::size_t pages = q.pages();
        std::size_t rows  = q.rows();
        std::size_t columns = q.columns();
        ir::node_data<double> b = extract_value_quatern<double>(
            std::move(bias), quats, pages, rows, columns, name_, codename_);

        //if (!arg.is_ref())
        //{
        //    arg = q + b.quatern();
        //    return primitive_argument_type{std::move(arg)};
        //}
        //else
        //{
        //    blaze::DynamicArray<4UL, double> result(
        //        quats, pages, rows, columns);
        //    result = q + b.quatern();
            blaze::DynamicArray<4UL, double> result = q;
            result += b.quatern();
            return primitive_argument_type{std::move(result)};
        //}
        return primitive_argument_type{std::move(arg)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> bias_add_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "bias_add_operation::eval",
                util::generate_error_message("the bias_add "
                                             "primitive requires two operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "bias_add_operation::eval",
                    util::generate_error_message(
                        "the bias_add primitive requires that the arguments "
                        "given by the operands array are valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {
            std::size_t x_ndim = extract_numeric_value_dimension(
                args[0], this_->name_, this_->codename_);
            std::size_t bias_ndim = extract_numeric_value_dimension(
                args[1], this_->name_, this_->codename_);

            if (bias_ndim != 1 && bias_ndim != x_ndim - 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "bias_add_operation::eval",
                    util::generate_error_message(
                        "bias has invalid number of dimensions",
                        this_->name_, this_->codename_));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims =
                extract_numeric_value_dimensions(
                    args[0], this_->name_, this_->codename_);
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& bias_dims =
                extract_numeric_value_dimensions(
                    args[1], this_->name_, this_->codename_);
            for (std::size_t i = 0; i != bias_ndim; ++i)
            {
                if (dims[x_ndim - i - 1] != bias_dims[bias_ndim - i - 1] &&
                    bias_dims[bias_ndim - i - 1] != 1)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "bias_add_operation::eval",
                        util::generate_error_message(
                            "bias has at least one incompatible dimension with "
                            "the original array",
                            this_->name_, this_->codename_));
            }

            switch (x_ndim)
            {
            case 2:
                return this_->bias_add2d(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_));
            case 3:
                return this_->bias_add3d(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_));

            case 4:
                return this_->bias_add4d(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "bias_add_operation::eval",
                    util::generate_error_message(
                        "operand a has an invalid "
                        "number of dimensions",
                        this_->name_, this_->codename_));
            }
        }),
        detail::map_operands(operands, functional::value_operand{}, args,
            name_, codename_, std::move(ctx)));
    }
}}}

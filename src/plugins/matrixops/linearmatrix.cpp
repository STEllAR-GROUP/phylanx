// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/matrixops/linearmatrix.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

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
    match_pattern_type const linearmatrix::match_data =
    {
        match_pattern_type{
            "linearmatrix",
            std::vector<std::string>{"linearmatrix(_1, _2, _3, _4, _5)"},
            &create_linearmatrix, &create_primitive<linearmatrix>, R"(
            nx, ny, x0, dx, dy
            Args:

                nx (int) : number of rows
                ny (int) : number of columns
                x0 (number) : value of the 0,0 element
                dx (number) : increment in value in the x-direction
                dy (number) : increment in value in the y-direction

            Returns:

            A matrix of size `nx` by `ny` with values beginning at `x0`
            and increasing by `dx` (or `dy`) as x (or y) is increased.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    linearmatrix::linearmatrix(primitive_arguments_type&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type linearmatrix::linmatrix(
        std::int64_t nx, std::int64_t ny, T x0, T dx, T dy) const
    {
        if (nx < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::linearmatrix::linmatrix",
                generate_error_message(
                    "the size of matrix in dimension x must at "
                        "least be one."));
        }

        if (ny < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::linearmatrix::linmatrix",
                generate_error_message(
                    "the size of matrix in dimension y must at "
                        "least be one."));
        }

        blaze::DynamicMatrix<T> result(nx, ny);

        for (std::int64_t x = 0; x < nx; ++x)
        {
            T base = x0 + dx * x;
            for (std::int64_t y = 0; y < ny; ++y)
            {
                result(x, y) = base + dy * y;
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type linearmatrix::linmatrix(std::int64_t nx,
        std::int64_t ny, primitive_argument_type&& x0,
        primitive_argument_type&& dx, primitive_argument_type&& dy) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(x0, dx, dy);
        }

        switch (t)
        {
        case node_data_type_int64:
            return linmatrix(nx, ny,
                extract_scalar_integer_value(std::move(x0), name_, codename_),
                extract_scalar_integer_value(std::move(dx), name_, codename_),
                extract_scalar_integer_value(std::move(dy), name_, codename_));

        case node_data_type_bool:   HPX_FALLTHROUGH;
        case node_data_type_double: HPX_FALLTHROUGH;
        case node_data_type_unknown:
            return linmatrix(nx, ny,
                extract_scalar_numeric_value(std::move(x0), name_, codename_),
                extract_scalar_numeric_value(std::move(dx), name_, codename_),
                extract_scalar_numeric_value(std::move(dy), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "linearmatrix::linmatrix",
            generate_error_message(
                "the linearmatrix primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> linearmatrix::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "linearmatrix",
                generate_error_message(
                    "the linearmatrix primitive requires exactly "
                        "five arguments."));
        }

        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "linearmatrix::eval",
                    generate_error_message(
                        "at least one of the arguments passed to "
                            "linearmatrix is not valid."));
            }
        }

        auto&& op0 =
            scalar_integer_operand(operands[0], args, name_, codename_, ctx);
        auto&& op1 =
            scalar_integer_operand(operands[1], args, name_, codename_, ctx);
        auto&& op2 = value_operand(operands[2], args, name_, codename_, ctx);
        auto&& op3 = value_operand(operands[3], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<std::int64_t>&& nx,
                    hpx::future<std::int64_t>&& ny,
                    hpx::future<primitive_argument_type>&& x0,
                    hpx::future<primitive_argument_type>&& dx,
                    hpx::future<primitive_argument_type>&& dy)
            -> primitive_argument_type
            {
                return this_->linmatrix(nx.get(), ny.get(), x0.get(),
                    dx.get(), dy.get());
            },
            std::move(op0), std::move(op1), std::move(op2), std::move(op3),
            value_operand(operands[4], args, name_, codename_, std::move(ctx)));
    }
}}}

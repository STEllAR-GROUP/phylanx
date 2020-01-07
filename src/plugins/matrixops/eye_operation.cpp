// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/eye_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cmath>
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
    match_pattern_type const eye_operation::match_data =
    {
        match_pattern_type{"eye",
            std::vector<std::string>{R"(
                eye(_1_N,
                    __arg(_2_M, nil),
                    __arg(_3_k, 0),
                    __arg(_4_dtype, "float")
                )
            )"},
            &create_eye_operation, &create_primitive<eye_operation>, R"(
            N, M, k, dtype
            Args:

                N (integer) : number of rows in the output.
                M (optional, integer) : number of columns in the output. If
                   None, defaults to N.
                k (optional, integer) : index of the diagonal: 0 (the default)
                  refers to the main diagonal, a positive value refers to an
                  upper diagonal, and a negative value to a lower diagonal.
                dtype (optional, string) : the data-type of the returned array,
                  defaults to 'float'.

            Returns:

            Return an N x M matrix with ones on the k-th diagonal and zeros
            elsewhere.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    eye_operation::eye_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    template <typename T>
    primitive_argument_type eye_operation::eye_n_helper(std::int64_t n) const
    {
        if (n < 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eye_n_helper",
                generate_error_message("input should be greater than zero"));
        }

        return primitive_argument_type{ir::node_data<T>{
            blaze::IdentityMatrix<T>(static_cast<std::size_t>(n))}};
    }

    primitive_argument_type eye_operation::eye_n(
        std::int64_t n, node_data_type dtype) const
    {
        switch (dtype)
        {
        case node_data_type_bool:
            return eye_n_helper<std::uint8_t>(n);

        case node_data_type_int64:
            return eye_n_helper<std::int64_t>(n);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return eye_n_helper<double>(n);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "eye_operation::eye_n",
            generate_error_message(
                "the eye primitive requires for all arguments to "
                "be numeric data types"));
    }

    template <typename T>
    primitive_argument_type eye_operation::eye_nmk_helper(
        std::int64_t n, std::int64_t m, std::int64_t k) const
    {
        if (n< 0 || m < 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eye_nm_helper",
                generate_error_message("inputs should be greater than zero"));
        }

        std::int64_t vecsize = 1;    // a positive number (k = 0)
        if (k > 0)
            vecsize = m - k;
        else if (k < 0)
            vecsize = n + k;

        blaze::DynamicMatrix<T> result(n, m, T(0));

        if (vecsize > 0)
            blaze::band(result, k) = T(1);

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type eye_operation::eye_nmk(std::int64_t n,
        std::int64_t m, std::int64_t k, node_data_type dtype) const
    {
        switch (dtype)
        {
        case node_data_type_bool:
            return eye_nmk_helper<std::uint8_t>(n, m, k);

        case node_data_type_int64:
            return eye_nmk_helper<std::int64_t>(n, m, k);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return eye_nmk_helper<double>(n, m, k);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "eye_operation::eye_nm",
            generate_error_message(
                "the eye primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> eye_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eval",
                util::generate_error_message(hpx::util::format(
                    "the eye_operation primitive can have one to "
                    "four operands, got {}", operands.size()),
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eval",
                util::generate_error_message(
                    "the eye_operation primitive requires that the "
                    "arguments given by the operands array are "
                    "valid",
                    name_, codename_));
        }

        // supply missing default arguments
        std::size_t numops = operands.size();
        primitive_arguments_type ops = operands;
        if (numops != 4)
        {
            ops.resize(4);
            ops[3] = primitive_argument_type{std::string("float")};
            if (numops < 3)
            {
                ops[2] = primitive_argument_type{std::int64_t(0)};
            }
            if (numops < 2)
            {
                ops[1] = primitive_argument_type{};
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](args_type&& args)
            -> primitive_argument_type
            {
                std::int64_t n = extract_scalar_integer_value(
                    std::move(args[0]), this_->name_, this_->codename_);

                std::int64_t m = valid(args[1]) ?
                    extract_scalar_integer_value(
                        std::move(args[1]), this_->name_, this_->codename_) :
                    n;

                std::int64_t k = valid(args[2]) ?
                    extract_scalar_integer_value(
                        std::move(args[2]), this_->name_, this_->codename_) :
                    0;

                node_data_type dtype = valid(args[3]) ?
                    map_dtype(extract_string_value(
                        std::move(args[3]), this_->name_, this_->codename_)) :
                    node_data_type_double;

                if (n == m && k == 0)
                {
                    return this_->eye_n(n, dtype);
                }
                return this_->eye_nmk(n, m, k, dtype);
            }),
            detail::map_operands(std::move(ops), functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}

//   Copyright (c) 2018 Bita Hasheminezhad
//   Copyright (c) 2018 Shahrzad Shirzad
//   Copyright (c) 2018 Hartmut kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/eye_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const eye_operation::match_data = {
        hpx::util::make_tuple("eye",
            std::vector<std::string>{"eye(_1)", "eye(_1,_2)", "eye(_1,_2,_3)"},
            &create_eye_operation, &create_primitive<eye_operation>,
            "N, M, k, data-type\n"
            "Args:\n"
            "\n"
            "")};

    ///////////////////////////////////////////////////////////////////////////
    eye_operation::eye_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {
    }
    template <typename T>
    primitive_argument_type eye_operation::eye_n_helper(val_type&& arg) const
    {
        if (extract_numeric_value_dimension(arg) != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eye_n_helper",
                generate_error_message("input should be a scalar"));
        }
        std::int64_t N = arg.scalar();
        if (N < 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eye_n_helper",
                generate_error_message("input should be greater than zero"));
        }
        std::size_t size = static_cast<std::size_t>(N);
        return primitive_argument_type{
            ir::node_data<T>{blaze::IdentityMatrix<T>(size)}};
    }

    primitive_argument_type eye_operation::eye_n(val_type&& arg) const
    {
        node_data_type t = dtype_;

        switch (t)
        {
        case node_data_type_bool:
            return eye_n_helper<std::uint8_t>(std::move(arg));

        case node_data_type_int64:
            return eye_n_helper<std::int64_t>(std::move(arg));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return eye_n_helper<double>(std::move(arg));

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
    primitive_argument_type eye_operation::eye_nmk_helper(args_type&& args) const
    {
        if (extract_numeric_value_dimension(args[0]) != 0 ||
            extract_numeric_value_dimension(args[1]) != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eye_n_helper",
                generate_error_message("input should be a scalar"));
        }
        auto N = args[0].scalar();
        auto M = args[1].scalar();
        auto k = 0;
        if (args.size() == 3)
        {
            if (extract_numeric_value_dimension(args[2]) != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "eye_operation::eye_n_helper",
                    generate_error_message("input should be a scalar"));
            }
            k = args[2].scalar();
        }

        if (N < 0 || M < 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eye_nm_helper",
                generate_error_message("inputs should be greater than zero"));
        }
        blaze::DynamicMatrix<T> result(N, M, T(0));

        auto vecsize = M;

        if (k > 0)
        {
            vecsize = M - k;
            if (vecsize < 0)
                vecsize = 0;
            else if (N < vecsize)
                vecsize = N;
        }
        else if (k < 0)
        {
            vecsize = N + k;
            if (vecsize < 0)
                vecsize = 0;
            else if (M < vecsize)
                vecsize = M;
        }

        if (vecsize > 0)
        {
            blaze::DynamicVector<T> ones(vecsize, T(1));
            blaze::Band<blaze::DynamicMatrix<T>> eye = blaze::band(result, k);
            eye = ones;
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }
    primitive_argument_type eye_operation::eye_nmk(args_type&& args) const
    {
        node_data_type t = dtype_;

        switch (t)
        {
        case node_data_type_bool:
            return eye_nmk_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return eye_nmk_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return eye_nmk_helper<double>(std::move(args));

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
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "eye_operation::eval",
                util::generate_error_message(
                    "the eye_operation primitive can have  one to"
                    "five operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "eye_operation::eval",
                    util::generate_error_message(
                        "the eye_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](args_type&& args)
                                      -> primitive_argument_type {
                switch (args.size())
                {
                case 1:
                    return this_->eye_n(std::move(args[0]));
                case 2:
                    return this_->eye_nmk(std::move(args));
                case 3:
                    return this_->eye_nmk(std::move(args));
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "eye_operation::eval",
                        util::generate_error_message("operand a has an invalid "
                                                     "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),

            detail::map_operands(operands, functional::integer_operand_strict{},
                args, name_, codename_, std::move(ctx)));
    }
}}}

//  Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/indices.hpp>
#include <phylanx/plugins/controls/fromfunction.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const fromfunction::match_data = {
        match_pattern_type{"fromfunction", std::vector<std::string>{R"(
                fromfunction(_1, _2, __arg(_2_dtype, "float"))
            )"},
            &create_fromfunction, &create_primitive<fromfunction>, R"(
            function, dimensions, dtype
            Args:

                function (callable) : The function is called with N parameters,
                  where N is the rank of 'dimension'. Each parameter represents
                  the coordinates of the array varying along a specific axis.
                dimensions (list of integers) : the shape of the array, which
                  also determines the shape of the coordinate arrays passed to
                  'function'.
                dtype (optional, string) : data-type of the coordinate arrays
                  passed to 'function', default is 'float'.

            Returns:

            The result of the call to 'function' is passed back directly.
            Therefore the shape of fromfunction is completely determined by
            'function'. If 'function' returns a scalar value, the shape of
            fromfunction would not match the shape parameter.)"}};

    ///////////////////////////////////////////////////////////////////////////
    fromfunction::fromfunction(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type fromfunction::call_fromfunction(
        primitive_argument_type&& func, ir::range&& shape, node_data_type dtype,
        eval_context ctx) const
    {
        primitive const* p = util::get_if<primitive>(&func);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fromfunction::call_fromfunction",
                generate_error_message(
                    "the first argument to fromfunction must be an invocable "
                    "object",
                    std::move(ctx)));
        }

        auto indices =
            common::generate_indices(shape, dtype, name_, codename_, ctx);
        auto args =
            extract_list_value_strict(std::move(indices), name_, codename_);
        if (!args.is_args())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fromfunction::call_fromfunction",
                generate_error_message(
                    "the value returned from common::indices is expected to "
                    "return a list of of arrays",
                    std::move(ctx)));
        }

        return p->eval(hpx::launch::sync, std::move(args.args()),
            add_frame(std::move(ctx), name_, codename_));
    }

    hpx::future<primitive_argument_type> fromfunction::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2 &&
            operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "fromfunction::eval",
                generate_error_message("the fromfunction primitive requires at "
                                       "most three operands",
                    std::move(ctx)));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "fromfunction::eval",
                generate_error_message(
                    "the fromfunction primitive requires that the "
                    "first and second arguments given are valid",
                    std::move(ctx)));
        }

        auto arg0 = value_operand(operands[0], args, name_, codename_,
            add_mode(ctx, eval_dont_evaluate_lambdas));
        auto arg1 =
            list_operand_strict(operands[1], args, name_, codename_, ctx);
        auto this_ = this->shared_from_this();

        if (operands.size() == 2)
        {
            return hpx::dataflow(
                hpx::launch::sync,
                [this_ = std::move(this_), ctx = std::move(ctx)](
                    hpx::future<primitive_argument_type>&& farg,
                    hpx::future<ir::range>&& fshape) mutable {
                    return this_->call_fromfunction(farg.get(), fshape.get(),
                        node_data_type_double, std::move(ctx));
                },
                std::move(arg0), std::move(arg1));
        }

        if (!valid(operands[2]) && !is_explicit_nil(operands[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "fromfunction::eval",
                generate_error_message(
                    "the fromfunction primitive requires that the "
                    "third argument is either valid or 'nil'"));
        }

        auto arg2 =
            string_operand_strict(operands[2], args, name_, codename_, ctx);
        return hpx::dataflow(
            hpx::launch::sync,
            [this_ = std::move(this_), ctx = std::move(ctx)](
                hpx::future<primitive_argument_type>&& farg,
                hpx::future<ir::range>&& fshape,
                hpx::future<std::string>&& fdtype) mutable {
                return this_->call_fromfunction(farg.get(), fshape.get(),
                    map_dtype(fdtype.get()), std::move(ctx));
            },
            std::move(arg0), std::move(arg1), std::move(arg2));
    }
}}}    // namespace phylanx::execution_tree::primitives

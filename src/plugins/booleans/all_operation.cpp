//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/booleans/all_operation.hpp>

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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const all_operation::match_data = {
        hpx::util::make_tuple("all", std::vector<std::string>{"all(_1)"},
            &create_all_operation, &create_primitive<all_operation>,
            "arg\n"
            "Args:\n"
            "\n"
            "    arg (matrix or vector of numbers) : the input values\n"
            "\n"
            "Returns:\n"
            "\n"
            "True if all values in the matrix/vector are nonzero,\n"
            "False otherwise.\n"
            )};

    ///////////////////////////////////////////////////////////////////////////
    all_operation::all_operation(primitive_arguments_type && args,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type all_operation::all0d(T&& arg) const
    {
        return primitive_argument_type{
            ir::node_data<std::uint8_t>{arg.scalar() != 0}};
    }

    template <typename T>
    primitive_argument_type all_operation::all1d(T&& arg) const
    {
        auto value = arg.vector();
        return primitive_argument_type{
            ir::node_data<std::uint8_t>{value.nonZeros() == value.size()}};
    }

    template <typename T>
    primitive_argument_type all_operation::all2d(T&& arg) const
    {
        auto value = arg.matrix();
        return primitive_argument_type{ir::node_data<std::uint8_t>{
            value.nonZeros() == value.rows() * value.columns()}};
    }

    template <typename T>
    primitive_argument_type all_operation::all_nd(T&& arg) const
    {
        auto dims = arg.num_dimensions();
        switch (dims)
        {
        case 0:
            return all0d(std::move(arg));

        case 1:
            return all1d(std::move(arg));

        case 2:
            return all2d(std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_operation::eval",
                generate_error_message(
                    "operand has unsupported number of dimensions"));
        }
    }

    hpx::future<primitive_argument_type> all_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_operation::eval",
                generate_error_message(
                    "the all_operation primitive requires one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_operation::eval",
                generate_error_message(
                    "the all_operation primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        hpx::future<primitive_argument_type> f =
            value_operand(operands[0], args, name_, codename_, std::move(ctx));

        return f.then(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& op)
            -> primitive_argument_type
            {
                switch (op.index())
                {
                case 1:
                    return this_->all_nd(util::get<1>(std::move(op)));

                case 2:
                    return this_->all_nd(util::get<2>(std::move(op)));

                case 4:
                    return this_->all_nd(util::get<4>(std::move(op)));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "all_operation::eval",
                        this_->generate_error_message(
                            "operand has unsupported type"));
                }
            }));
    }
}}}

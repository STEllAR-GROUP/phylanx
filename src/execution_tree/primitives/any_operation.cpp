//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/any_operation.hpp>
#include <phylanx/ir/node_data.hpp>

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
    primitive create_any_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("any");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const any_operation::match_data = {
        hpx::util::make_tuple("any", std::vector<std::string>{"any(_1)"},
            &create_any_operation, &create_primitive<any_operation>)};

    ///////////////////////////////////////////////////////////////////////////
    any_operation::any_operation(std::vector<primitive_argument_type> && args,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type any_operation::any0d(T&& arg) const
    {
        return primitive_argument_type{ir::node_data<std::uint8_t>{arg.scalar() != 0}};
    }

    template <typename T>
    primitive_argument_type any_operation::any1d(T&& arg) const
    {
        auto value = arg.vector();
        return primitive_argument_type{
            ir::node_data<std::uint8_t>{value.nonZeros() != 0}};
    }

    template <typename T>

    primitive_argument_type any_operation::any2d(T&& arg) const
    {
        auto value = arg.matrix();
        return primitive_argument_type{
            ir::node_data<std::uint8_t>{value.nonZeros() != 0}};
    }

    template <typename T>

    primitive_argument_type any_operation::any_nd(T&& arg) const
    {
        auto dims = arg.num_dimensions();
        switch (dims)
        {
        case 0:
            return any0d(std::move(arg));
        case 1:
            return any1d(std::move(arg));
        case 2:
            return any2d(std::move(arg));
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "any_operation::eval",
                execution_tree::generate_error_message(
                    "operand has unsupported "
                    "number of dimensions",
                    name_, codename_));
        }
    }

    hpx::future<primitive_argument_type> any_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.empty() || operands.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "any_operation::eval",
                execution_tree::generate_error_message(
                    "the any_operation primitive requires one "
                    "operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "any_operation::eval",
                execution_tree::generate_error_message(
                    "the any_operation primitive requires that the "
                    "arguments given by the operands array are "
                    "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        hpx::future<primitive_argument_type> f =
            value_operand(operands[0], args, name_, codename_);

        return f.then(hpx::launch::sync, hpx::util::unwrapping(
            [this_](primitive_argument_type&& op) -> primitive_argument_type {
                switch (op.index())
                {
                case 1:
                    return this_->any_nd(util::get<1>(std::move(op)));
                case 4:
                    return this_->any_nd(util::get<4>(std::move(op)));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "any_operation::eval",
                        execution_tree::generate_error_message(
                            "operand has unsupported "
                            "type",
                            this_->name_, this_->codename_));
                }
            }));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> any_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

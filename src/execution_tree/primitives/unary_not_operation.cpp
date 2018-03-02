//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/unary_not_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_unary_not_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__not");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const unary_not_operation::match_data =
    {
        hpx::util::make_tuple("__not",
            std::vector<std::string>{"!_1"},
            &create_unary_not_operation, &create_primitive<unary_not_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    unary_not_operation::unary_not_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type unary_not_operation::unary_not_all(operand_type&& ops) const
    {
        std::size_t dims = ops.num_dimensions();
        switch (dims)
        {
        case 0:
            return primitive_argument_type(
                ir::node_data<bool>{ops.scalar() == false});

        case 1:
            // TODO: SIMD functionality should be added, blaze implementation
            // is not currently available
            return primitive_argument_type(
                ir::node_data<bool>{blaze::map(
                    ops.vector(), [](bool x) { return x == false; })});

        case 2:
            // TODO: SIMD functionality should be added, blaze implementation
            // is not currently available
            return primitive_argument_type(
                ir::node_data<bool>{blaze::map(
                    ops.matrix(), [](bool x) { return x == false; })});

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not_operation::eval",
                execution_tree::generate_error_message(
                    "operand has unsupported number of dimensions",
                    name_, codename_));
        }
    }

    struct unary_not_operation::visit_unary_not
    {
        template <typename T>
        primitive_argument_type operator()(T&& ops) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not::eval",
                execution_tree::generate_error_message(
                    "operand has unsupported type",
                    that_.name_, that_.codename_));
        }

        primitive_argument_type operator()(
            ir::node_data<double>&& ops) const
        {
            return that_.unary_not_all(
                ir::node_data<bool>{std::move(ops)});
        }

        primitive_argument_type operator()(
            ir::node_data<bool>&& ops) const
        {
            return that_.unary_not_all(std::move(ops));
        }

        unary_not_operation const& that_;
    };

    hpx::future<primitive_argument_type> unary_not_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        //TODO: support for operands.size()>1
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not_operation::unary_not_operation",
                execution_tree::generate_error_message(
                    "the unary_not_operation primitive requires "
                        "exactly one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not_operation::unary_not_operation",
                execution_tree::generate_error_message(
                    "the unary_not_operation primitive requires "
                        "that the argument given by the operands "
                        "array is valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(
            hpx::util::unwrapping(
                [this_](
                    operands_type&& ops) -> primitive_argument_type {
                    return primitive_argument_type(
                        util::visit(visit_unary_not{*this_},
                            std::move(ops[0].variant())));

                }),
            detail::map_operands(
                operands, functional::literal_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    // Implement unary '!' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> unary_not_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

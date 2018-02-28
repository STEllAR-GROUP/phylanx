//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/all_operation.hpp>
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
    primitive create_all_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("all");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const all_operation::match_data = {
        hpx::util::make_tuple("all", std::vector<std::string>{"all(_1)"},
            &create_all_operation, &create_primitive<all_operation>)};

    ///////////////////////////////////////////////////////////////////////////
    all_operation::all_operation(std::vector<primitive_argument_type> && args,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////

    primitive_argument_type all_operation::all0d(args_type && args) const
    {
        return primitive_argument_type{
            ir::node_data<bool>{args[0].scalar() != 0}};
    }

    primitive_argument_type all_operation::all1d(args_type && args) const
    {
        auto value = args[0].vector();

        return primitive_argument_type{
            ir::node_data<bool>{value.nonZeros() == value.size()}};
    }

    primitive_argument_type all_operation::all2d(args_type && args) const
    {
        auto value = args[0].matrix();
        return primitive_argument_type{ir::node_data<bool>{
            value.nonZeros() == value.rows() * value.columns()}};
    }

    hpx::future<primitive_argument_type> all_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.empty() || operands.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_operation::eval",
                execution_tree::generate_error_message(
                    "the all_operation primitive requires one "
                    "operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_operation::eval",
                execution_tree::generate_error_message(
                    "the all_operation primitive requires that the "
                    "arguments given by the operands array are "
                    "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(
            hpx::util::unwrapping(
                [this_](args_type&& args) -> primitive_argument_type {
                    auto dims = args[0].num_dimensions();
                    switch (dims)
                    {
                    case 0:
                        return this_->all0d(std::move(args));
                    case 1:
                        return this_->all1d(std::move(args));
                    case 2:
                        return this_->all2d(std::move(args));
                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "all_operation::eval",
                            execution_tree::generate_error_message(
                                "operand has unsupported "
                                "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }),
            detail::map_operands(operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> all_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

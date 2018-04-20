// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/add_dimension.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

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
    primitive create_add_dimension(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("add_dim");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const add_dimension::match_data =
    {
        hpx::util::make_tuple("add_dim",
        std::vector<std::string>{"add_dim(_1)"},
        &create_add_dimension, &create_primitive<add_dimension>)
    };

    ///////////////////////////////////////////////////////////////////////////
    add_dimension::add_dimension(std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type add_dimension::add_dim_0d(args_type && args) const
    {
        return primitive_argument_type{
            blaze::DynamicVector<double>{args[0].scalar()}};
    }

    primitive_argument_type add_dimension::add_dim_1d(args_type && args) const
    {
        auto arg = args[0].vector();
        return primitive_argument_type{
            blaze::DynamicMatrix<double>{arg.size(), 1, arg.data()}};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> add_dimension::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_dimension::eval",
                execution_tree::generate_error_message(
                    "the add_dimension primitive requires exactly one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_dimension::eval",
                execution_tree::generate_error_message(
                    "the add_dimension primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](args_type&& args) -> primitive_argument_type
            {
                std::size_t a_dims = args[0].num_dimensions();
                switch (a_dims)
                {
                case 0:
                    return this_->add_dim_0d(std::move(args));
                case 1:
                    return this_->add_dim_1d(std::move(args));
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_dimension::eval",
                        execution_tree::generate_error_message(
                            "operand a has an invalid "
                            "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> add_dimension::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

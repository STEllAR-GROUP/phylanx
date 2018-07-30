// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/linspace.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const linspace::match_data =
    {
        hpx::util::make_tuple("linspace",
            std::vector<std::string>{"linspace(_1, _2, _3)"},
            &create_linspace, &create_primitive<linspace>)
    };

    ///////////////////////////////////////////////////////////////////////////
    linspace::linspace(std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type linspace::linspace1d(args_type&& args) const
    {
        double start = args[0][0];
        double stop = args[1][0];
        double num_samples = args[2][0];

        if (num_samples < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "detail::linspace1d",
                util::generate_error_message(
                    "the linspace primitive requires at least one "
                    "interval",
                    name_, codename_));
        }

        if (1 == num_samples)
        {
            vector_type result{ start };
            return primitive_argument_type{ arg_type{std::move(result)} };
        }
        else
        {
            auto result = vector_type(num_samples);
            double dx = (stop - start) / (num_samples - 1);
            for (std::size_t i = 0; i < num_samples; i++)
            {
                result[i] = start + dx * i;
            }
            return primitive_argument_type{ arg_type{std::move(result)} };
        }
    }

    hpx::future<primitive_argument_type> linspace::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::linspace",
                util::generate_error_message(
                    "the linspace primitive requires exactly three "
                    "arguments.",
                    name_, codename_));
        }

        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "linspace::eval",
                    util::generate_error_message(
                        "at least one of the arguments passed to "
                        "linspace is not valid.",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](args_type&& args) -> primitive_argument_type
            {
                return this_->linspace1d(std::move(args));
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> linspace::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}

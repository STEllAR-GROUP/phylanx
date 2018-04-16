// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/linearmatrix.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_linearmatrix(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("linearmatrix");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const linearmatrix::match_data =
    {
        hpx::util::make_tuple("linearmatrix",
            std::vector<std::string>{"linearmatrix(_1, _2, _3, _4, _5)"},
            &create_linearmatrix, &create_primitive<linearmatrix>)
    };

    ///////////////////////////////////////////////////////////////////////////
    linearmatrix::linearmatrix(std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type linearmatrix::linmatrix(args_type&& args) const
    {

        if (5 != args.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linearmatrix::linmatrix",
                execution_tree::generate_error_message(
                    "linearmatrix primitive requires five "
                        "arguments.",
                    name_, codename_));
        }

        std::size_t nx = extract_integer_value(args[0]);
        std::size_t ny = extract_integer_value(args[1]);
        double base_value = args[2].scalar();
        double dx = args[3].scalar();
        double dy = args[4].scalar();

        if (nx < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "detail::linearmatrix",
                execution_tree::generate_error_message(
                    "the size of matrix in dimension x must at "
                        "least be one.",
                    name_, codename_));
        }

        if (ny < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "detail::linearmatrix",
                execution_tree::generate_error_message(
                    "the size of matrix in dimension y must at "
                        "least be one.",
                    name_, codename_));
        }

        matrix_type result{nx, ny};

        for(int x = 0; x < nx; x++)
        {
            for(int y = 0; y < ny; y++)
            {
                result(x, y) = base_value + dx * x + dy * y;
            }
        }

        return arg_type{std::move(result)};
    }

    hpx::future<primitive_argument_type> linearmatrix::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "linearmatrix",
                execution_tree::generate_error_message(
                    "the linearmatrix primitive requires exactly "
                        "five arguments.",
                    name_, codename_));
        }

        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "linearmatrix::eval",
                    execution_tree::generate_error_message(
                        "at least one of the arguments passed to "
                            "linearmatrix is not valid.",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](args_type&& args) -> primitive_argument_type
            {
                return this_->linmatrix(std::move(args));
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> linearmatrix::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

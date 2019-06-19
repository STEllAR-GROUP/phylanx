//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/solvers/decomposition.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
///////////////////////////////////////////////////////////////////////////////
#define PHYLANX_DECOM_MATCH_DATA(name)                                         \
    match_pattern_type{name, std::vector<std::string>{name "(_1)"},            \
        &create_decomposition, &create_primitive<decomposition>,               \
        "m\n"                                                                  \
        "Args:\n"                                                              \
        "\n"                                                                   \
        "    m (matrix): a matrix"                                             \
        "\n"                                                                   \
        "Returns:\n"                                                           \
        "\n"                                                                   \
        "Computes LU decomposition of a general matrix in form of "            \
        "A = L*U*P where P is a permutation matrix, L is a lower "             \
        "triangular matrix, and U is an upper triangular matrix. "             \
    }                                                                          \
    /**/

    std::vector<match_pattern_type> const decomposition::match_data = {
        PHYLANX_DECOM_MATCH_DATA("lu")};

#undef PHYLANX_DECOM_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    decomposition::vector_function_ptr decomposition::get_decomposition_map(
        std::string const& name) const
    {
        static std::map<std::string, vector_function_ptr> decompositions = {{"lu",
            //computes LU decomposition of a general matrix in form of
            // A = L*U*P where P is a permutation matrix, L is a lower
            // triangular matrix, and U is an upper triangular matrix.
            [](args_type&& args) -> primitive_argument_type {
                storage2d_type P, L, U;

                if (!args[0].is_ref())
                {
                    blaze::lu(args[0].matrix(), L, U, P);
                }
                else
                {
                    storage2d_type A{(args[0].matrix())};
                    blaze::lu(A, L, U, P);
                }
                return primitive_argument_type{
                    primitive_arguments_type{
                        primitive_argument_type{L}, primitive_argument_type{U},
                        primitive_argument_type{P}}};
            }}};
        return decompositions[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::string function_name(std::string const& name)
        {
            compiler::primitive_name_parts name_parts;
            if (!compiler::parse_primitive_name(name, name_parts))
            {
                return name;
            }

            return name_parts.primitive;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    decomposition::decomposition(
        primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
        std::string func_name = detail::function_name(name);

        func_ = get_decomposition_map(func_name);

        HPX_ASSERT(func_ != nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type decomposition::calculate_decomposition(
        args_type && op) const
    {
        return primitive_argument_type{func_(std::move(op))};
    }

    hpx::future<primitive_argument_type> decomposition::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "decomposition::eval",
                util::generate_error_message(
                    "the decomposition  primitive "
                    "requires exactly one operands ",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "decomposition_operation::eval",
                util::generate_error_message(
                    "the decomposition primitive requires "
                    "that the argument given by the operands "
                    "array is valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](args_type&& args)
            -> primitive_argument_type
            {
                if (args[0].num_dimensions() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "decomposition_operation::eval",
                        util::generate_error_message(
                            "the decomposition primitive "
                            "requires the operand to be a matrix ",
                            this_->name_, this_->codename_));
                }

                return this_->calculate_decomposition(std::move(args));
            }),
            detail::map_operands(operands, functional::numeric_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}


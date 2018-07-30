// Copyright (c) 2018 Shahrzad Shirzad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/solvers/linear_solver.hpp>

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
#define PHYLANX_LIN_MATCH_DATA(name)                                           \
    hpx::util::make_tuple(name,                                                \
        std::vector<std::string>{name "(_1, _2)", name "(_1, _2, _3)"},        \
        &create_linear_solver, &create_primitive<linear_solver>)
    /**/

    std::vector<match_pattern_type> const linear_solver::match_data = {
        PHYLANX_LIN_MATCH_DATA("linear_solver_lu"),
        PHYLANX_LIN_MATCH_DATA("linear_solver_ldlt"),
        PHYLANX_LIN_MATCH_DATA("linear_solver_cholesky")};

#undef PHYLANX_LIN_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    linear_solver::vector_function_ptr linear_solver::get_lin_solver_map(
        std::string const& name) const
    {
        static std::map<std::string, vector_function_ptr> lin_solver = {
            {"linear_solver_lu",
                // Linear solver based on LU decomposition of a general square matrix
                [](args_type&& args) -> arg_type {
                    storage2d_type A{blaze::trans(args[0].matrix())};
                    storage1d_type b{args[1].vector()};
                    const std::unique_ptr<int[]> ipiv(new int[b.size()]);
                    blaze::gesv(A, b, ipiv.get());
                    return arg_type{std::move(b)};
                }},
            {"linear_solver_ldlt",
                // Linear solver based on LDLT decomposition of a square
                // symmetric indefinite matrix
                [](args_type&& args) -> arg_type {
                    storage2d_type A{blaze::trans(args[0].matrix())};
                    storage1d_type b{args[1].vector()};
                    const std::unique_ptr<int[]> ipiv(new int[b.size()]);
                    blaze::sysv(A, b, 'U', ipiv.get());
                    return arg_type{std::move(b)};
                }},
            {"linear_solver_cholesky",
                // Linear solver based on cholesky(LLH) decomposition of a
                // square positive definite matrix
                [](args_type&& args) -> arg_type {
                    storage2d_type A{blaze::trans(args[0].matrix())};
                    storage1d_type b{args[1].vector()};
                    blaze::posv(A, b, 'U');
                    return arg_type{std::move(b)};
                }}};
        return lin_solver[name];
    }

    linear_solver::vector_function_ptr_ul linear_solver::get_lin_solver_map_ul(
        std::string const& name) const
    {
        static std::map<std::string, vector_function_ptr_ul> lin_solver = {
            {"linear_solver_ldlt",
                // Linear solver based on LDLT decomposition of a symmetric
                // indefinite matrix
                [](arg_type&& arg_0, arg_type&& rhs,
                    std::string ul) -> arg_type {
                    storage2d_type A{blaze::trans(arg_0.matrix())};
                    storage1d_type b{rhs.vector()};
                    const std::unique_ptr<int[]> ipiv(new int[b.size()]);
                    blaze::sysv(A, b, ul == "L" ? 'L' : 'U', ipiv.get());
                    return arg_type{std::move(b)};
                }},
            {"linear_solver_cholesky",
                // Linear solver based on cholesky(LLH) decomposition of a
                // positive definite matrix
                [](arg_type&& arg_0, arg_type&& rhs,
                    std::string ul) -> arg_type {
                    storage2d_type A{blaze::trans(arg_0.matrix())};
                    storage1d_type b{rhs.vector()};
                    blaze::posv(A, b, ul == "L" ? 'L' : 'U');
                    return arg_type{std::move(b)};
                }}};
        return lin_solver[name];
    }
    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        std::string extract_function_name(std::string const& name)
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
    linear_solver::linear_solver(
        std::vector<primitive_argument_type> && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
        std::string func_name = detail::extract_function_name(name);

        func_ = get_lin_solver_map(func_name);
        func_ul_ = get_lin_solver_map_ul(func_name);

        HPX_ASSERT(func_ != nullptr || func_ul_ != nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type linear_solver::calculate_linear_solver(
        args_type && op) const
    {
        if (func_ == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linear_solver::eval",
                util::generate_error_message(
                    "this linear_solver primitive "
                    "requires exactly three operands",
                    name_, codename_));
        }
        return primitive_argument_type{func_(std::move(op))};
    }

    primitive_argument_type linear_solver::calculate_linear_solver(
        arg_type && lhs, arg_type && rhs, std::string ul) const
    {
        if (func_ul_ == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linear_solver::eval",
                util::generate_error_message(
                    "this linear_solver primitive "
                    "requires exactly two operands",
                    name_, codename_));
        }
        return primitive_argument_type{
            func_ul_(std::move(lhs), std::move(rhs), std::move(ul))};
    }

    hpx::future<primitive_argument_type> linear_solver::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 2 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linear_solver::eval",
                util::generate_error_message(
                    "the linear_solver primitive "
                    "requires exactly two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]) ||
            (operands.size() == 3 && !valid(operands[2])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linear_solver_operation::eval",
                util::generate_error_message(
                    "the linear_solver primitive requires "
                    "that the arguments given by the operands "
                    "array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 3)
        {
            return hpx::dataflow(hpx::launch::sync,
                hpx::util::unwrapping(
                    [this_](arg_type&& lhs,
                        arg_type&& rhs,
                        std::string ul) -> primitive_argument_type {
                        if (lhs.num_dimensions() != 2 ||
                            rhs.num_dimensions() != 1)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "linear_solver::eval",
                                util::generate_error_message(
                                    "the linear_solver primitive "
                                    "requires "
                                    "that first operand to be a matrix and "
                                    "the second "
                                    "operand to be a vector",
                                    this_->name_, this_->codename_));
                        }
                        if (ul != "L" && ul != "U")
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "linear_solver::eval",
                                util::generate_error_message(
                                    "the linear_solver primitive "
                                    "requires "
                                    "that third argument to be either 'L' "
                                    "or 'U'",
                                    this_->name_, this_->codename_));
                        }

                        return this_->calculate_linear_solver(
                            std::move(lhs), std::move(rhs), std::move(ul));
                    }),
                numeric_operand(operands[0], args, name_, codename_),
                numeric_operand(operands[1], args, name_, codename_),
                string_operand(operands[2], args, name_, codename_));
        }
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_](args_type&& args) -> primitive_argument_type {
                    if (args[0].num_dimensions() != 2 ||
                        args[1].num_dimensions() != 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "linear_solver_operation::eval",
                            util::generate_error_message(
                                "the linear_solver_operation primitive "
                                "requires "
                                "that first operand to be a matrix and "
                                "the second "
                                "operand to be a vector",
                                this_->name_, this_->codename_));
                    }

                    return this_->calculate_linear_solver(std::move(args));
                }),
            detail::map_operands(operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> linear_solver::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}


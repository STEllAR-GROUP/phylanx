//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/booleans/nonzero_where.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <array>
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
    std::vector<match_pattern_type> const nonzero_where::match_data =
    {
        match_pattern_type{"nonzero",
            std::vector<std::string>{"nonzero(_1)"},
            &create_nonzero, &create_primitive<nonzero_where>, R"(
            arg
            Args:

                arg (vector or matrix) : a vector or matrix

            Returns:

            A 1D array containing the indices of the elements of `arg` that are
            non-zero.
            )"
        },

        match_pattern_type{"where",
            std::vector<std::string>{"where(_1)", "where(_1, _2, _3)"},
            &create_where, &create_primitive<nonzero_where>, R"(
            cond, var, val
            Args:

                cond (boolean expression): a condition to apply
                var (matrix, optional): a matrix (used if cond is true)
                val (matrix, optional): a value to supply when cond is false

            Returns:

            A new matrix in which cells in matrix `var` that have values
            that do not match `cond` are replaced with value `val`.

            Note:

            Either both of or none of `var` and `val` can be provided. If none
            are provided, then `where` is equivalent to `cond.nonzero()`.
            )"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    std::string nonzero_where::extract_function_name(
        std::string const& name)
    {
        compiler::primitive_name_parts name_parts;
        if (!compiler::parse_primitive_name(name, name_parts))
        {
            std::string::size_type p = name.find_first_of("$");
            if (p != std::string::npos)
            {
                return name.substr(0, p);
            }
        }

        return name_parts.primitive;
    }

    nonzero_where::nonzero_where(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , nonzero_(false)
      , where_(false)
    {
        auto func_name = extract_function_name(name_);
        if (func_name == "nonzero")
        {
            nonzero_ = true;
        }
        else
        {
            HPX_ASSERT(func_name == "where");
            where_ = true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type nonzero_where::nonzero_elements(
        ir::node_data<T> && op) const
    {
        using storage1d_type =
            typename ir::node_data<std::int64_t>::storage1d_type;

        std::size_t dims = op.num_dimensions();
        switch (dims)
        {
        case 0:
            {
                storage1d_type indices(
                    op.scalar() != T(0) ? 1 : 0, std::int64_t(0));

                primitive_arguments_type result;
                result.reserve(1);
                result.emplace_back(std::move(indices));
                return primitive_argument_type{std::move(result)};
            }

        case 1:
            {
                auto v = op.vector();
                storage1d_type indices(v.size());
                std::size_t count = 0;
                for (std::size_t i = 0; i != v.size(); ++i)
                {
                    if (v[i] != T(0))
                    {
                        indices[count++] = std::int64_t(i);
                    }
                }
                indices.resize(count);
                indices.shrinkToFit();

                primitive_arguments_type result;
                result.reserve(1);
                result.emplace_back(std::move(indices));
                return primitive_argument_type{std::move(result)};
            }

        case 2:
            {
                auto m = op.matrix();
                storage1d_type indices_row(op.size());
                storage1d_type indices_column(op.size());
                std::size_t count = 0;

                for (std::size_t i = 0; i != m.rows(); ++i)
                {
                    for (std::size_t j = 0; j != m.columns(); ++j)
                    {
                        if (m(i, j) != T(0))
                        {
                            indices_row[count] = std::int64_t(i);
                            indices_column[count++] = std::int64_t(j);
                        }
                    }
                }

                indices_row.resize(count);
                indices_row.shrinkToFit();

                indices_column.resize(count);
                indices_column.shrinkToFit();

                primitive_arguments_type result;
                result.reserve(2);
                result.emplace_back(std::move(indices_row));
                result.emplace_back(std::move(indices_column));
                return primitive_argument_type{std::move(result)};
            }

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "nonzero::eval",
                util::generate_error_message(
                    "operand has unsupported number of dimensions",
                    name_, codename_));
        }
    }

    struct nonzero_where::visit_nonzero
    {
        template <typename T>
        primitive_argument_type operator()(T&& op) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "nonzero_where::visit_nonzero::operator()",
                util::generate_error_message(
                    "operand has unsupported type",
                    that_.name_, that_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(ir::node_data<T>&& op) const
        {
            return that_.nonzero_elements(std::move(op));
        }

        nonzero_where const& that_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // where condition is zero dimensional
    template <typename R, typename T>
    primitive_argument_type nonzero_where::where_elements0d(
        ir::node_data<T>&& op, primitive_argument_type&& lhs,
        primitive_argument_type&& rhs) const
    {
        switch (extract_largest_dimension(name_, codename_, lhs, rhs))
        {
        case 0:
            return primitive_argument_type{extract_value_scalar<R>(
                op.scalar() ? std::move(lhs) : std::move(rhs),
                name_, codename_)};

        case 1:
            {
                std::size_t size =
                    extract_largest_dimensions(name_, codename_, lhs, rhs)[1];
                return primitive_argument_type{extract_value_vector<R>(
                    op.scalar() ? std::move(lhs) : std::move(rhs),
                    size, name_, codename_)};
            }

        case 2:
            {
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> sizes =
                    extract_largest_dimensions(name_, codename_, lhs, rhs);
                return primitive_argument_type{extract_value_matrix<R>(
                    op.scalar() ? std::move(lhs) : std::move(rhs),
                    sizes[0], sizes[1], name_, codename_)};
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "nonzero_where::where_elements",
            util::generate_error_message(
                "operands have unsupported number of dimensions",
                name_, codename_));
    }

    // where condition is one dimensional
    template <typename R, typename T>
    primitive_argument_type nonzero_where::where_elements1d(
        ir::node_data<T>&& op, primitive_argument_type&& lhs,
        primitive_argument_type&& rhs) const
    {
        auto sizes = extract_largest_dimensions(name_, codename_, op, lhs, rhs);
        switch (extract_largest_dimension(name_, codename_, lhs, rhs))
        {
        case 0:
            {
                auto rhs_val = extract_value_scalar<R>(
                    std::move(rhs), name_, codename_);

                return primitive_argument_type{
                    extract_value_vector<R>(std::move(lhs),
                        [&](T val, std::size_t index)
                        {
                            return op[index] ? val : rhs_val.scalar();
                        },
                        sizes[1], name_, codename_)};
            }

        case 1:
            {
                auto rhs_val = extract_value_vector<R>(
                    std::move(rhs), sizes[1], name_, codename_);
                auto rhs_vector = rhs_val.vector();

                return primitive_argument_type{
                    extract_value_vector<R>(std::move(lhs),
                        [&](T val, std::size_t index)
                        {
                            return op[index] ? val : rhs_vector[index];
                        },
                        sizes[1], name_, codename_)};
            }

        case 2:
            {
                auto rhs_val = extract_value_matrix<R>(
                    std::move(rhs), sizes[0], sizes[1], name_, codename_);
                auto rhs_matrix = rhs_val.matrix();

                return primitive_argument_type{
                    extract_value_matrix<R>(std::move(lhs),
                        [&](T val, std::size_t row, std::size_t column)
                        {
                            return op[column] ?
                                val : rhs_matrix(row, column);
                        },
                        sizes[0], sizes[1], name_, codename_)};
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "nonzero_where::where_elements",
            util::generate_error_message(
                "operands have unsupported number of dimensions",
                name_, codename_));
    }

    // where condition is two dimensional
    template <typename R, typename T>
    primitive_argument_type nonzero_where::where_elements2d(
        ir::node_data<T>&& op, primitive_argument_type&& lhs,
        primitive_argument_type&& rhs) const
    {
        auto sizes = extract_largest_dimensions(name_, codename_, op, lhs, rhs);
        switch (extract_largest_dimension(name_, codename_, lhs, rhs))
        {
        case 0:
            {
                auto rhs_val = extract_value_scalar<R>(
                    std::move(rhs), name_, codename_);

                return primitive_argument_type{
                    extract_value_matrix<R>(std::move(lhs),
                        [&](T val, std::size_t row, std::size_t column)
                        {
                            return op.at(row, column) ? val : rhs_val.scalar();
                        },
                        sizes[0], sizes[1], name_, codename_)};
            }

        case 1:
            {
                auto rhs_val = extract_value_vector<R>(
                    std::move(rhs), sizes[1], name_, codename_);
                auto rhs_vector = rhs_val.vector();

                return primitive_argument_type{
                    extract_value_matrix<R>(std::move(lhs),
                        [&](T val, std::size_t row, std::size_t column)
                        {
                            return op.at(row, column) ? val : rhs_vector[column];
                        },
                        sizes[0], sizes[1], name_, codename_)};
            }

        case 2:
            {
                auto rhs_val = extract_value_matrix<R>(
                    std::move(rhs), sizes[0], sizes[1], name_, codename_);
                auto rhs_matrix = rhs_val.matrix();

                return primitive_argument_type{
                    extract_value_matrix<R>(std::move(lhs),
                        [&](T val, std::size_t row, std::size_t column)
                        {
                            return op.at(row, column) ?
                                val : rhs_matrix(row, column);
                        },
                        sizes[0], sizes[1], name_, codename_)};
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "nonzero_where::where_elements",
            util::generate_error_message(
                "operands have unsupported number of dimensions",
                name_, codename_));
    }

    template <typename R, typename T>
    primitive_argument_type nonzero_where::where_elements(ir::node_data<T>&& op,
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        using storage2d_type = typename ir::node_data<R>::storage2d_type;

        std::size_t dims = op.num_dimensions();
        switch (dims)
        {
        case 0:
            return where_elements0d<R>(
                std::move(op), std::move(lhs), std::move(rhs));

        case 1:
            return where_elements1d<R>(
                std::move(op), std::move(lhs), std::move(rhs));

        case 2:
            return where_elements2d<R>(
                std::move(op), std::move(lhs), std::move(rhs));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "nonzero_where::where_elements",
            util::generate_error_message(
                "operands have unsupported number of dimensions",
                name_, codename_));
    }

    struct nonzero_where::visit_where
    {
        template <typename T>
        primitive_argument_type operator()(T&& op) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "nonzero_where::visit_where::operator()",
                util::generate_error_message(
                    "operand has unsupported type",
                    that_.name_, that_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(ir::node_data<T>&& op) const
        {
            switch (extract_common_type(lhs_, rhs_))
            {
            case node_data_type_bool:
                return that_.where_elements<std::uint8_t>(
                    std::move(op), std::move(lhs_), std::move(rhs_));

            case node_data_type_int64:
                return that_.where_elements<std::int64_t>(
                    std::move(op), std::move(lhs_), std::move(rhs_));

            case node_data_type_double:
                return that_.where_elements<double>(
                    std::move(op), std::move(lhs_), std::move(rhs_));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "nonzero_where::visit_where::operator()",
                util::generate_error_message(
                    "operand has unsupported type",
                    that_.name_, that_.codename_));
        }

        nonzero_where const& that_;
        primitive_argument_type&& lhs_;
        primitive_argument_type&& rhs_;
    };

    hpx::future<primitive_argument_type> nonzero_where::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (nonzero_ && operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "nonzero_where::eval",
                util::generate_error_message(
                    "the nonzero primitive requires exactly one operand",
                    name_, codename_));
        }
        else if (operands.size() != 1 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "nonzero_where::eval",
                util::generate_error_message(
                    "the where primitive requires exactly one or three "
                    "operands",
                    name_, codename_));
        }

        if (nonzero_ || (where_ && operands.size() == 1))
        {
            if (!valid(operands[0]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "nonzero_where::eval",
                    util::generate_error_message(
                        "the nonzero primitive requires "
                        "that the argument given by the operands "
                        "array is valid",
                        name_, codename_));
            }
        }
        else if (!valid(operands[0]) || !valid(operands[1]) ||
            !valid(operands[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "nonzero_where::eval",
                util::generate_error_message(
                    "the where primitive requires "
                    "that the arguments given by the operands "
                    "array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (nonzero_ || (where_ && operands.size() == 1))
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_argument_type&& op)
                -> primitive_argument_type
                {
                    return util::visit(visit_nonzero{*this_},
                        std::move(op.variant()));
                }),
                value_operand(operands[0], args, name_, codename_));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& cond,
                primitive_argument_type&& lhs, primitive_argument_type&& rhs)
            -> primitive_argument_type
            {
                return util::visit(
                    visit_where{*this_, std::move(lhs), std::move(rhs)},
                    std::move(cond.variant()));
            }),
            value_operand(operands[0], args, name_, codename_),
            value_operand(operands[1], args, name_, codename_),
            value_operand(operands[2], args, name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> nonzero_where::eval(
        primitive_arguments_type const& args, eval_context) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}

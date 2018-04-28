// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/generic_operation.hpp>

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
#define PHYLANX_GEN_MATCH_DATA(name)                                           \
    hpx::util::make_tuple(name, std::vector<std::string>{name "(_1)"},         \
        &create_generic_operation, &create_primitive<generic_operation>)
/**/

    std::vector<match_pattern_type> const generic_operation::match_data =
    {
        PHYLANX_GEN_MATCH_DATA("amin"),
        PHYLANX_GEN_MATCH_DATA("amax"),
        PHYLANX_GEN_MATCH_DATA("absolute"),
        PHYLANX_GEN_MATCH_DATA("floor"),
        PHYLANX_GEN_MATCH_DATA("ceil"),
        PHYLANX_GEN_MATCH_DATA("trunc"),
        PHYLANX_GEN_MATCH_DATA("rint"),
        PHYLANX_GEN_MATCH_DATA("conj"),
        PHYLANX_GEN_MATCH_DATA("real"),
        PHYLANX_GEN_MATCH_DATA("imag"),
        PHYLANX_GEN_MATCH_DATA("sqrt"),
        PHYLANX_GEN_MATCH_DATA("invsqrt"),
        PHYLANX_GEN_MATCH_DATA("cbrt"),
        PHYLANX_GEN_MATCH_DATA("invcbrt"),
        PHYLANX_GEN_MATCH_DATA("exp"),
        PHYLANX_GEN_MATCH_DATA("exp2"),
        PHYLANX_GEN_MATCH_DATA("exp10"),
        PHYLANX_GEN_MATCH_DATA("log"),
        PHYLANX_GEN_MATCH_DATA("log2"),
        PHYLANX_GEN_MATCH_DATA("log10"),
        PHYLANX_GEN_MATCH_DATA("sin"),
        PHYLANX_GEN_MATCH_DATA("cos"),
        PHYLANX_GEN_MATCH_DATA("tan"),
        PHYLANX_GEN_MATCH_DATA("arcsin"),
        PHYLANX_GEN_MATCH_DATA("arccos"),
        PHYLANX_GEN_MATCH_DATA("arctan"),
        PHYLANX_GEN_MATCH_DATA("arcsinh"),
        PHYLANX_GEN_MATCH_DATA("arccosh"),
        PHYLANX_GEN_MATCH_DATA("arctanh"),
        PHYLANX_GEN_MATCH_DATA("erf"),
        PHYLANX_GEN_MATCH_DATA("erfc"),
    };

#undef PHYLANX_GEN_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    generic_operation::scalar_function_ptr
        generic_operation::get_0d_map(std::string const& name) const
    {
        static std::map<std::string, scalar_function_ptr> map0d = {
            {"amin", [](double m) -> double { return m; }},
            {"amax", [](double m) -> double { return m; }},
            {"absolute", [](double m) -> double { return blaze::abs(m); }},
            {"floor", [](double m) -> double { return blaze::floor(m); }},
            {"ceil", [](double m) -> double { return blaze::ceil(m); }},
            {"trunc", [](double m) -> double { return blaze::trunc(m); }},
            {"rint", [](double m) -> double { return blaze::round(m); }},
            {"conj", [](double m) -> double { return blaze::conj(m); }},
            {"real", [](double m) -> double { return blaze::real(m); }},
            {"imag", [](double m) -> double { return blaze::imag(m); }},
            {"sqrt", [](double m) -> double { return blaze::sqrt(m); }},
            {"invsqrt", [](double m) -> double { return blaze::invsqrt(m); }},
            {"cbrt", [](double m) -> double { return blaze::cbrt(m); }},
            {"invcbrt", [](double m) -> double { return blaze::invcbrt(m); }},
            {"exp", [](double m) -> double { return blaze::exp(m); }},
            {"exp2", [](double m) -> double { return blaze::exp2(m); }},
            {"exp10", [](double m) -> double { return blaze::pow(10, m); }},
            {"log", [](double m) -> double { return blaze::log(m); }},
            {"log2", [](double m) -> double { return blaze::log2(m); }},
            {"log10", [](double m) -> double { return blaze::log10(m); }},
            {"sin", [](double m) -> double { return blaze::sin(m); }},
            {"cos", [](double m) -> double { return blaze::cos(m); }},
            {"tan", [](double m) -> double { return blaze::tan(m); }},
            {"arcsin", [](double m) -> double { return blaze::asin(m); }},
            {"arccos", [](double m) -> double { return blaze::acos(m); }},
            {"arctan", [](double m) -> double { return blaze::atan(m); }},
            {"arcsinh", [](double m) -> double { return blaze::asinh(m); }},
            {"arccosh",
                [](double m) -> double
                {
#if defined(PHYLANX_DEBUG)
                    if (m < 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter, "arccosh",
                            "scalar arccosh: domain error");
                    }
#endif
                    return blaze::acosh(m);
                }},
            {"arctanh",
                [](double m) -> double
                {
#if defined(PHYLANX_DEBUG)
                    if (m <= -1 || m >= 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter, "arctanh",
                            "scalar arctanh: domain error");
                    }
#endif
                    return blaze::atanh(m);
                }},
            {"erf", [](double m) -> double { return blaze::erf(m); }},
            {"erfc", [](double m) -> double { return blaze::erfc(m); }},
            {"normalize",
                [](double m) -> double
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "normalize",
                        "normalize does not support scalars");
                }},
            {"trace", [](double m) -> double { return m; }}};
        return map0d[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    generic_operation::vector_function_ptr
        generic_operation::get_1d_map(std::string const& name) const
    {
        static std::map<std::string, vector_function_ptr> map1d =
        {
            { "amin",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return dynamic_vector_type(1, (blaze::min)(m));
                }
            },
            { "amax",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return dynamic_vector_type(1, (blaze::max)(m));
                }
            },
            { "absolute",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::abs(m);
                }
            },
            { "floor",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::floor(m);
                }
            },
            { "ceil",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::ceil(m);
                }
            },
            { "trunc",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::trunc(m);
                }
            },
            { "rint",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::round(m);
                }
            },
            { "conj",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::conj(m);
                }
            },
            { "real",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::real(m);
                }
            },
            { "imag",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::imag(m);
                }
            },
            { "sqrt",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::sqrt(m);
                }
            },
            { "invsqrt",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::invsqrt(m);
                }
            },
            { "cbrt",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::cbrt(m);
                }
            },
            { "invcbrt",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::invcbrt(m);
                }
            },
            { "exp",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::exp(m);
                }
            },
            { "exp2",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::exp2(m);
                }
            },
            { "exp10",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::exp10(m);
                }
            },
            { "log",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::log(m);
                }
            },
            { "log2",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::log2(m);
                }
            },
            { "log10",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::log10(m);
                }
            },
            { "sin",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::sin(m);
                }
            },
            { "cos",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::cos(m);
                }
            },
            {"tan",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::tan(m);
                }
            },
            { "arcsin",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::asin(m);
                }
            },
            { "arccos",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::acos(m);
                }
            },
            { "arctan",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::atan(m);
                }
            },
            { "arcsinh",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::asinh(m);
                }
            },
            { "arccosh",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
#if defined(PHYLANX_DEBUG)
                    for (auto a : m)
                    {
                        if (a < 1)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter, "arccosh",
                                "vector arccosh: domain error");
                        }
                    }
#endif
                    return blaze::acosh(m);
                }
            },
            { "arctanh",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
#if defined(PHYLANX_DEBUG)
                    for (auto a : m)
                    {
                        if (a >= 1 || a <= -1)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter, "arctanh",
                                "vector arctanh: domain error");
                        }
                    }
#endif
                    return blaze::atanh(m);
                }
            },
            { "erf",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::erf(m);
                }
            },
            { "erfc",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::erfc(m);
                }
            },
            { "normalize",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    return blaze::normalize(m);
                }
            },
            { "trace",
                [](custom_vector_type const& m) -> dynamic_vector_type
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "trace",
                        "vector operation is not supported for trace()");
                }
            }
        };
        return map1d[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    generic_operation::matrix_function_ptr
        generic_operation::get_2d_map(std::string const& name) const
    {
        static std::map<std::string, matrix_function_ptr> map2d =
        {
            { "amin",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return dynamic_matrix_type{1, 1, (blaze::min)(m)};
                }
            },
            { "amax",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return dynamic_matrix_type{1, 1, (blaze::max)(m)};
                }
            },
            { "absolute",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::abs(m);
                }
            },
            { "floor",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::floor(m);
                }
            },
            { "ceil",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::ceil(m);
                }
            },
            { "trunc",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::trunc(m);
                }
            },
            { "rint",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::round(m);
                }
            },
            { "conj",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::conj(m);
                }
            },
            { "real",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::real(m);
                }
            },
            { "imag",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::imag(m);
                }
            },
            { "sqrt",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::sqrt(m);
                }
            },
            { "invsqrt",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::invsqrt(m);
                }
            },
            { "cbrt",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::cbrt(m);
                }
            },
            { "invcbrt",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::invcbrt(m);
                }
            },
            { "exp",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::exp(m);
                }
            },
            { "exp2",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::exp2(m);
                }
            },
            { "exp10",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::exp10(m);
                }
            },
            { "log",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::log(m);
                }
            },
            { "log2",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::log2(m);
                }
            },
            { "log10",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::log10(m);
                }
            },
            { "sin",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::sin(m);
                }
            },
            { "cos",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::cos(m);
                }
            },
            { "tan",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::tan(m);
                }
            },
            { "arcsin",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::asin(m);
                }
            },
            { "arccos",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::acos(m);
                }
            },
            { "arctan",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::atan(m);
                }
            },
            { "arcsinh",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::asinh(m);
                }
            },
            { "arccosh",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
#if defined(PHYLANX_DEBUG)
                    for (size_t i = 0UL; i < m.rows(); ++i)
                    {
                        for (size_t j = 0UL; j < m.columns(); ++j)
                        {
                            if (m(i, j) < 1)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "arccosh", "matrix arccosh: domain error");
                            }
                        }
                    }
#endif
                    return blaze::acosh(m);
                }
            },
            { "arctanh",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
#if defined(PHYLANX_DEBUG)
                    for (size_t i = 0UL; i < m.rows(); ++i)
                    {
                        for (size_t j = 0UL; j < m.columns(); ++j)
                        {
                            if (m(i, j) <= -1 || m(i, j) >= 1)
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "arctanh", "matrix arctanh: domain error");
                        }
                    }
#endif
                    return blaze::atanh(m);
                }
            },
            { "erf",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::erf(m);
                }
            },
            { "erfc",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::erfc(m);
                }
            },
            { "normalize",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "normalize",
                        "normalize() is not a supported matrix operation");
                }
            },
            { "trace",
                [](custom_matrix_type const& m) -> dynamic_matrix_type
                {
                    return blaze::DynamicMatrix<double>(
                        1, 1, blaze::trace(m));
                }
            }
        };
        return map2d[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::string extract_function_name(std::string const& name)
        {
            std::string::size_type p = name.find_first_of("$");
            if (p != std::string::npos)
            {
                return name.substr(0, p);
            }
            return name;
        }
    }

    generic_operation::generic_operation(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
        std::string func_name = detail::extract_function_name(name);

        func0d_ = get_0d_map(func_name);
        func1d_ = get_1d_map(func_name);
        func2d_ = get_2d_map(func_name);

        HPX_ASSERT(
            func0d_ != nullptr && func1d_ != nullptr && func2d_ != nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generic_operation::generic0d(
        operand_type&& op) const
    {
        return primitive_argument_type{func0d_(op.scalar())};
    }

    primitive_argument_type generic_operation::generic1d(
        operand_type&& op) const
    {
        return primitive_argument_type{
            ir::node_data<double>{func1d_(op.vector())}};
    }

    primitive_argument_type generic_operation::generic2d(
        operand_type&& op) const
    {
        return primitive_argument_type{
            ir::node_data<double>{func2d_(op.matrix())}};
    }

    hpx::future<primitive_argument_type> generic_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::eval",
                generate_error_message(
                    "the generic_operation primitive requires"
                        "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::eval",
                generate_error_message(
                    "the generic_operation primitive requires "
                        "that the arguments given by the operands "
                        "array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](operand_type&& op) -> primitive_argument_type
            {
                std::size_t dims = op.num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->generic0d(std::move(op));

                case 1:
                    return this_->generic1d(std::move(op));

                case 2:
                    return this_->generic2d(std::move(op));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "generic_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            numeric_operand(operands[0], args, name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> generic_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

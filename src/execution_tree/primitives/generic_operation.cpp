// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/generic_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

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
    ///////////////////////////////////////////////////////////////////////////
    primitive create_generic_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__gen");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

#define PHYLANX_GEN_MATCH_DATA(name)                                           \
    hpx::util::make_tuple(name, std::vector<std::string>{name "(_1)"},         \
        &create_generic_operation, &create_primitive<generic_operation>) /**/

    std::vector<match_pattern_type> const generic_operation::match_data =
    {
        PHYLANX_GEN_MATCH_DATA("sin"),
        PHYLANX_GEN_MATCH_DATA("cos"),
        PHYLANX_GEN_MATCH_DATA("tan"),
        PHYLANX_GEN_MATCH_DATA("sinh"),
        PHYLANX_GEN_MATCH_DATA("cosh"),
        PHYLANX_GEN_MATCH_DATA("tanh"),
        PHYLANX_GEN_MATCH_DATA("floor"),
        PHYLANX_GEN_MATCH_DATA("ceil"),
        PHYLANX_GEN_MATCH_DATA("trunc"),
//         PHYLANX_GEN_MATCH_DATA("exp"),
        PHYLANX_GEN_MATCH_DATA("exp2"),
        PHYLANX_GEN_MATCH_DATA("log"),
        PHYLANX_GEN_MATCH_DATA("log10"),
        PHYLANX_GEN_MATCH_DATA("log2"),
        PHYLANX_GEN_MATCH_DATA("sqrt"),
        PHYLANX_GEN_MATCH_DATA("cbrt")
    };

#undef PHYLANX_GEN_MATCH_DATA

    double (*generic_operation::get_0d_map(std::string const& name))(double)
    {
        static std::map<std::string, double (*)(double)> map0d = {
            {"sin", [](double m) -> double { return blaze::sin(m); }},
            {"cos", [](double m) -> double { return blaze::cos(m); }},
            {"tan", [](double m) -> double { return blaze::tan(m); }},
            {"sinh", [](double m) -> double { return blaze::sinh(m); }},
            {"cosh", [](double m) -> double { return blaze::cosh(m); }},
            {"tanh", [](double m) -> double { return blaze::tanh(m); }},
            {"floor", [](double m) -> double { return blaze::floor(m); }},
            {"ceil", [](double m) -> double { return blaze::ceil(m); }},
            {"trunc", [](double m) -> double { return blaze::trunc(m); }},
            {"exp", [](double m) -> double { return blaze::exp(m); }},
            {"exp2", [](double m) -> double { return blaze::exp2(m); }},
            {"log", [](double m) -> double { return blaze::log(m); }},
            {"log10", [](double m) -> double { return blaze::log10(m); }},
            {"log2", [](double m) -> double { return blaze::log2(m); }},
            {"sqrt", [](double m) -> double { return blaze::sqrt(m); }},
            {"cbrt", [](double m) -> double { return blaze::cbrt(m); }}};
        return map0d[name];
    }

    blaze::DynamicVector<double> (
        *generic_operation::get_1d_map(std::string const& name))(
        const blaze::CustomVector<double, blaze::aligned, blaze::padded>&)
    {
        static std::map<std::string,
            blaze::DynamicVector<double> (*)(const blaze::CustomVector<double,
                blaze::aligned, blaze::padded>&)>
            map1d = {
                {"sin",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::sin(m);
                    }},
                {"cos",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::cos(m);
                    }},
                {"tan",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::tan(m);
                    }},
                {"sinh",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::sinh(m);
                    }},
                {"cosh",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::cosh(m);
                    }},
                {"tanh",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::tanh(m);
                    }},
                {"floor",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::floor(m);
                    }},
                {"ceil",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::ceil(m);
                    }},
                {"trunc",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::trunc(m);
                    }},
                {"exp",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::exp(m);
                    }},
                {"exp2",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::exp2(m);
                    }},
                {"log",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::log(m);
                    }},
                {"log10",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::log10(m);
                    }},
                {"log2",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::log2(m);
                    }},
                {"sqrt",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::sqrt(m);
                    }},
                {"cbrt",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::cbrt(m);
                    }}};
        return map1d[name];
    }

    blaze::DynamicMatrix<double> (
        *generic_operation::get_2d_map(std::string const& name))(
        const blaze::CustomMatrix<double, blaze::aligned, blaze::padded>&)
    {
        static std::map<std::string,
            blaze::DynamicMatrix<double> (*)(const blaze::CustomMatrix<double,
                blaze::aligned, blaze::padded>&)>
            map2d = {
                {"sin",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::sin(m);
                    }},
                {"cos",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::cos(m);
                    }},
                {"tan",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::tan(m);
                    }},
                {"sinh",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::sinh(m);
                    }},
                {"cosh",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::cosh(m);
                    }},
                {"tanh",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::tanh(m);
                    }},
                {"floor",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::floor(m);
                    }},
                {"ceil",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::ceil(m);
                    }},
                {"trunc",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::trunc(m);
                    }},
                {"exp",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::exp(m);
                    }},
                {"exp2",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::exp2(m);
                    }},
                {"log",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::log(m);
                    }},
                {"log10",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::log10(m);
                    }},
                {"log2",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::log2(m);
                    }},
                {"sqrt",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::sqrt(m);
                    }},
                {"cbrt",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::cbrt(m);
                    }}};
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
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generic_operation::generic0d(operands_type && ops)
        const
    {
        ops[0] = func0d_(ops[0].scalar());
        return primitive_argument_type{std::move(ops[0])};
    }

    primitive_argument_type generic_operation::generic1d(operands_type && ops)
        const
    {
        using vector_type = blaze::DynamicVector<double>;

        vector_type result = func1d_(ops[0].vector());
        return primitive_argument_type{
            ir::node_data<double>(std::move(result))};
    }

    primitive_argument_type generic_operation::generic2d(operands_type && ops)
        const
    {
        using matrix_type = blaze::DynamicMatrix<double>;

        matrix_type result = func2d_(ops[0].matrix());
        return primitive_argument_type{
            ir::node_data<double>(std::move(result))};
    }

    hpx::future<primitive_argument_type> generic_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::eval",
                execution_tree::generate_error_message(
                    "the generic_operation primitive requires"
                    "exactly one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::eval",
                execution_tree::generate_error_message(
                    "the generic_operation primitive requires "
                    "that the arguments given by the operands "
                    "array is valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(
            hpx::util::unwrapping(
                [this_](operands_type&& ops) -> primitive_argument_type {
                    std::size_t dims = ops[0].num_dimensions();
                    switch (dims)
                    {
                    case 0:
                        return this_->generic0d(std::move(ops));

                    case 1:
                        return this_->generic1d(std::move(ops));

                    case 2:
                        return this_->generic2d(std::move(ops));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "generic_operation::eval",
                            execution_tree::generate_error_message(
                                "left hand side operand has unsupported "
                                "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }),
            detail::map_operands(operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    // Implement 'gen' for all possible combinations of lhs and rhs
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

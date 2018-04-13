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
        static std::string type("gen");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const generic_operation::match_data = {
        hpx::util::make_tuple("gen",
            std::vector<std::string>{"gen(_1)"},
            &create_generic_operation,
            &create_primitive<generic_operation>)};

    double (*generic_operation::get_0d_map(std::string const& name))(double)
        const
    {
        static std::map<std::string, double (*)(double)> map0d = {
            {"exp", [](double m) -> double { return blaze::exp(m); }},
            {"log", [](double m) -> double { return blaze::log(m); }},
            {"sin", [](double m) -> double { return blaze::sin(m); }},
            {"sinh", [](double m) -> double { return blaze::sinh(m); }}};
        return map0d[name];
    }

    blaze::DynamicVector<double> (
        *generic_operation::get_1d_map(std::string const& name))(
        const blaze::CustomVector<double, blaze::aligned, blaze::padded>&) const
    {
        static std::map<std::string,
            blaze::DynamicVector<double> (*)(const blaze::CustomVector<double,
                blaze::aligned, blaze::padded>&)>
            map1d = {
                {"exp",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::exp(m);
                    }},
                {"log",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::log(m);
                    }},
                {"sin",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::sin(m);
                    }},
                {"sinh",
                    [](const blaze::CustomVector<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicVector<double> {
                        return blaze::sinh(m);
                    }}};
        return map1d[name];
    }

    blaze::DynamicMatrix<double> (
        *generic_operation::get_2d_map(std::string const& name))(
        const blaze::CustomMatrix<double, blaze::aligned, blaze::padded>&) const
    {
        static std::map<std::string,
            blaze::DynamicMatrix<double> (*)(const blaze::CustomMatrix<double,
                blaze::aligned, blaze::padded>&)>
            map2d = {
                {"exp",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::exp(m);
                    }},
                {"log",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::log(m);
                    }},
                {"sin",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::sin(m);
                    }},
                {"sinh",
                    [](const blaze::CustomMatrix<double, blaze::aligned,
                        blaze::padded>& m) -> blaze::DynamicMatrix<double> {
                        return blaze::sinh(m);
                    }}};
        return map2d[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    generic_operation::generic_operation(
        std::vector<primitive_argument_type> && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
        func0d_ = get_0d_map(name);
        func1d_ = get_1d_map(name);
        func2d_ = get_2d_map(name);
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

//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <bindings/binding_helpers.hpp>
#include <bindings/type_casters.hpp>

#include <pybind11/pybind11.h>

#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// expose execution tree
void phylanx::bindings::bind_execution_tree(pybind11::module m)
{
    auto execution_tree = m.def_submodule("execution_tree");

    execution_tree.def("generate_tree", &phylanx::bindings::generate_tree,
        "generate expression tree from given expression");

    execution_tree.def("var",
        [](double d) {
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{d});
                });
        },
        "create a new variable from a floating point value");
    execution_tree.def("var",
        [](const std::string& d) {
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{d});
                });
        },
        "create a new variable from a string");
    execution_tree.def("var",
        [](const std::vector<double>& d) {
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{
                            phylanx::ir::node_data<double>{d}});
                });
        },
        "create a new variable from a vector floating point values");
    execution_tree.def("var",
        [](const std::vector<std::vector<double>>& d) {
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{
                            phylanx::ir::node_data<double>{d}});
                });
        },
        "create a new variable from a matrix floating point values");

    execution_tree.def("eval", phylanx::bindings::expression_compiler,
        "compile and evaluate a numerical expression in PhySL");

    pybind11::class_<phylanx::execution_tree::primitive>(execution_tree,
        "primitive", "type representing an arbitrary execution tree")
        .def(pybind11::init<>())
        .def("eval", [](phylanx::execution_tree::primitive const& p)
            {
                return hpx::threads::run_as_hpx_thread(
                    [&]() {
                        using namespace phylanx::execution_tree;
                        return numeric_operand(primitive_argument_type{p}, {})
                            .get();
                    });
            },
            "evaluate execution tree")
        .def("assign", [](phylanx::execution_tree::primitive p, double d)
            {
                hpx::threads::run_as_hpx_thread(
                    [&]() {
                        using namespace phylanx::execution_tree;
                        p.store(hpx::launch::sync, primitive_argument_type{d});
                    });
            },
            "assign another value to variable")
        .def("__str__",
            &phylanx::bindings::as_string<phylanx::execution_tree::primitive>)
        .def("__repr__",
            &phylanx::bindings::repr<phylanx::execution_tree::primitive>)
    ;
}

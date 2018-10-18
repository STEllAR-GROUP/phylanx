//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <bindings/binding_helpers.hpp>
#include <bindings/type_casters.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// expose execution tree
void phylanx::bindings::bind_execution_tree(pybind11::module m)
{
    auto execution_tree = m.def_submodule("execution_tree");

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

    execution_tree.def("compile", phylanx::bindings::expression_compiler,
        "compile a numerical expression in PhySL");

    execution_tree.def("eval", phylanx::bindings::expression_evaluator,
        "compile and evaluate a numerical expression in PhySL");

    execution_tree.def("eval",
        [](std::string const& xexpr, compiler_state& c, pybind11::args args)
        ->  phylanx::execution_tree::primitive_argument_type
        {
            return phylanx::bindings::expression_evaluator(
                "<unknown>", xexpr, c, args);
        },
        "compile and evaluate a numerical expression in PhySL");

    // expose functionalities needed for accessing performance data
    execution_tree.def("enable_measurements",
        phylanx::bindings::enable_measurements,
        "enable all performance counters for the current execution tree");

    execution_tree.def("retrieve_counter_data",
        phylanx::bindings::retrieve_counter_data,
        "retrieve performance data from all active performance counters");

    execution_tree.def("retrieve_dot_tree_topology",
        phylanx::bindings::retrieve_dot_tree_topology,
        "retrieve the DOT tree topology for the given execution tree");

    execution_tree.def("retrieve_newick_tree_topology",
        phylanx::bindings::retrieve_newick_tree_topology,
        "retrieve the Newick tree topology for the given execution tree");

    execution_tree.def("retrieve_tree_topology",
        phylanx::bindings::retrieve_tree_topology,
        "retrieve the Newick and DOT tree topologies for the given "
        "execution tree");

    // phylanx.execution_tree.primitive
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
                        p.store(hpx::launch::sync,
                            primitive_argument_type{d}, {});
                    });
            },
            "assign another value to variable")
        .def("__str__",
            &phylanx::bindings::as_string<phylanx::execution_tree::primitive>)
        .def("__repr__",
            &phylanx::bindings::repr<phylanx::execution_tree::primitive>)
    ;
}

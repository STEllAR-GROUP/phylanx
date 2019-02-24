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

#include <cstdint>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// expose execution tree
template <typename T>
void bind_variable(pybind11::module &execution_tree)
{
    execution_tree.def("var",
        [](T d) {
            pybind11::gil_scoped_release release;       // release GIL
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
        [](std::vector<T> const& d) {
            pybind11::gil_scoped_release release;       // release GIL
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{
                            phylanx::ir::node_data<T>{d}});
                });
        },
        "create a new variable from a vector floating point values");
    execution_tree.def("var",
        [](std::vector<std::vector<T>> const& d) {
            pybind11::gil_scoped_release release;       // release GIL
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{
                            phylanx::ir::node_data<T>{d}});
                });
        },
        "create a new variable from a matrix floating point values");
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    execution_tree.def("var",
        [](std::vector<std::vector<std::vector<T>>> const& d) {
            pybind11::gil_scoped_release release;       // release GIL
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{
                            phylanx::ir::node_data<T>{d}});
                });
        },
        "create a new variable from a tensor floating point values");
#endif
}

void phylanx::bindings::bind_execution_tree(pybind11::module m)
{
    auto execution_tree = m.def_submodule("execution_tree");

    execution_tree.def("var",
        [](std::string const& d) {
            pybind11::gil_scoped_release release;       // release GIL
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{d});
                });
        },
        "create a new variable from a string");

    bind_variable<double>(execution_tree);
    bind_variable<std::int64_t>(execution_tree);
    bind_variable<std::uint8_t>(execution_tree);

    execution_tree.def("var",
        [](phylanx::execution_tree::primitive_argument_type const& arg) {
            pybind11::gil_scoped_release release;       // release GIL
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", arg);
                });
        },
        "create a new variable from a primitive_argument_type");

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
                pybind11::gil_scoped_release release;       // release GIL
                return hpx::threads::run_as_hpx_thread(
                    [&]() {
                        using namespace phylanx::execution_tree;
                        return value_operand(primitive_argument_type{p},
                            primitive_argument_type{}).get();
                    });
            },
            "evaluate execution tree")
        .def("assign", [](phylanx::execution_tree::primitive p, double d)
            {
                pybind11::gil_scoped_release release;       // release GIL
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

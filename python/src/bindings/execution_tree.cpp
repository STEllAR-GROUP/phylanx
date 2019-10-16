//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <bindings/binding_helpers.hpp>
#include <bindings/type_casters.hpp>
#include <bindings/variable.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <hpx/errors/exception.hpp>
#include <hpx/runtime/threads/run_as_hpx_thread.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// expose execution tree
void phylanx::bindings::bind_execution_tree(pybind11::module m)
{
    auto execution_tree = m.def_submodule("execution_tree");

    // Compiler State
    pybind11::class_<phylanx::bindings::compiler_state>(
            execution_tree, "compiler_state")
        .def(pybind11::init<std::string>());

    ///////////////////////////////////////////////////////////////////////////
    execution_tree.def("compile", phylanx::bindings::expression_compiler,
        "compile a numerical expression in PhySL");

    execution_tree.def("eval", phylanx::bindings::expression_evaluator,
        "compile and evaluate a numerical expression in PhySL");

    execution_tree.def(
        "eval",
        [](compiler_state& state, std::string const& xexpr, pybind11::args args,
            pybind11::kwargs kwargs)
            -> phylanx::execution_tree::primitive_argument_type
        {
            return phylanx::bindings::expression_evaluator(
                state, state.codename_, xexpr, args, kwargs);
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

    execution_tree.def("code_for", phylanx::bindings::code_for,
        "extract compiled code for given function");

    execution_tree.def("bound_code_for", phylanx::bindings::bound_code_for,
        "extract compiled code for given function and bind it to given "
        "arguments");

    // phylanx.execution_tree.variable
    auto var =
        pybind11::class_<phylanx::execution_tree::variable>(execution_tree,
            "variable", "type representing an arbitrary execution tree")
            // copy-constructor must go before constructor that takes
            // primitive_argument_type to avoid implicit conversion
            .def(pybind11::init<phylanx::execution_tree::variable const&>())
            .def(pybind11::init<phylanx::execution_tree::primitive,
                     pybind11::object, pybind11::object, pybind11::object>(),
                pybind11::arg("value"),
                pybind11::arg("dtype") = pybind11::none(),
                pybind11::arg("name") = pybind11::none(),
                pybind11::arg("constraint") = pybind11::none())
            .def(pybind11::init<pybind11::array, pybind11::object,
                     pybind11::object, pybind11::object>(),
                pybind11::arg("value"),
                pybind11::arg("dtype") = pybind11::none(),
                pybind11::arg("name") = pybind11::none(),
                pybind11::arg("constraint") = pybind11::none())
            .def(pybind11::init<std::string, pybind11::object, pybind11::object,
                     pybind11::object>(),
                pybind11::arg("value"),
                pybind11::arg("dtype") = pybind11::none(),
                pybind11::arg("name") = pybind11::none(),
                pybind11::arg("constraint") = pybind11::none())
            .def(
                pybind11::init<phylanx::execution_tree::primitive_argument_type,
                    pybind11::object, pybind11::object, pybind11::object>(),
                pybind11::arg("value"),
                pybind11::arg("dtype") = pybind11::none(),
                pybind11::arg("name") = pybind11::none(),
                pybind11::arg("constraint") = pybind11::none())
            .def(
                "eval",
                [](phylanx::execution_tree::variable const& var,
                    pybind11::args args) {
                    pybind11::gil_scoped_release release;       // release GIL
                    return hpx::threads::run_as_hpx_thread(
                            [&]() { return var.eval(std::move(args)); });
                },
                "evaluate execution tree")
            .def(
                "__call__",
                [](phylanx::execution_tree::variable const& var,
                    pybind11::args args) {
                    pybind11::gil_scoped_release release;       // release GIL
                    return hpx::threads::run_as_hpx_thread(
                        [&]() { return var.eval(std::move(args)); });
                },
                "evaluate execution tree")
            .def_property_readonly(
                "dtype",
                [](phylanx::execution_tree::variable const& var) {
                    return var.dtype();
                },
                "return the dtype of the value stored by the variable")
            .def_property_readonly(
                "name",
                [](phylanx::execution_tree::variable const& var) {
                    return var.name();
                },
                "return the name of the variable")
            .def("__str__",
                [](phylanx::execution_tree::variable const& var) {
                    return var.name();
                })
            .def("__repr__",
                [](phylanx::execution_tree::variable const& var) {
                    return bindings::repr<phylanx::execution_tree::primitive>(
                        var.value());
                })
            .def("__add__", &phylanx::execution_tree::add_variables)
            .def("__add__", &phylanx::execution_tree::add_variables_gen)
            .def("__radd__", &phylanx::execution_tree::radd_variables_gen)
            .def("__sub__", &phylanx::execution_tree::sub_variables)
            .def("__sub__", &phylanx::execution_tree::sub_variables_gen)
            .def("__rsub__", &phylanx::execution_tree::rsub_variables_gen)
            .def("__mul__", &phylanx::execution_tree::mul_variables)
            .def("__mul__", &phylanx::execution_tree::mul_variables_gen)
            .def("__rmul__", &phylanx::execution_tree::rmul_variables_gen)
            .def("__div__", &phylanx::execution_tree::div_variables)
            .def("__div__", &phylanx::execution_tree::div_variables_gen)
            .def("__rdiv__", &phylanx::execution_tree::rdiv_variables_gen)
            .def("__neg__", &phylanx::execution_tree::unary_minus_variables)
            .def("__iadd__", &phylanx::execution_tree::iadd_variables)
            .def("__iadd__", &phylanx::execution_tree::iadd_variables_gen)
            .def("update_add", &phylanx::execution_tree::iadd_variables)
            .def("update_add", &phylanx::execution_tree::iadd_variables_gen)
            .def("__isub__", &phylanx::execution_tree::isub_variables)
            .def("__isub__", &phylanx::execution_tree::isub_variables_gen)
            .def("update_sub", &phylanx::execution_tree::isub_variables)
            .def("update_sub", &phylanx::execution_tree::isub_variables_gen)
            .def("update_moving_average",
                &phylanx::execution_tree::moving_average_variables)
            .def("update_moving_average",
                &phylanx::execution_tree::moving_average_variables_gen);

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
        .def_property_readonly("dtype", &phylanx::bindings::extract_dtype,
            "return the dtype of the value stored by the variable")
        .def("__str__",
            &phylanx::bindings::as_string<phylanx::execution_tree::primitive>)
        .def("__repr__",
            &phylanx::bindings::repr<phylanx::execution_tree::primitive>);
}

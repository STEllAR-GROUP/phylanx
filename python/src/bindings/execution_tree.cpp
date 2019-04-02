//  Copyright (c) 2017-2018 Hartmut Kaiser
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

#include <hpx/exception.hpp>
#include <hpx/runtime/threads/run_as_hpx_thread.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// expose execution tree
template <typename T>
void bind_variable(pybind11::class_<phylanx::execution_tree::variable>& var)
{
    var.def(pybind11::init<std::vector<T>, pybind11::object,
                pybind11::object, pybind11::object>(),
           pybind11::arg("value"),
           pybind11::arg("dtype") = pybind11::none(),
           pybind11::arg("name") = pybind11::none(),
           pybind11::arg("constraint") = pybind11::none())
        .def(pybind11::init<std::vector<std::vector<T>>, pybind11::object,
                 pybind11::object, pybind11::object>(),
            pybind11::arg("value"),
            pybind11::arg("dtype") = pybind11::none(),
            pybind11::arg("name") = pybind11::none(),
            pybind11::arg("constraint") = pybind11::none())
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        .def(pybind11::init<std::vector<std::vector<std::vector<T>>>,
                 pybind11::object, pybind11::object, pybind11::object>(),
            pybind11::arg("value"),
            pybind11::arg("dtype") = pybind11::none(),
            pybind11::arg("name") = pybind11::none(),
            pybind11::arg("constraint") = pybind11::none())
#endif
        ;
}

void phylanx::bindings::bind_execution_tree(pybind11::module m)
{
    auto execution_tree = m.def_submodule("execution_tree");

    // Compiler State
    pybind11::class_<phylanx::bindings::compiler_state>(
            execution_tree, "compiler_state")
        .def(pybind11::init<>())
        .def("code_for",
            [](phylanx::bindings::compiler_state const& state,
                    std::string const& func_name)
            -> phylanx::execution_tree::primitive
            {
                pybind11::gil_scoped_release release;       // release GIL
                return hpx::threads::run_as_hpx_thread([&]()
                {
                    // locate requested function entry point
                    for (auto const& entry_point :
                        state.eval_snippets.program_.entry_points())
                    {
                        if (func_name == entry_point.func_name_)
                        {
                            auto && funcs = entry_point.functions();
                            if (funcs.empty())
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "phylanx::bindings::bind_execution_tree",
                                    hpx::util::format("cannot locate requested "
                                        "function entry point '{}'", func_name));
                            }

                            return phylanx::execution_tree::primitive_operand(
                                funcs.back().arg_);
                        }
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::bindings::bind_execution_tree",
                        hpx::util::format("cannot locate requested "
                            "function entry point '{}'", func_name));
                    return phylanx::execution_tree::primitive();
                });
            });

    ///////////////////////////////////////////////////////////////////////////
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
                    pybind11::args args)
                {
                    pybind11::gil_scoped_release release;       // release GIL
                    return hpx::threads::run_as_hpx_thread(
                            [&]() { return var.eval(std::move(args)); });
                },
                "evaluate execution tree")
            .def(
                "__call__",
                [](phylanx::execution_tree::variable const& var,
                    pybind11::args args)
                {
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
                });

    //     bind_variable<double>(var);
    //     bind_variable<std::int64_t>(var);
    //     bind_variable<std::uint8_t>(var);

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

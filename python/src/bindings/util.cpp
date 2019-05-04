//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/include/ast.hpp>
#include <phylanx/include/ir.hpp>
#include <phylanx/include/util.hpp>

#include <bindings/binding_helpers.hpp>
#include <bindings/type_casters.hpp>

#include <pybind11/pybind11.h>

#include <hpx/include/iostreams.hpp>

#include <cstdint>
#include <vector>
#include <map>
#include <sstream>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// expose util submodule
void phylanx::bindings::bind_util(pybind11::module m)
{
    auto util = m.def_submodule("util");

    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::nil>,
        "serialize an AST optoken object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::optoken>,
        "serialize an AST optoken object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::identifier>,
        "serialize an AST identifier object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::primary_expr>,
        "serialize an AST primary_expr object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::operand>,
        "serialize an AST operand object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::unary_expr>,
        "serialize an AST unary_expr object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::operation>,
        "serialize an AST operation object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::expression>,
        "serialize an AST expression object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::function_call>,
        "serialize an AST function_call object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::literal_value_type>,
        "serialize an AST literal_value_type object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<std::vector<phylanx::ast::expression>>,
        "serialize a vector of AST expressions into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ir::node_data<double>>,
        "serialize a node_data<double> expression object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ir::node_data<std::int64_t>>,
        "serialize a node_data<std::int64_t> expression object into a "
        "byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ir::node_data<std::uint8_t>>,
        "serialize a node_data<std::uint8_t> expression object into a "
        "byte-stream");

    util.def(
        "unserialize",
        [](std::vector<char> const& input)
                -> std::vector<phylanx::ast::expression> {
            pybind11::gil_scoped_release release;    // release GIL
            return phylanx::util::unserialize<
                std::vector<phylanx::ast::expression>>(input);
        },
        "un-serialize a byte-stream into a Phylanx object");
    util.def(
        "unserialize_expr",
        [](std::vector<char> const& input) -> phylanx::ast::expression {
            pybind11::gil_scoped_release release;    // release GIL
            return phylanx::util::unserialize<phylanx::ast::expression>(input);
        },
        "un-serialize a byte-stream into a Phylanx object");

    util.def(
        "phyhelpex",
        [](std::string const& s) -> std::string {
            pybind11::gil_scoped_release release;    // release GIL
            return phylanx::execution_tree::find_help(s);
        },
        "display help strings for Phylanx primitives and plugins.");
    util.def(
        "phylist",
        []() -> std::map<std::string, std::vector<std::string>> {
            pybind11::gil_scoped_release release;    // release GIL
            return phylanx::execution_tree::list_patterns();
        },
        "display help strings for Phylanx primitives and plugins.");

    util.def(
        "debug_output",
        []() -> std::string
        {
            pybind11::gil_scoped_release release;    // release GIL
            std::stringstream const& strm = hpx::get_consolestream();
            return strm.str();
        },
        "return all the output generated through the debug() primitive");
}

//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/include/iostreams.hpp>

#include <bindings/binding_helpers.hpp>
#include <bindings/type_casters.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace bindings
{
    ///////////////////////////////////////////////////////////////////////////
    std::string expression_compiler(std::string const& file_name,
        std::string const& xexpr_str, compiler_state& c)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                auto const& code = phylanx::execution_tree::compile(
                    file_name, xexpr_str, c.eval_snippets, c.eval_env);

                auto const& funcs = code.functions();

                if (c.enable_measurements)
                {
                    if (!funcs.empty())
                    {
                        c.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                // add all definitions to the global execution environment
                code.run(c.eval_ctx);

                return !funcs.empty() ? funcs.front().name_ : "";
            });
    }

    phylanx::execution_tree::primitive_argument_type
    expression_evaluator(
        std::string const& file_name, std::string const& xexpr_str,
        compiler_state& c, pybind11::args args)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> phylanx::execution_tree::primitive_argument_type
            {
                // Make sure None is printed as "None"
                phylanx::util::none_wrapper wrap_cout(hpx::cout);
                phylanx::util::none_wrapper wrap_debug(hpx::consolestream);

                auto const& code_x = phylanx::execution_tree::compile(
                    file_name, xexpr_str, c.eval_snippets, c.eval_env);

                if (c.enable_measurements)
                {
                    auto const& funcs = code_x.functions();
                    if (!funcs.empty())
                    {
                        c.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                auto x = code_x.run(c.eval_ctx);

                phylanx::execution_tree::primitive_arguments_type keep_alive;
                keep_alive.reserve(args.size());
                phylanx::execution_tree::primitive_arguments_type fargs;
                fargs.reserve(args.size());

                {
                    pybind11::gil_scoped_acquire acquire;
                    for (auto const& item : args)
                    {
                        using phylanx::execution_tree::primitive_argument_type;

                        primitive_argument_type value =
                            item.cast<primitive_argument_type>();

                        keep_alive.emplace_back(std::move(value));
                        fargs.emplace_back(extract_ref_value(keep_alive.back()));
                    }
                }

                return x(std::move(fargs), c.eval_ctx);
            });
    }

    ///////////////////////////////////////////////////////////////////////////
    // initialize measurements for tree evaluations
    std::vector<std::string> enable_measurements(compiler_state& c,
        bool reset_counters)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::vector<std::string>
        {
            // measurements have been enabled
            c.enable_measurements = true;

            c.primitive_instances =
                phylanx::util::enable_measurements(c.primitive_instances);

            if (reset_counters)
            {
                hpx::reset_active_counters();
            }

            return c.primitive_instances;
        });
    }

    // retrieve performance data from all active performance counters
    std::string retrieve_counter_data(compiler_state& c)
    {
        if (!c.enable_measurements)
        {
            return std::string{};
        }

        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                // make sure all counters 'know' about all primitives
                hpx::reinit_active_counters();

                std::ostringstream os;

                // CSV Header
                os << "primitive_instance,display_name,count,time,eval_direct\n";

                // Print performance data
                for (auto const& entry :
                    phylanx::util::retrieve_counter_data(c.primitive_instances))
                {
                    os << "\"" << entry.first << "\",\""
                       << phylanx::execution_tree::compiler::
                              primitive_display_name(entry.first)
                       << "\"";
                    for (auto const& counter_value : entry.second)
                    {
                        os << "," << counter_value;
                    }
                    os << "\n";
                }

                os << "\n";
                return os.str();
            });
    }

    // retrieve tree topology in DOT format for given expression
    std::string retrieve_dot_tree_topology(std::string const& file_name,
        std::string const& xexpr_str, compiler_state& c)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                auto const& code = phylanx::execution_tree::compile(
                    file_name, xexpr_str, c.eval_snippets, c.eval_env);

                if (c.enable_measurements)
                {
                    auto const& funcs = code.functions();
                    if (!funcs.empty())
                    {
                        c.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                auto const& program = c.eval_snippets.program_;

                std::set<std::string> resolve_children;
                for (auto const& ep : program.entry_points())
                {
                    for (auto const& f : ep.functions())
                    {
                        resolve_children.insert(f.name_);
                    }
                }
                for (auto const& entry : program.scratchpad())
                {
                    for (auto const& f : entry.second)
                    {
                        resolve_children.insert(f.name_);
                    }
                }

                auto topology = program.get_expression_topology(
                    std::set<std::string>{}, std::move(resolve_children));

                return phylanx::execution_tree::dot_tree(file_name, topology);
            });
    }

    std::string retrieve_newick_tree_topology(std::string const& file_name,
        std::string const& xexpr_str, compiler_state& c)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                auto const& code = phylanx::execution_tree::compile(
                    file_name, xexpr_str, c.eval_snippets, c.eval_env);

                if (c.enable_measurements)
                {
                    auto const& funcs = code.functions();
                    if (!funcs.empty())
                    {
                        c.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                auto const& program = c.eval_snippets.program_;

                std::set<std::string> resolve_children;
                for (auto const& ep : program.entry_points())
                {
                    for (auto const& f : ep.functions())
                    {
                        resolve_children.insert(f.name_);
                    }
                }
                for (auto const& entry : program.scratchpad())
                {
                    for (auto const& f : entry.second)
                    {
                        resolve_children.insert(f.name_);
                    }
                }

                auto topology = program.get_expression_topology(
                    std::set<std::string>{}, std::move(resolve_children));

                return phylanx::execution_tree::newick_tree(file_name, topology);
            });
    }

    std::list<std::string> retrieve_tree_topology(
        std::string const& file_name, std::string const& xexpr_str,
        compiler_state& c)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::list<std::string>
            {
                auto const& code = phylanx::execution_tree::compile(
                    file_name, xexpr_str, c.eval_snippets, c.eval_env);

                if (c.enable_measurements)
                {
                    auto const& funcs = code.functions();
                    if (!funcs.empty())
                    {
                        c.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                auto const& program = c.eval_snippets.program_;

                std::set<std::string> resolve_children;
                for (auto const& ep : program.entry_points())
                {
                    for (auto const& f : ep.functions())
                    {
                        resolve_children.insert(f.name_);
                    }
                }
                for (auto const& entry : program.scratchpad())
                {
                    for (auto const& f : entry.second)
                    {
                        resolve_children.insert(f.name_);
                    }
                }

                auto topology = program.get_expression_topology(
                    std::set<std::string>{}, std::move(resolve_children));

                std::list<std::string> result;
                result.push_back(
                    phylanx::execution_tree::newick_tree(file_name, topology));
                result.push_back(
                    phylanx::execution_tree::dot_tree(file_name, topology));

                return result;
            });
    }

    ///////////////////////////////////////////////////////////////////////////
    pybind11::dtype extract_dtype(
        phylanx::execution_tree::primitive_argument_type const& p)
    {
        auto f =
            [&]() -> pybind11::dtype
            {
                using namespace phylanx::execution_tree;

                std::int64_t type_id = p.index();
                if (type_id == primitive_argument_type::primitive_index)
                {
                    primitive_arguments_type args;
                    args.emplace_back(p);

                    primitive type = primitives::create_phytype(
                        hpx::find_here(), std::move(args), "dtype", "<unknown>");

                    primitive_argument_type id = type.eval(hpx::launch::sync);
                    type_id = extract_scalar_integer_value_strict(
                        id, "dtype", "<unknown>");
                }

                pybind11::gil_scoped_acquire acquire;

                switch (type_id)
                {
                case primitive_argument_type::nil_index:
                    return pybind11::dtype("O");

                case primitive_argument_type::bool_index:
                    return pybind11::dtype("int8");

                case primitive_argument_type::int64_index:
                    return pybind11::dtype("int64");

                case primitive_argument_type::string_index:
                    return pybind11::dtype("S");

                case primitive_argument_type::float64_index:
                    return pybind11::dtype("float64");

                case primitive_argument_type::primitive_index:
                    return pybind11::dtype("O");

                case primitive_argument_type::expression_index:
                    return pybind11::dtype("O");

                case primitive_argument_type::list_index:
                    return pybind11::dtype("O");

                case primitive_argument_type::dictionary_index:
                    return pybind11::dtype("O");

                default:
                    break;
                }
                return pybind11::dtype("");
            };

        pybind11::gil_scoped_release release;       // release GIL
        if (hpx::threads::get_self_ptr() == nullptr)
        {
            return hpx::threads::run_as_hpx_thread(std::move(f));
        }
        return f();
    }
}}

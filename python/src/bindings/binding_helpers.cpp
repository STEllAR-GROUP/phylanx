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
    std::string expression_compiler(compiler_state& state,
        std::string const& file_name, std::string const& func_name,
        std::string const& xexpr_str)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                auto const& code = phylanx::execution_tree::compile(
                    file_name, func_name, xexpr_str, state.eval_snippets,
                    state.eval_env);

                auto const& funcs = code.functions();

                if (state.enable_measurements)
                {
                    if (!funcs.empty())
                    {
                        state.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                // add all definitions to the global execution environment
                code.run(state.eval_ctx);

                return !funcs.empty() ? funcs.front().name_ : "";
            });
    }

    phylanx::execution_tree::primitive_argument_type expression_evaluator(
        compiler_state& state, std::string const& file_name,
        std::string const& xexpr_str, pybind11::args args)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> phylanx::execution_tree::primitive_argument_type
            {
                // Make sure None is printed as "None"
                phylanx::util::none_wrapper wrap_cout(hpx::cout);
                phylanx::util::none_wrapper wrap_debug(hpx::consolestream);

                auto const& code_x =
                    phylanx::execution_tree::compile(file_name, xexpr_str,
                        xexpr_str, state.eval_snippets, state.eval_env);

                if (state.enable_measurements)
                {
                    auto const& funcs = code_x.functions();
                    if (!funcs.empty())
                    {
                        state.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                auto x = code_x.run(state.eval_ctx);

                phylanx::execution_tree::primitive_arguments_type fargs;
                fargs.reserve(args.size());

                {
                    pybind11::gil_scoped_acquire acquire;
                    for (auto const& item : args)
                    {
                        using phylanx::execution_tree::primitive_argument_type;
                        fargs.emplace_back(item.cast<primitive_argument_type>());
                    }
                }

                return x(std::move(fargs), state.eval_ctx);
            });
    }

    ///////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::primitive code_for(
        phylanx::bindings::compiler_state& state, std::string const& file_name,
        std::string const& func_name)
    {
        pybind11::gil_scoped_release release;       // release GIL
        return hpx::threads::run_as_hpx_thread([&]()
        {
            using namespace phylanx::execution_tree;

            // if the requested name is defined in the environment, use it
            primitive_argument_type* var = state.eval_ctx.get_var(func_name);
            if (var != nullptr)
            {
                if (is_primitive_operand(*var))
                {
                    return primitive_operand(*var, func_name, file_name);
                }

                return create_primitive_component(hpx::find_here(), "variable",
                    extract_ref_value(*var, func_name, file_name),
                    func_name, file_name, false);
            }

            // alternatively, locate requested function entry point
            for (auto const& entry_point :
                state.eval_snippets.program_.entry_points())
            {
                if (func_name == entry_point.func_name_)
                {
                    auto && funcs = entry_point.functions();
                    if (funcs.empty())
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::bindings::code_for",
                            hpx::util::format("cannot locate requested "
                                "function entry point '{}'", func_name));
                    }

                    if (is_primitive_operand(*var))
                    {
                        return primitive_operand(
                            funcs.back().arg_, func_name, file_name);
                    }

                    return create_primitive_component(hpx::find_here(),
                        "variable", extract_ref_value(
                            funcs.back().arg_, func_name, file_name),
                        func_name, file_name, false);
                }
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::bindings::code_for",
                hpx::util::format("cannot locate requested "
                    "function entry point '{}'", func_name));
            return primitive();
        });
    }

    ///////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::primitive create_bound_primitive(
        phylanx::bindings::compiler_state& state, std::string const& file_name,
        std::string const& func_name, std::string const& func_full_name,
        pybind11::args args,
        phylanx::execution_tree::primitive_argument_type&& value)
    {
        using namespace phylanx::execution_tree;

        // transfer arguments
        primitive_arguments_type fargs;
        fargs.reserve(args.size() + 1);

        // first 'argument' is the function itself
        fargs.push_back(std::move(value));

        // now bind the transferred arguments
        {
            pybind11::gil_scoped_acquire acquire;
            for (auto const& item : args)
            {
                fargs.emplace_back(
                    item.cast<primitive_argument_type>());
            }
        }

        // create a target-reference object that binds arguments
        // to function
        compiler::primitive_name_parts name_parts;
        if (!compiler::parse_primitive_name(func_full_name, name_parts))
        {
            name_parts.instance = func_name;
        }

        name_parts.primitive = "target-reference";
        return create_primitive_component(hpx::find_here(),
            name_parts.primitive, std::move(fargs),
            compiler::compose_primitive_name(name_parts),
            file_name, false);
    }

    phylanx::execution_tree::primitive bound_code_for(
        phylanx::bindings::compiler_state& state, std::string const& file_name,
        std::string const& func_name, pybind11::args args)
    {
        pybind11::gil_scoped_release release;       // release GIL
        return hpx::threads::run_as_hpx_thread([&]()
        {
            using namespace phylanx::execution_tree;

            // if the requested name is defined in the environment, use it
            primitive_argument_type* var = state.eval_ctx.get_var(func_name);
            if (var != nullptr)
            {
                return create_bound_primitive(state, file_name, func_name,
                    func_name, args,
                    extract_ref_value(*var, func_name, file_name));
            }

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
                            "phylanx::bindings::bound_code_for",
                            hpx::util::format("cannot locate requested "
                                "function entry point '{}'", func_name));
                    }
                    return create_bound_primitive(state, file_name, func_name,
                        funcs.back().name_, args,
                        extract_ref_value(
                            funcs.back().arg_, func_name, file_name));
                }
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::bindings::bound_code_for",
                hpx::util::format("cannot locate requested "
                    "function entry point '{}'", func_name));
            return phylanx::execution_tree::primitive();
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
    std::string retrieve_dot_tree_topology(compiler_state& state,
        std::string const& file_name, std::string const& xexpr_str)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                auto const& code = phylanx::execution_tree::compile(
                    file_name, xexpr_str, state.eval_snippets, state.eval_env);

                if (state.enable_measurements)
                {
                    auto const& funcs = code.functions();
                    if (!funcs.empty())
                    {
                        state.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                auto const& program = state.eval_snippets.program_;

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

    std::string retrieve_newick_tree_topology(compiler_state& state,
        std::string const& file_name, std::string const& xexpr_str)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                auto const& code = phylanx::execution_tree::compile(
                    file_name, xexpr_str, state.eval_snippets, state.eval_env);

                if (state.enable_measurements)
                {
                    auto const& funcs = code.functions();
                    if (!funcs.empty())
                    {
                        state.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                auto const& program = state.eval_snippets.program_;

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

    std::list<std::string> retrieve_tree_topology(compiler_state& state,
        std::string const& file_name, std::string const& xexpr_str)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::list<std::string>
            {
                auto const& code = phylanx::execution_tree::compile(
                    file_name, xexpr_str, state.eval_snippets, state.eval_env);

                if (state.enable_measurements)
                {
                    auto const& funcs = code.functions();
                    if (!funcs.empty())
                    {
                        state.primitive_instances.push_back(
                            phylanx::util::enable_measurements(
                                funcs.front().name_));
                    }
                }

                auto const& program = state.eval_snippets.program_;

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
                    args.emplace_back(value_operand_sync(
                        p, primitive_arguments_type{}, "dtype", "<unknown>"));

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

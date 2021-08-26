//  Copyright (c) 2021 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#include <hpx/modules/components_base.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/runtime_local.hpp>
#include <hpx/program_options.hpp>

#include <string>

namespace phylanx { namespace util {

    // This will be called to return special command line options supported by
    // this component.
    hpx::program_options::options_description command_line_options()
    {
        hpx::program_options::options_description opts(
            "Additional command line options for the phylanx component");
        opts.add_options()("phylanx:performance",
            hpx::program_options::value<std::string>()->implicit_value(""),
            "enables the performance counters "
            "implemented by the phylanx primitives");
        return opts;
    }

    // Parse the command line to figure out whether the sine performance
    // counters need to be created.
    bool need_performance_counters(std::string& filename)
    {
        using hpx::util::retrieve_commandline_arguments;

        // Retrieve command line using the HPX ProgramOptions library.
        hpx::program_options::variables_map vm;
        if (!retrieve_commandline_arguments(command_line_options(), vm))
        {
            HPX_THROW_EXCEPTION(hpx::commandline_option_error,
                "phylanx::util::need_performance_counters",
                "Failed to handle command line options");
            return false;
        }

        // We enable the performance counters if --phylanx:performance is
        // specified on the command line.
        if (vm.count("phylanx:performance") == 0)
        {
            return false;
        }

        filename = vm["phylanx:performance"].as<std::string>();
        return true;
    }
}}    // namespace phylanx::util

// Register a function to be called to populate the special command line
// options supported by this component.
//
// Note that this macro can be used not more than once in one module.
HPX_REGISTER_COMMANDLINE_MODULE(::phylanx::util::command_line_options);

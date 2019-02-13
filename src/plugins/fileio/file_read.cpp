//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/file_read.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/execution_tree.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/runtime/threads/run_as_os_thread.hpp>

#include <cstddef>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_read::match_data =
    {
        hpx::util::make_tuple("file_read",
            std::vector<std::string>{"file_read(_1)"},
            &create_file_read, &create_primitive<file_read>,
            R"(fname

            Args:

                fname (string) : a file name

            Returns:

            An object deserialized from the data in fname.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    file_read::file_read(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    // read data from given file and return content
    hpx::future<primitive_argument_type> file_read::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::eval",
                generate_error_message(
                    "the file_read primitive requires exactly one "
                        "literal argument"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::eval",
                generate_error_message(
                    "the file_read primitive requires that the given "
                        "operand is valid"));
        }

        std::string filename = string_operand_sync(
            operands[0], args, name_, codename_, std::move(ctx));

        auto this_ = this->shared_from_this();
        return hpx::threads::run_as_os_thread(
            [filename = std::move(filename), this_ = std::move(this_)]()
            ->  primitive_argument_type
            {
                std::ifstream infile(filename.c_str(),
                    std::ios::binary | std::ios::in | std::ios::ate);

                if (!infile.is_open())
                {
                    throw std::runtime_error(this_->generate_error_message(
                        "couldn't open file: " + filename));
                }

                std::streamsize count = infile.tellg();
                infile.seekg(0);

                std::vector<char> data;
                data.resize(count);

                if (!infile.read(data.data(), count))
                {
                    throw std::runtime_error(this_->generate_error_message(
                        "couldn't read expected number of bytes from file: " +
                        filename));
                }

                // assume data in file is result of a serialized
                // primitive_argument_type
                primitive_argument_type val;
                phylanx::util::unserialize(data, val);

                return val;
            });
    }
}}}

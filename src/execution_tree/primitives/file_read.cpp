//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/file_read.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <cstddef>
#include <fstream>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::file_read>
    file_read_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    file_read_type, phylanx_file_read_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(file_read_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    file_read::file_read(std::vector<primitive_argument_type>&& operands)
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::file_read",
                "the file_read primitive requires exactly one literal "
                    "argument");
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::file_read",
                "the file_read primitive requires that the "
                    "exactly one element of the literals and operands "
                    "arrays is valid");
        }

        std::string* name = util::get_if<std::string>(&operands[0]);
        if (name == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::file_read",
                "the first literal argument must be a string representing a "
                    "valid file name");
        }

        filename_ = std::move(*name);
    }

    // read data from given file and return content
    hpx::future<ir::node_data<double>> file_read::eval() const
    {
        std::ifstream infile(filename_.c_str(),
            std::ios::binary | std::ios::in | std::ios::ate);

        if (!infile.is_open())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::eval",
                "couldn't open file: " + filename_);
        }

        std::streamsize count = infile.tellg();
        infile.seekg(0);

        std::vector<char> data;
        data.resize(count);

        if (!infile.read(data.data(), count))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::eval",
                "couldn't read expected number of bytes from file: " +
                    filename_);
        }

        // assume data in file is result of a serialized ir::node_data
        ir::node_data<double> nd;
        phylanx::util::detail::unserialize(data, nd);

        return hpx::make_ready_future(std::move(nd));
    }
}}}

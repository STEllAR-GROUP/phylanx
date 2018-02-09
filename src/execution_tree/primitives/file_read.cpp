//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/file_read.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/execution_tree.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <cstddef>
#include <fstream>
#include <string>
#include <utility>
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
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_read::match_data =
    {
        hpx::util::make_tuple("file_read",
            std::vector<std::string>{"file_read(_1)"},
            &create<file_read>)
    };

    ///////////////////////////////////////////////////////////////////////////
    file_read::file_read(std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    // read data from given file and return content
    hpx::future<primitive_argument_type> file_read::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::eval",
                "the file_read primitive requires exactly one literal "
                    "argument");
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::eval",
                "the file_read primitive requires that the given operand is "
                    "valid");
        }

        std::string filename = string_operand_sync(operands_[0], args);
        std::ifstream infile(filename.c_str(),
            std::ios::binary | std::ios::in | std::ios::ate);

        if (!infile.is_open())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::eval",
                "couldn't open file: " + filename);
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
                    filename);
        }

        // assume data in file is result of a serialized primitive_argument_type
        primitive_argument_type val;
        phylanx::util::unserialize(data, val);

        return hpx::make_ready_future(std::move(val));
    }
}}}

//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/file_write.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/optional.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <fstream>
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::file_write>
    file_write_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    file_write_type, phylanx_file_write_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(file_write_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_write::match_data =
    {
        "file_write(_1, _2)", &create<file_write>
    };

    ///////////////////////////////////////////////////////////////////////////
    file_write::file_write(std::vector<primitive_argument_type>&& operands)
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::file_write",
                "the file_write primitive requires exactly two operands");
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::file_write",
                "the file_write primitive requires that the given operands "
                    "are valid");
        }

        std::string* name = util::get_if<std::string>(&operands[0]);
        if (name == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::file_write",
                "the first literal argument must be a string representing a "
                    "valid file name");
        }

        filename_ = std::move(*name);
        operand_ = std::move(operands[1]);
    }

    void write_to_file(
        std::string const& filename, primitive_result_type const& val)
    {
        std::ofstream outfile(filename.c_str(),
            std::ios::binary | std::ios::out | std::ios::trunc);
        if (!outfile.is_open())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                "couldn't open file: " + filename);
        }

        std::vector<char> data = phylanx::util::serialize(val);
        if (!outfile.write(data.data(), data.size()))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                "couldn't read expected number of bytes from file: " +
                    filename);
        }
    }

    // read data from given file and return content
    hpx::future<primitive_result_type> file_write::eval() const
    {
        return literal_operand(operand_).then(hpx::util::unwrapping(
            [this](primitive_result_type && val) -> primitive_result_type
            {
                if (!valid(val))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "file_write::eval",
                        "the file_write primitive requires that the argument"
                            " value given by the operand is non-empty");
                }

                write_to_file(filename_, val);
                return primitive_result_type(std::move(val));
            }));
    }
}}}

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
                "the file_write primitive requires that the "
                    "exactly one element of the literals and operands "
                    "arrays is valid");
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
        std::string const& filename, ir::node_data<double> const& nd)
    {
        std::ofstream outfile(filename.c_str(),
            std::ios::binary | std::ios::out | std::ios::trunc);
        if (!outfile.is_open())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                "couldn't open file: " + filename);
        }

        std::vector<char> data = phylanx::util::serialize(nd);
        if (!outfile.write(data.data(), data.size()))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                "couldn't read expected number of bytes from file: " +
                    filename);
        }
    }

    // read data from given file and return content
    hpx::future<util::optional<ir::node_data<double>>> file_write::eval() const
    {
        primitive const* p = util::get_if<primitive>(&operand_);
        if (p != nullptr)
        {
            return p->eval().then(hpx::util::unwrapping(
                [this](util::optional<ir::node_data<double>> && nd)
                ->  operand_type
                {
                    write_to_file(filename_, nd.value());
                    return operand_type(std::move(nd));
                }));
        }

        ir::node_data<double> const* nd =
            util::get_if<ir::node_data<double>>(&operand_);

        if (nd == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                "second argument must be a literator of type "
                    "ir::node_data<double> or another primitive");
        }

        write_to_file(filename_, *nd);
        return hpx::make_ready_future(operand_type(*nd));
    }
}}}

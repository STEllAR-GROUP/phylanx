//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/file_write.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/execution_tree.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_file_write(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("file_write");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const file_write::match_data =
    {
        hpx::util::make_tuple("file_write",
            std::vector<std::string>{"file_write(_1, _2)"},
            &create_file_write, &create_primitive<file_write>)
    };

    ///////////////////////////////////////////////////////////////////////////
    file_write::file_write(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    void file_write::write_to_file(
        primitive_argument_type const& val, std::string const& filename) const
    {
        std::ofstream outfile(filename.c_str(),
            std::ios::binary | std::ios::out | std::ios::trunc);
        if (!outfile.is_open())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                execution_tree::generate_error_message(
                    "couldn't open file: " + filename,
                    name_, codename_));
        }

        std::vector<char> data = phylanx::util::serialize(val);
        if (!outfile.write(data.data(), data.size()))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                execution_tree::generate_error_message(
                    "couldn't read expected number of bytes from file: " +
                        filename,
                    name_, codename_));
        }
    }

    hpx::future<primitive_argument_type> file_write::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                execution_tree::generate_error_message(
                    "the file_write primitive requires exactly two "
                    "operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                execution_tree::generate_error_message(
                    "the file_write primitive requires that the "
                    "given operands are valid",
                    name_, codename_));
        }

        std::string filename =
            string_operand_sync(operands[0], args, name_, codename_);

        auto this_ = this->shared_from_this();
        return literal_operand(operands[1], args, name_, codename_)
            .then(hpx::util::unwrapping(
                [this_, filename = std::move(filename)](
                    primitive_argument_type && val) ->  primitive_argument_type
        {
            if (!valid(val))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "file_write::eval",
                    execution_tree::generate_error_message(
                        "the file_write primitive requires that the argument "
                        "value given by the operand is non-empty",
                        this_->name_, this_->codename_));
            }

            this_->write_to_file(val, std::move(filename));
            return primitive_argument_type(std::move(val));
        }));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Write data to given file and return content
    hpx::future<primitive_argument_type> file_write::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

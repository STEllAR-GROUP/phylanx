//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/file_write.hpp>
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
    match_pattern_type const file_write::match_data =
    {
        hpx::util::make_tuple("file_write",
            std::vector<std::string>{"file_write(_1, _2)"},
            &create_file_write, &create_primitive<file_write>,
            R"(fname, obj
            Args:

                fname (string): the file in which to save the data
                obj (object): the object to serialize

            Returns:)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    file_write::file_write(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> file_write::write_to_file(
        primitive_argument_type && val, std::string && filename) const
    {
        auto this_ = this->shared_from_this();
        return hpx::threads::run_as_os_thread(
            [this_ = std::move(this_)](
                primitive_argument_type && val, std::string && filename)
            {
                std::ofstream outfile(filename.c_str(),
                    std::ios::binary | std::ios::out | std::ios::trunc);
                if (!outfile.is_open())
                {
                    throw std::runtime_error(this_->generate_error_message(
                        "couldn't open file: " + filename));
                }

                std::vector<char> data = phylanx::util::serialize(val);
                if (!outfile.write(data.data(), data.size()))
                {
                    throw std::runtime_error(this_->generate_error_message(
                        "couldn't read expected number of bytes from file: " +
                        filename));
                }
                return primitive_argument_type{std::move(val)};
            },
            std::move(val), std::move(filename));
    }

    hpx::future<primitive_argument_type> file_write::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                generate_error_message(
                    "the file_write primitive requires exactly two "
                    "operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::eval",
                generate_error_message(
                    "the file_write primitive requires that the "
                    "given operands are valid"));
        }

        std::string filename =
            string_operand_sync(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return value_operand(operands[1], args, name_, codename_, std::move(ctx))
            .then(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_), filename = std::move(filename)](
                        primitive_argument_type && val) mutable
                ->  hpx::future<primitive_argument_type>
                {
                    if (!valid(val))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "file_write::eval",
                            this_->generate_error_message(
                                "the file_write primitive requires that the "
                                "argument value given by the operand is "
                                "non-empty"));
                    }

                    return this_->write_to_file(
                        std::move(val), std::move(filename));
                }));
    }
}}}

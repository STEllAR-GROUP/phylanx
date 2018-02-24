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

    namespace detail
    {
        struct file_write : std::enable_shared_from_this<file_write>
        {
            file_write(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {
            }

        protected:
            std::string name_;
            std::string codename_;

        protected:
            void write_to_file(primitive_argument_type const& val)
            {
                std::ofstream outfile(filename_.c_str(),
                    std::ios::binary | std::ios::out | std::ios::trunc);
                if (!outfile.is_open())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::eval",
                        generate_error_message(
                            "couldn't open file: " + filename_,
                            name_, codename_));
                }

                std::vector<char> data = phylanx::util::serialize(val);
                if (!outfile.write(data.data(), data.size()))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::eval",
                        generate_error_message(
                            "couldn't read expected number of bytes from "
                                "file: " + filename_,
                            name_, codename_));
                }
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::"
                            "eval",
                        generate_error_message(
                            "the file_write primitive requires exactly two "
                                "operands",
                            name_, codename_));
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::"
                            "eval",
                        generate_error_message(
                            "the file_write primitive requires that the "
                                "given operands are valid",
                            name_, codename_));
                }

                filename_ =
                    string_operand_sync(operands[0], args, name_, codename_);

                auto this_ = this->shared_from_this();
                return literal_operand(operands[1], args, name_, codename_)
                    .then(hpx::util::unwrapping(
                        [this_](primitive_argument_type && val)
                        ->  primitive_argument_type
                        {
                            if (!valid(val))
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "file_write::eval",
                                    generate_error_message(
                                        "the file_write primitive requires "
                                            "that the argument value given "
                                            "by the operand is non-empty",
                                    this_->name_, this_->codename_));
                            }

                            this_->write_to_file(val);
                            return primitive_argument_type(std::move(val));
                        }));
            }

        private:
            std::string filename_;
            primitive_argument_type operand_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // write data to given file and return content
    hpx::future<primitive_argument_type> file_write::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::file_write>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::file_write>(name_, codename_)
            ->eval(operands_, args);
    }
}}}

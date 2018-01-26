//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/file_write.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/execution_tree.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
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
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_write::match_data =
    {
        hpx::util::make_tuple("file_write",
            std::vector<std::string>{"file_write(_1, _2)"},
            &create<file_write>)
    };

    ///////////////////////////////////////////////////////////////////////////
    file_write::file_write(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {}

    namespace detail
    {
        struct file_write : std::enable_shared_from_this<file_write>
        {
            file_write() = default;

        protected:
            void write_to_file(primitive_result_type const& val)
            {
                std::ofstream outfile(filename_.c_str(),
                    std::ios::binary | std::ios::out | std::ios::trunc);
                if (!outfile.is_open())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::eval",
                        "couldn't open file: " + filename_);
                }

                std::vector<char> data = phylanx::util::serialize(val);
                if (!outfile.write(data.data(), data.size()))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::eval",
                        "couldn't read expected number of bytes from file: " +
                            filename_);
                }
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::"
                            "eval",
                        "the file_write primitive requires exactly two operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::"
                            "eval",
                        "the file_write primitive requires that the given "
                            "operands are valid");
                }

                filename_ = string_operand_sync(operands[0], args);

                auto this_ = this->shared_from_this();
                return literal_operand(operands[1], args)
                    .then(hpx::util::unwrapping(
                        [this_](primitive_result_type && val)
                        ->  primitive_result_type
                        {
                            if (!valid(val))
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "file_write::eval",
                                    "the file_write primitive requires that the "
                                        "argument value given by the operand is "
                                        "non-empty");
                            }

                            this_->write_to_file(val);
                            return primitive_result_type(std::move(val));
                        }));
            }

        private:
            std::string filename_;
            primitive_argument_type operand_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // write data to given file and return content
    hpx::future<primitive_result_type> file_write::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::file_write>()->eval(args, noargs);
        }

        return std::make_shared<detail::file_write>()->eval(operands_, args);
    }
}}}

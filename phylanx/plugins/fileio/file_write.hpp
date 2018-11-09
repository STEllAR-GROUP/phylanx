//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FILE_WRITE_SEP_17_2017_0111PM)
#define PHYLANX_PRIMITIVES_FILE_WRITE_SEP_17_2017_0111PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class file_write
      : public primitive_component_base
      , public std::enable_shared_from_this<file_write>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args, eval_context ctx) const;

    public:
        static match_pattern_type const match_data;

        file_write() = default;

        file_write(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        hpx::future<primitive_argument_type> write_to_file(
            primitive_argument_type&& val, std::string&& filename) const;

        std::string filename_;
        primitive_argument_type operand_;
    };

    inline primitive create_file_write(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "file_write", std::move(operands), name, codename);
    }
}}}

#endif



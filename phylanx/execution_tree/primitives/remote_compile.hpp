//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_REMOTE_COMPILE_FEB_10_2020_200PM)
#define      PHYLANX_PRIMITIVES_REMOTE_COMPILE_FEB_10_2020_200PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/execution_tree/compiler_component.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>
#include <map>

using phylanx::execution_tree::physl_compiler;

namespace phylanx { namespace execution_tree { namespace primitives
{
    class remote_compile
      : public primitive_component_base
      , public std::enable_shared_from_this<remote_compile>
    {
    public:
        static match_pattern_type const match_data;

        remote_compile() = default;

        remote_compile(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        static std::map<int,hpx::naming::id_type> compilers;
    };

    PHYLANX_EXPORT primitive create_remote_compile(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif



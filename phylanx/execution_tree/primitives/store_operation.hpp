// Copyright (c) 2017 Alireza kheirkhahan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_STORE_PRIMITIVE_OCT_05_2017_0204PM)
#define PHYLANX_PRIMITIVES_STORE_PRIMITIVE_OCT_05_2017_0204PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx {namespace execution_tree { namespace primitives
{
    class store_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<store_operation>
    {
    public:
        static match_pattern_type const match_data;

        store_operation() = default;

        store_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args,
            eval_context) const override;

        hpx::future<primitive_argument_type> eval(
            primitive_argument_type&& arg, eval_context ctx) const override;
    };

    PHYLANX_EXPORT primitive create_store_operation(
        hpx::id_type const& locality, primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif

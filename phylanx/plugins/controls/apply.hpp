//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_APPLY_FEB_09_2018_0744PM)
#define PHYLANX_PRIMITIVES_APPLY_FEB_09_2018_0744PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class apply
      : public primitive_component_base
      , public std::enable_shared_from_this<apply>
    {
    public:
        static match_pattern_type const match_data;

        apply() = default;

        apply(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context ctx) const override;
    };

    inline primitive create_apply(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "apply", std::move(operands), name, codename);
    }
}}}

#endif



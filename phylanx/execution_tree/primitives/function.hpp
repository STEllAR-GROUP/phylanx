//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FUNCTION_JUN_29_2018_0734AM)
#define PHYLANX_PRIMITIVES_FUNCTION_JUN_29_2018_0734AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class function
      : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        function() = default;

        function(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context ctx) const override;

        bool bind(primitive_arguments_type const& params,
            eval_context ctx) const override;

        void store(primitive_argument_type&& data,
            primitive_arguments_type&& params, eval_context ctx) override;

        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

    private:
        bool value_set_;
    };

    PHYLANX_EXPORT primitive create_function(hpx::id_type const& locality,
        primitive_argument_type&& operand,
        std::string const& name = "", std::string const& codename = "",
        bool register_with_agas = true);
}}}

#endif



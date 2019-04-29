//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ACCESS_FUNCTION_JUN_29_2018_0535AM)
#define PHYLANX_PRIMITIVES_ACCESS_FUNCTION_JUN_29_2018_0535AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/util/hashed_string.hpp>

#include <hpx/lcos/future.hpp>

#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class access_function : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        access_function() = default;

        access_function(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        void store(primitive_arguments_type&& val,
            primitive_arguments_type&& params, eval_context ctx) override;
        void store(primitive_argument_type&& val,
            primitive_arguments_type&& params, eval_context ctx) override;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context ctx) const override;

        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

    private:
        util::hashed_string target_name_;   // name of the represented variable
    };
}}}

#endif


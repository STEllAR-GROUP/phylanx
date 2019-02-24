//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CALL_FUNCCTION_OCT_22_2017_0111PM)
#define PHYLANX_PRIMITIVES_CALL_FUNCCTION_OCT_22_2017_0111PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class call_function
      : public primitive_component_base
      , public std::enable_shared_from_this<call_function>
    {
    public:
        static match_pattern_type const match_data;

        call_function() = default;

        call_function(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        // return the topology for this function definition
        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context ctx) const override;

        void store(primitive_arguments_type&& data,
            primitive_arguments_type&& params, eval_context ctx) override;
    };
}}}

#endif


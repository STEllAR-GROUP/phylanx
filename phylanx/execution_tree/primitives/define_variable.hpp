//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DEF_OPERATION_OCT_13_2017_1120AM)
#define PHYLANX_PRIMITIVES_DEF_OPERATION_OCT_13_2017_1120AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/util/hashed_string.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <set>
#include <string>

namespace phylanx { namespace execution_tree { namespace primitives
{
    // This primitive creates a new instance of the variable of the given name
    // and initializes it by evaluating the given body.
    //
    // This is a helper primitive needed for proper binding of the expression
    // value to a variable.
    class define_variable
      : public primitive_component_base
      , public std::enable_shared_from_this<define_variable>
    {
    public:
        static match_pattern_type const match_data;
        static match_pattern_type const match_data_define;
        static match_pattern_type const match_data_lambda;

        define_variable() = default;

        define_variable(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        // Create a new instance of the variable and initialize it with the
        // value as returned by evaluating the given body.
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        // return the topology for this variable definition
        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

    private:
        util::hashed_string target_name_;   // name of the represented variable
    };
}}}

#endif



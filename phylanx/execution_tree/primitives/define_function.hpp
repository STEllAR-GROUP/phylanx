// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DEF_FUNCTION_NOV_01_2017_1242PM)
#define PHYLANX_PRIMITIVES_DEF_FUNCTION_NOV_01_2017_1242PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    // This primitive creates a new instance of the variable of the given name
    // and initializes it by evaluating the given body.
    //
    // This is a helper primitive needed for proper binding of the expression
    // value to a variable.
    class define_function
        : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;
        static match_pattern_type const match_data_lambda;

        define_function() = default;

        define_function(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        // Create a new instance of the variable and initialize it with the
        // value as returned by evaluating the given body.
        primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& args) const override;

        // return the topology for this function definition
        topology expression_topology(
            std::set<std::string>&& functions) const override;

        // Initialize the expression representing the function body, this has
        // to be done separately in order to support recursive functions.
        void set_body(primitive_argument_type&& target) override;
    };
}}}

#endif



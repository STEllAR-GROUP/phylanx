//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DEF_FUNCTION_NOV_01_2017_1242PM)
#define PHYLANX_PRIMITIVES_DEF_FUNCTION_NOV_01_2017_1242PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    // This primitive creates a new instance of the variable of the given name
    // and initializes it by evaluating the given body.
    //
    // This is a helper primitive needed for proper binding of the expression
    // value to a variable.
    class HPX_COMPONENT_EXPORT define_function
      : public base_primitive
      , public hpx::components::component_base<define_function>
    {
    public:
        define_function() = default;

        define_function(primitive_argument_type&& operands);
        define_function(primitive_argument_type&& operands, std::string&& name);

        // Create a new instance of the variable and initialize it with the
        // value as returned by evaluating the given body.
        primitive_result_type eval_direct(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type body_;
        mutable primitive_argument_type target_;
        std::string name_;
    };
}}}

#endif



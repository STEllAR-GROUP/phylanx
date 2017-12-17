//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DEF_OPERATION_OCT_13_2017_1120AM)
#define PHYLANX_PRIMITIVES_DEF_OPERATION_OCT_13_2017_1120AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    // This primitive creates a new instance of the variable of the given name
    // and initializes it by evaluating the given body.
    //
    // This is a helper primitive needed for proper binding of the expression
    // value to a variable.
    class HPX_COMPONENT_EXPORT define_variable
      : public base_primitive
      , public hpx::components::component_base<define_variable>
    {
    public:
        static std::vector<match_pattern_type> const match_data;

        define_variable() = default;

        define_variable(primitive_argument_type&& operands);
        define_variable(primitive_argument_type&& operands, std::string name);

        // Create a new instance of the variable and initialize it with the
        // value as returned by evaluating the given body.
        primitive_result_type eval_direct(
            std::vector<primitive_argument_type> const& args) const override;
        void store(primitive_result_type && val) override;

    protected:
        std::string extract_function_name() const;

    private:
        primitive_argument_type body_;
        mutable primitive_argument_type target_;
        std::string name_;
    };
}}}

#endif



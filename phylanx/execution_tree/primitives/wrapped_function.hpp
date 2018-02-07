//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_WRAPPED_PRIMITIVE_OCT_22_2017_0111PM)
#define PHYLANX_PRIMITIVES_WRAPPED_PRIMITIVE_OCT_22_2017_0111PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class wrapped_function
      : public base_primitive
      , public hpx::components::component_base<wrapped_function>
    {
    public:
        wrapped_function() = default;

        PHYLANX_EXPORT wrapped_function(
            primitive_argument_type target, std::string name);
        PHYLANX_EXPORT wrapped_function(primitive_argument_type target,
            std::vector<primitive_argument_type>&& operands, std::string name);

        // return the topology for this function definition
        PHYLANX_EXPORT topology expression_topology() const override;

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        primitive_argument_type target_;
        std::vector<primitive_argument_type> args_;
        std::string name_;
    };
}}}

#endif


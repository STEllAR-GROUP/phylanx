//  Copyright (c) 2017 Hartmut Kaiser
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
    class HPX_COMPONENT_EXPORT wrapped_function
      : public base_primitive
      , public hpx::components::locking_hook<
          hpx::components::component_base<wrapped_function>>
    {
    public:
        wrapped_function() = default;

        wrapped_function(std::string name);

        wrapped_function(primitive_argument_type target);
        wrapped_function(primitive_argument_type target, std::string name);

        wrapped_function(primitive_argument_type target,
            std::vector<primitive_argument_type>&& operands);
        wrapped_function(primitive_argument_type target,
            std::vector<primitive_argument_type>&& operands, std::string name);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        primitive_argument_type target_;
        std::vector<primitive_argument_type> args_;
        std::string name_;
    };
}}}

#endif


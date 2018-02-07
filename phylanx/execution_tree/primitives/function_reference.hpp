//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FUNCTION_REFERENCE_JAN_23_2018_0557PM)
#define PHYLANX_PRIMITIVES_FUNCTION_REFERENCE_JAN_23_2018_0557PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class function_reference
      : public base_primitive
      , public hpx::components::component_base<function_reference>
    {
    public:
        function_reference() = default;

        PHYLANX_EXPORT function_reference(primitive_argument_type target,
            std::vector<primitive_argument_type>&& operands, std::string name);

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

        PHYLANX_EXPORT topology expression_topology() const override;

    private:
        primitive_argument_type target_;
        std::vector<primitive_argument_type> args_;
        std::string name_;
    };
}}}

#endif


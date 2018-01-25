//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CONSOLE_OUTPUT_NOV_07_2017_1150AM)
#define PHYLANX_PRIMITIVES_CONSOLE_OUTPUT_NOV_07_2017_1150AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT console_output
      : public base_primitive
      , public hpx::components::component_base<console_output>
    {
    public:
        static match_pattern_type const match_data;

        console_output() = default;

        console_output(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
        bool bind(std::vector<primitive_argument_type> const&) override;
    };
}}}

#endif



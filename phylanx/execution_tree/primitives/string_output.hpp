//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_STRING_OUTPUT_JAN_18_2018_0105PM)
#define PHYLANX_PRIMITIVES_STRING_OUTPUT_JAN_18_2018_0105PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class string_output
      : public base_primitive
      , public hpx::components::component_base<string_output>
    {
    public:
        static match_pattern_type const match_data;

        string_output() = default;

        string_output(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif



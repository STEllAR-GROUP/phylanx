//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_ENABLE_TRACING_FEB_05_2018_0523PM)
#define PHYLANX_ENABLE_TRACING_FEB_05_2018_0523PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <cstddef>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT enable_tracing
        : public base_primitive
        , public hpx::components::component_base<enable_tracing>
    {
    public:
        static match_pattern_type const match_data;

        enable_tracing() = default;

        enable_tracing(std::vector<primitive_argument_type>&& operands);

        primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& params) const override;
    };
}}}

#endif



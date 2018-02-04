//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_EXTRACT_SHAPE_OCT_25_2017_1237PM)
#define PHYLANX_PRIMITIVES_EXTRACT_SHAPE_OCT_25_2017_1237PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <cstddef>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT extract_shape
        : public base_primitive
        , public hpx::components::component_base<extract_shape>
    {
    public:
        static match_pattern_type const match_data;

        extract_shape() = default;

        extract_shape(std::vector<primitive_argument_type> && params);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;
    };
}}}

#endif



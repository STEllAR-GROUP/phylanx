//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM)
#define PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    class HPX_COMPONENT_EXPORT add_operation
      : public base_primitive
      , public hpx::components::component_base<add_operation>
    {
    public:
        static match_pattern_type const match_data;

        add_operation() = default;

        add_operation(std::vector<primitive_argument_type> && operands);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif

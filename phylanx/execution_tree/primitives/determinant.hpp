//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DETERMINANT_OCT_09_2017_0158PM)
#define PHYLANX_PRIMITIVES_DETERMINANT_OCT_09_2017_0158PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class determinant
      : public base_primitive
      , public hpx::components::component_base<determinant>
    {
    public:
        static match_pattern_type const match_data;

        determinant() = default;

        determinant(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif

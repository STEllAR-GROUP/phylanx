//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_OR_OPERATION_OCT_06_2017_0522PM)
#define PHYLANX_PRIMITIVES_OR_OPERATION_OCT_06_2017_0522PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>

#include <cstdint>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class or_operation
      : public base_primitive
      , public hpx::components::component_base<or_operation>
    {
    private:
        using operands_type = std::vector<std::uint8_t>;

    public:
        static match_pattern_type const match_data;

        or_operation() = default;

        PHYLANX_EXPORT or_operation(
            std::vector<primitive_argument_type>&& operands);

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif



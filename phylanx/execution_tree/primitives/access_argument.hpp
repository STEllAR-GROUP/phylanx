//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ACCESS_ARGUMENT_OCT_20_2017_0804PM)
#define PHYLANX_PRIMITIVES_ACCESS_ARGUMENT_OCT_20_2017_0804PM

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
    class access_argument
        : public base_primitive
        , public hpx::components::component_base<access_argument>
    {
    public:
        static match_pattern_type const match_data;

        access_argument() = default;

        access_argument(std::size_t argnum)
          : argnum_(argnum)
        {}

        PHYLANX_EXPORT primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        std::size_t argnum_;
    };
}}}

#endif



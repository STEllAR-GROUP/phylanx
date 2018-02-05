//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_EQUAL_OCT_07_2017_0212PM)
#define PHYLANX_PRIMITIVES_EQUAL_OCT_07_2017_0212PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT equal
        : public base_primitive
        , public hpx::components::component_base<equal>
    {
    public:
        static match_pattern_type const match_data;
        static match_pattern_type const match_data_element_wise;

        equal() = default;

        equal(std::vector<primitive_argument_type>&& operands,
            bool element_wise = false);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        bool element_wise_;
    };
}}}

#endif




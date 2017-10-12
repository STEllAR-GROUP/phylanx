//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_VARIABLE_SEP_05_2017_1105AM)
#define PHYLANX_PRIMITIVES_VARIABLE_SEP_05_2017_1105AM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>

#include <utility>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT variable
      : public base_primitive
      , public hpx::components::locking_hook<
          hpx::components::component_base<variable>>
    {
    public:
        variable() = default;

        variable(primitive_argument_type&& operand);
        variable(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval() const override;
        void store(primitive_result_type const& data) override;

    private:
        primitive_result_type data_;
    };
}}}

#endif



// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_WHILE_OPERATION_OCT_06_2017_1127AM)
#define PHYLANX_PRIMITIVES_WHILE_OPERATION_OCT_06_2017_1127AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class while_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<while_operation>
    {
    public:
        static match_pattern_type const match_data;

        while_operation() = default;

        while_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context ctx) const override;

    private:
        struct iteration;
    };

    inline primitive create_while_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "while", std::move(operands), name, codename);
    }
}}}

#endif



// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_ASYNC_OPERATION_MAR_13_2020_1156AM)
#define PHYLANX_ASYNC_OPERATION_MAR_13_2020_1156AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <utility>

namespace phylanx { namespace execution_tree { namespace primitives {

    class async_operation
      : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        async_operation() = default;

        async_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
    };

    inline primitive create_async_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "async", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::execution_tree::primitives

#endif

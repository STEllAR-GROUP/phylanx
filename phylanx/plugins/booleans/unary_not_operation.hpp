// Copyright (c) 2017-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_UNARY_NOT_OPERATION_OCT_10_2017_0310PM)
#define PHYLANX_PRIMITIVES_UNARY_NOT_OPERATION_OCT_10_2017_0310PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class unary_not_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<unary_not_operation>
    {
    protected:
        using operand_type = ir::node_data<std::uint8_t>;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data[2];

        unary_not_operation() = default;

        unary_not_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        struct visit_unary_not;

        template <typename T>
        primitive_argument_type unary_not_all(ir::node_data<T>&& ops) const;
    };

    inline primitive create_unary_not_operation(
        hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__not", std::move(operands), name, codename);
    }
}}}

#endif



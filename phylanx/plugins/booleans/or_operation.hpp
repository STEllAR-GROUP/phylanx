//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_OR_OPERATION_OCT_06_2017_0522PM)
#define PHYLANX_PRIMITIVES_OR_OPERATION_OCT_06_2017_0522PM

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
    class or_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<or_operation>
    {
    protected:
        using operand_type = ir::node_data<std::uint8_t>;
        using operands_type = primitive_arguments_type;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args) const;

    public:
        static match_pattern_type const match_data;

        or_operation() = default;

        or_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args, eval_mode) const override;

    private:
        struct visit_or_operation;
        template <typename T>
        primitive_argument_type or_operation0d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation0d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation1d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation1d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation1d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation2d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation2d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation2d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_operation2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type or_all(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
    };

    inline primitive create_or_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__or", std::move(operands), name, codename);
    }
}}}

#endif



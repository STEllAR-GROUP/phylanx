// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_GREATER_OCT_07_2017_0223PM)
#define PHYLANX_PRIMITIVES_GREATER_OCT_07_2017_0223PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class greater
      : public primitive_component_base
      , public std::enable_shared_from_this<greater>
    {
    protected:
        using operands_type = primitive_arguments_type;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args) const;

    public:
        static match_pattern_type const match_data;

        greater() = default;

        greater(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args, eval_mode) const override;

    private:
        struct visit_greater;

        template <typename T>
        primitive_argument_type greater0d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater0d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater0d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater1d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater1d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater1d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater2d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater2d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater2d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type greater_all(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
    };

    inline primitive create_greater(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__gt", std::move(operands), name, codename);
    }
}}}

#endif



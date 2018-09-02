// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Shahrzad Shirzad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_NOT_EQUAL_OCT_07_2017_0212PM)
#define PHYLANX_PRIMITIVES_NOT_EQUAL_OCT_07_2017_0212PM

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
    class not_equal
      : public primitive_component_base
      , public std::enable_shared_from_this<not_equal>
    {
    protected:
        using operands_type = primitive_arguments_type;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args) const;

    public:
        static match_pattern_type const match_data;

        not_equal() = default;

        not_equal(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args, eval_mode) const override;

    private:
        struct visit_not_equal;

        template <typename T>
        primitive_argument_type not_equal0d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal0d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal0d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal1d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal1d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal1d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal2d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal2d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal2d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type not_equal_all(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
    };

    inline primitive create_not_equal(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__ne", std::move(operands), name, codename);
    }
}}}

#endif



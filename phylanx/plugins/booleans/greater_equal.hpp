//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_greater_equal_OCT_07_2017_0212PM)
#define PHYLANX_PRIMITIVES_greater_equal_OCT_07_2017_0212PM

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
    class greater_equal
      : public primitive_component_base
      , public std::enable_shared_from_this<greater_equal>
    {
    protected:
        using operand_type = ir::node_data<double>;
        using operands_type = primitive_arguments_type;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args) const;

    public:
        static match_pattern_type const match_data;

        greater_equal() = default;

        greater_equal(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args) const override;

    private:
        struct visit_greater_equal;

        template <typename T>
        primitive_argument_type greater_equal0d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal0d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal0d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal1d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal1d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal1d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal2d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal2d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal2d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type greater_equal_all(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
    };

    inline primitive create_greater_equal(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__ge", std::move(operands), name, codename);
    }
}}}

#endif



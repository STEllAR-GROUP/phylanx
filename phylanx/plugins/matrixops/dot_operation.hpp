//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Alireza Kheirkhahan
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DOT_OPERATION_OCT_10_2017_0501PM)
#define PHYLANX_PRIMITIVES_DOT_OPERATION_OCT_10_2017_0501PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class dot_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<dot_operation>
    {
    public:
        enum dot_mode
        {
            outer_product,
            dot_product,
            doubledot_product
        };

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
        using val_type = std::int64_t;

    public:
        static std::vector<match_pattern_type> const match_data;

        dot_operation() = default;

        dot_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        void positivize_axis(val_type& axis, std::size_t const& dim) const;

        template <typename T>
        blaze::DynamicVector<T> convert_to_1d(ir::node_data<T>&& arr) const;

        primitive_argument_type outer1d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type outer2d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type outer_nd(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type outer_nd_helper(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        template <typename T>
        primitive_argument_type outer1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename Vector1, typename Vector2>
        primitive_argument_type outer1d1d(Vector1&& lhs, Vector2&& rhs) const;
        template <typename T>
        primitive_argument_type outer1d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type outer2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type outer2d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type outer_nd_helper(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type outer3d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        template <typename T>
        primitive_argument_type outer3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif
        primitive_argument_type dot0d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type dot1d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type dot2d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        primitive_argument_type dot_nd(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        template <typename T>
        primitive_argument_type dot0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot0d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot0d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot0d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type dot1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot1d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot1d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot1d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type dot2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot2d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot2d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename Matrix1, typename Matrix2>
        primitive_argument_type dot2d2d(Matrix1&& lhs, Matrix2&& rhs) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type dot0d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot1d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot2d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type dot3d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot3d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot3d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot3d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        primitive_argument_type dot3d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        template <typename T>
        primitive_argument_type dot3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif

        primitive_argument_type contraction2d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        primitive_argument_type contraction_nd(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        template <typename T>
        primitive_argument_type contraction2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename Matrix1, typename Matrix2>
        blaze::ElementType_t<typename std::decay<Matrix1>::type>
        contraction2d2d(Matrix1&& lhs, Matrix2&& rhs) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type contraction2d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        primitive_argument_type contraction3d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        template <typename T>
        primitive_argument_type contraction3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type contraction3d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type contraction3d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif

        primitive_argument_type tensordot_range_of_scalars(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs,
            val_type axis_a, val_type axis_b) const;

        primitive_argument_type tensordot_scalar_axis(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs,
            ir::range&& axes) const;
        primitive_argument_type tensordot_range_axes(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs,
            ir::range&& axes) const;

        template <typename T>
        primitive_argument_type tensordot_range_of_scalars(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, val_type axis_a,
            val_type axis_b) const;

        template <typename T>
        primitive_argument_type tensordot2d2d_0_1(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type tensordot1d3d_0_0(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot2d3d_0_0(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot2d3d_0_1(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot2d3d_0_2(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot2d3d_1_0(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot2d3d_1_2(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot3d2d_0_0(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot3d2d_0_1(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot3d2d_1_0(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot3d2d_1_1(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type tensordot3d2d_2_1(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif

    private:
        dot_mode mode_;
    };

    inline primitive create_outer_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "outer", std::move(operands), name, codename);
    }

    inline primitive create_dot_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "dot", std::move(operands), name, codename);
    }

    inline primitive create_tensordot_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "tensordot", std::move(operands), name, codename);
    }
}}}

#endif

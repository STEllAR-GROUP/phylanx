// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_KERAS_SUPPORT_SOFTMAX_OPERATION)
#define PHYLANX_PLUGINS_KERAS_SUPPORT_SOFTMAX_OPERATION

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

namespace phylanx {  namespace execution_tree {  namespace primitives  {
/// \brief Returns an array of the same shape which is the normalized exponential
///        function of the given array.  The resulting array consists of real
///        values in the range (0..1], which add up to 1 in direction of the
///        given axis
///
/// \param a      The scalar, vector, or matrix to perform softmax over
/// \param axis   Optional. The default is the last axis (axis == -1). Effective
///               when the array is >1d

    class softmax_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<softmax_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
        using val_type = double;
        using arg_type = ir::node_data<val_type>;

    public:
        static match_pattern_type const match_data;

        softmax_operation() = default;

        softmax_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type softmax0d() const;
        primitive_argument_type softmax1d(arg_type&& arg) const;

        primitive_argument_type softmax2d(
            arg_type&& arg, std::int64_t axis) const;
        primitive_argument_type softmax2d_axis0(arg_type&& arg) const;
        primitive_argument_type softmax2d_axis1(arg_type&& arg) const;

        primitive_argument_type softmax3d_axis0(arg_type&& arg) const;
        primitive_argument_type softmax3d_axis1(arg_type&& arg) const;
        primitive_argument_type softmax3d_axis2(arg_type&& arg) const;
        primitive_argument_type softmax3d(
            arg_type&& arg, std::int64_t axis) const;

        primitive_argument_type softmax4d_axis0(arg_type&& arg) const;
        primitive_argument_type softmax4d_axis1(arg_type&& arg) const;
        primitive_argument_type softmax4d_axis2(arg_type&& arg) const;
        primitive_argument_type softmax4d_axis3(arg_type&& arg) const;
        primitive_argument_type softmax4d(
            arg_type&& arg, std::int64_t axis) const;
    };

    inline primitive create_softmax_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "softmax", std::move(operands), name, codename);
    }
}}}

#endif

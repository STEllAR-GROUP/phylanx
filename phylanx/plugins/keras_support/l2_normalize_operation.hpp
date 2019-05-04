// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_KERAS_SUPPORT_L2_NORMALIZE_OPERATION)
#define PHYLANX_PLUGINS_KERAS_SUPPORT_L2_NORMALIZE_OPERATION

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
/// \brief Returns an array of the same shape which is l2 normalized
///
/// \param a      The scalar, vector, matrix, or tensor to perform l2
///               normalization over
/// \param axis   Optional. The default is None (normalizing over the whole
///               array).
/// Immitates the functionality of https://keras.io/backend/#l2_normalize

    class l2_normalize_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<l2_normalize_operation>
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

        l2_normalize_operation() = default;

        l2_normalize_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type l2_normalize0d() const;

        primitive_argument_type l2_normalize1d(arg_type&& arg) const;

        primitive_argument_type l2_normalize2d_axis0(arg_type&& arg) const;
        primitive_argument_type l2_normalize2d_axis1(arg_type&& arg) const;
        primitive_argument_type l2_normalize2d_flatten(arg_type&& arg) const;
        primitive_argument_type l2_normalize2d(
            arg_type&& arg, std::int64_t axis) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type l2_normalize3d_axis0(arg_type&& arg) const;
        primitive_argument_type l2_normalize3d_axis1(arg_type&& arg) const;
        primitive_argument_type l2_normalize3d_axis2(arg_type&& arg) const;
        primitive_argument_type l2_normalize3d_flatten(arg_type&& arg) const;
        primitive_argument_type l2_normalize3d(
            arg_type&& arg, std::int64_t axis) const;
#endif
    };
    inline primitive create_l2_normalize_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "l2_normalize", std::move(operands), name, codename);
    }
}}}

#endif

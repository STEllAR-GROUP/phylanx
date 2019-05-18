// Copyright (c) 2019 Stevn R. Brandt
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_KERAS_SUPPORT_CATEGORICAL_CROSSENTROPY_OPERATION)
#define PHYLANX_PLUGINS_KERAS_SUPPORT_CATEGORICAL_CROSSENTROPY_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx {  namespace execution_tree {  namespace primitives  {

    class cat_cross_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<cat_cross_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
        using arg_type = ir::node_data<double>;

    public:
        static match_pattern_type const match_data;

        cat_cross_operation() = default;

        cat_cross_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type cat_cross0d(
            arg_type&& target, arg_type&& output, bool from_logbits) const;
        primitive_argument_type cat_cross1d(
            arg_type&& target, arg_type&& output, bool from_logbits) const;
        primitive_argument_type cat_cross2d(
            arg_type&& target, arg_type&& output,
            bool from_logbits,int axis) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type cat_cross3d(
            arg_type&& target, arg_type&& output,
            bool from_logbits,int axis) const;
#endif
    };
    inline primitive create_cat_cross_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality,
            "categorical_crossentropy",
            std::move(operands), name, codename);
    }
}}}

#endif

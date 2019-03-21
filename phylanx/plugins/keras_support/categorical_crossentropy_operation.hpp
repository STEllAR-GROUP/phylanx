// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
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
/// \param a      The scalar, vector, or matrix to perform categorical_crossentropy over
/// \param axis   Optional. The default is the last axis (axis == -1). Effective
///               when the array is >1d

    class categorical_crossentropy_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<categorical_crossentropy_operation>
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

        categorical_crossentropy_operation() = default;

        categorical_crossentropy_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type categorical_crossentropy0d() const;
        primitive_argument_type categorical_crossentropy1d(
            arg_type&& target, arg_type&& output, bool from_logbits) const;
        primitive_argument_type categorical_crossentropy2d(
            arg_type&& target, arg_type&& output, bool from_logbits) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type categorical_crossentropy3d(
            arg_type&& target, arg_type&& output, bool from_logbits) const;
#endif
    };
    inline primitive create_categorical_crossentropy_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "categorical_crossentropy", std::move(operands), name, codename);
    }
}}}

#endif

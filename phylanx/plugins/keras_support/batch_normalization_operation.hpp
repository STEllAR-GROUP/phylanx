// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_KERAS_SUPPORT_BATCH_NORMALIZATION)
#define PHYLANX_PRIMITIVES_KERAS_SUPPORT_BATCH_NORMALIZATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief Applies batch normalization on x given mean, var, beta and gamma.
/// \param x       The 4d array (the quaternion)
/// \param mean    Mean of batch
/// \param var     Variance of batch
/// \param beta    Tensor by which to center the input
/// \param gamma   Tensor by which to scale the input
/// \param axis    Optional. Integer, the axis that should be normalized
/// \param epsilon Optional. Fuzz factor

    class batch_normalization_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<batch_normalization_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        batch_normalization_operation() = default;

        batch_normalization_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        //primitive_argument_type spatial_padding(
        //    ir::node_data<double>&& arg) const;
        //primitive_argument_type spatial_padding(ir::node_data<double>&& arg,
        //    std::size_t pad_front, std::size_t pad_rear, std::size_t pad_top,
        //    std::size_t pad_bottom) const;
    };
    inline primitive create_batch_normalization_operation(
        hpx::id_type const& locality, primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(locality, "batch_normalization",
            std::move(operands), name, codename);
    }
}}}

#endif

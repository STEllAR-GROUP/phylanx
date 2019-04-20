// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_CONV1D_OPERATION)
#define PHYLANX_KERAS_SUPPORT_CONV1D_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief returns 1D convoltion
/// \param x              a vector
/// \param kernel         a vector, the filter
/// \param padding        Padding mode, either `valid`, `same` or `causal`
/// \param strides        The step to apply convolution
/// \param dilation_rate  The rate to sample x in each step

    class conv1d_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<conv1d_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        conv1d_operation() = default;

        conv1d_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename Vector1, typename Vector2>
        double convolve_step(const Vector1& v1, const Vector2& v2) const;
        template <typename Vector1, typename Vector2>
        double convolve_step(const Vector1& v1, const Vector2& v2,
            std::int64_t dilation_rate, std::size_t kernel_size) const;
        template <typename Vector1, typename Vector2>
        double convolve_step(const Vector1& v1, const Vector2& v2,
            std::int64_t dilation_rate, std::size_t kernel_size,
            std::size_t r) const;

        primitive_argument_type conv1d_valid(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel) const;
        primitive_argument_type conv1d_valid(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::int64_t strides) const;
        primitive_argument_type conv1d_valid_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::int64_t dilation_rate) const;

        primitive_argument_type conv1d_same(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel) const;
        primitive_argument_type conv1d_same(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::int64_t strides) const;
        primitive_argument_type conv1d_same_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::int64_t dilation_rate) const;

        primitive_argument_type conv1d_causal(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel) const;
        primitive_argument_type conv1d_causal(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::int64_t strides) const;
        primitive_argument_type conv1d_causal_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::int64_t dilation_rate) const;

        primitive_argument_type conv1d_any_pad(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::string&& padding) const;
        primitive_argument_type conv1d_any_pad(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::string&& padding,
            std::int64_t strides) const;
        primitive_argument_type conv1d_any_pad_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::string&& padding, std::int64_t dilation_rate) const;
    };

    inline primitive create_conv1d_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "conv1d", std::move(operands), name, codename);
    }
}}}

#endif

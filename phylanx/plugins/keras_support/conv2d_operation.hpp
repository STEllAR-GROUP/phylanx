// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_CONV2D_OPERATION)
#define PHYLANX_KERAS_SUPPORT_CONV2D_OPERATION

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
/// \brief returns 2D convoltion
/// \param x              a matrix
/// \param kernel         a matrix, the filter
/// \param padding        Padding mode, either `valid` or `same`
/// \param strides        The step to apply convolution on each dimension
/// \param dilation_rate  The rate to sample x in each step

    class conv2d_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<conv2d_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        conv2d_operation() = default;

        conv2d_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename Matrix1, typename Matrix2>
        double convolve_step(const Matrix1& m1, const Matrix2& m2) const;
        template <typename Matrix1, typename Matrix2>
        double convolve_step(const Matrix1& m1, const Matrix2& m2,
            std::size_t dilation_rate_r, std::size_t dilation_rate_c,
            std::size_t kernel_rows, std::size_t kernel_columns) const;
        template <typename Vector1, typename Vector2>
        double convolve_step(const Vector1& v1, const Vector2& v2,
            std::int64_t dilation_rate, std::size_t kernel_size,
            std::size_t r) const;

        primitive_argument_type conv2d_valid(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel) const;
        primitive_argument_type conv2d_valid(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, ir::range&& strides) const;
        primitive_argument_type conv2d_valid_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            ir::range&& dilation_rate) const;

        primitive_argument_type conv2d_same(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel) const;
        primitive_argument_type conv2d_same(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, ir::range&& strides) const;
        primitive_argument_type conv2d_same_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            ir::range&& dilation_rate) const;


        primitive_argument_type conv2d_any_pad(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::string&& padding) const;
        primitive_argument_type conv2d_any_pad(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::string&& padding,
            ir::range&& strides) const;
        primitive_argument_type conv2d_any_pad_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::string&& padding, ir::range&& dilation_rate) const;
    };

    inline primitive create_conv2d_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "conv2d", std::move(operands), name, codename);
    }
}}}

#endif

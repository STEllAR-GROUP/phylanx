// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_CONV2D_TRANSPOSE_OPERATION)
#define PHYLANX_KERAS_SUPPORT_CONV2D_TRANSPOSE_OPERATION

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
/// \param output_shape   a tuple of two integers indicating output shape
/// \param padding        Padding mode, either `valid` or `same`
/// \param strides        The step to apply convolution on each dimension
/// \param dilation_rate  The rate to sample x in each step

    class conv2d_transpose_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<conv2d_transpose_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        conv2d_transpose_operation() = default;

        conv2d_transpose_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        blaze::DynamicMatrix<double> flip_filter(
            ir::node_data<double>&& kernel) const;
        template <typename Matrix1, typename Matrix2>
        double convolve_step(const Matrix1& m1, const Matrix2& m2) const;
        template <typename Matrix1, typename Matrix2>
        double convolve_step(const Matrix1& m1, const Matrix2& m2,
            std::size_t dilation_rate_r, std::size_t dilation_rate_c,
            std::size_t kernel_rows, std::size_t kernel_columns) const;
        template <typename Matrix1, typename Matrix2>
        double convolve_step(const Matrix1& m1, const Matrix2& m2,
            std::size_t dilation_height, std::size_t dilation_width,
            std::size_t kernel_rows, std::size_t kernel_columns,
            std::size_t r_remainder, std::size_t c_remainder) const;

        primitive_argument_type conv2d_trans_valid(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, ir::range&& output_shape) const;
        primitive_argument_type conv2d_trans_same(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, ir::range&& output_shape) const;

        //primitive_argument_type conv2d_trans_strides(
        //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        //    ir::range&& output_shape, ir::range&& strides) const;
        primitive_argument_type conv2d_trans_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            ir::range&& output_shape, ir::range&& dilation_rate) const;

        primitive_argument_type conv2d_trans_any_pad(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            ir::range&& output_shape, std::string&& padding) const;
    };

    inline primitive create_conv2d_transpose_operation(
        hpx::id_type const& locality, primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "conv2d_transpose", std::move(operands), name, codename);
    }
}}}

#endif

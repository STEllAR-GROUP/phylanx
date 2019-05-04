// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_POOL_OPERATION)
#define PHYLANX_KERAS_SUPPORT_POOL_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief Pools out the value with each stride of a pool_size filter
/// \param x         The matrix or tensor to pool information out of it
/// \param pool_size The size of pooling oevr each dimension
/// \param padding   Padding mode, either `same` or `valid`
/// \param stride    The step to apply pooling on each dimension

    class pool_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<pool_operation>
    {
    public:
        enum pool_mode
        {
            max_pool,
            avg_pool
        };

    protected:
        using val_type = std::int64_t;
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static std::vector<match_pattern_type> const match_data;

        pool_operation() = default;

        pool_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        bool validate_pooling(
            std::size_t const& ndim, ir::range const& pool_size) const;
        bool validate_pool_sizes_no_padding(
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims,
            ir::range const& pool_size) const;
        bool validate_strides(
            std::size_t const& ndim, ir::range& strides) const;

        template <typename Tensor>
        double mean(const Tensor& t) const;

        template <typename T>
        primitive_argument_type max_pool2d(ir::node_data<T>&& arg,
            ir::range&& pool_size) const;
        primitive_argument_type avg_pool2d(ir::node_data<double>&& arg,
            ir::range&& pool_size) const;
        template <typename T>
        primitive_argument_type max_pool2d(ir::node_data<T>&& arg,
            ir::range&& pool_size, ir::range&& strides) const;
        primitive_argument_type avg_pool2d(ir::node_data<double>&& arg,
            ir::range&& pool_size, ir::range&& strides) const;

        template <typename T>
        primitive_argument_type max_pool2d_with_pad(ir::node_data<T>&& arg,
            ir::range&& pool_size) const;
        primitive_argument_type avg_pool2d_with_pad(ir::node_data<double>&& arg,
            ir::range&& pool_size) const;
        template <typename T>
        primitive_argument_type max_pool2d_with_pad(ir::node_data<T>&& arg,
            ir::range&& pool_size, ir::range&& strides) const;
        primitive_argument_type avg_pool2d_with_pad(ir::node_data<double>&& arg,
            ir::range&& pool_size, ir::range&& strides) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type max_pool3d(ir::node_data<T>&& arg,
            ir::range&& pool_size) const;
        primitive_argument_type avg_pool3d(ir::node_data<double>&& arg,
            ir::range&& pool_size) const;
        template <typename T>
        primitive_argument_type max_pool3d(ir::node_data<T>&& arg,
            ir::range&& pool_size, ir::range&& strides) const;
        primitive_argument_type avg_pool3d(ir::node_data<double>&& arg,
            ir::range&& pool_size, ir::range&& strides) const;

        template <typename T>
        primitive_argument_type max_pool3d_with_pad(ir::node_data<T>&& arg,
            ir::range&& pool_size) const;
        primitive_argument_type avg_pool3d_with_pad(ir::node_data<double>&& arg,
            ir::range&& pool_size) const;
        template <typename T>
        primitive_argument_type max_pool3d_with_pad(ir::node_data<T>&& arg,
            ir::range&& pool_size, ir::range&& strides) const;
        primitive_argument_type avg_pool3d_with_pad(ir::node_data<double>&& arg,
            ir::range&& pool_size, ir::range&& strides) const;
#endif

        template <typename T>
        primitive_argument_type max_pool_nd(ir::node_data<T>&& arg,
            ir::range&& pool_size, std::string&& padding) const;
        primitive_argument_type avg_pool_nd(ir::node_data<double>&& arg,
            ir::range&& pool_size, std::string&& padding) const;
        template <typename T>
        primitive_argument_type max_pool_nd(ir::node_data<T>&& arg,
            ir::range&& pool_size, std::string&& padding,
            ir::range&& strides) const;
        primitive_argument_type avg_pool_nd(ir::node_data<double>&& arg,
            ir::range&& pool_size, std::string&& padding,
            ir::range&& strides) const;

    private:
        pool_mode mode_;
    };

    inline primitive create_pool_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "max_pool", std::move(operands), name, codename);
    }
    inline primitive create_avg_pool_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "avg_pool", std::move(operands), name, codename);
    }
}}}

#endif

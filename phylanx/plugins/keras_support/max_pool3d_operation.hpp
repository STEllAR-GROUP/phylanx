// Copyright (c) 3019 Bita Hasheminezhad
// Copyright (c) 3019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_MAX_POOL3D_OPERATION)
#define PHYLANX_KERAS_SUPPORT_MAX_POOL3D_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/futures/future.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief Pools out the value with each stride of a pool_size filter
/// \param x         The tensor to pool information out of it
/// \param pool_size The size of pooling over each dimension
/// \param padding   Padding mode, either `same` or `valid`
/// \param strides   The step to apply pooling on each dimension

    class max_pool3d_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<max_pool3d_operation>
    {
    protected:
        using val_type = std::int64_t;
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        max_pool3d_operation() = default;

        max_pool3d_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type max_pool3d(ir::node_data<double>&& arg,
            std::size_t filter_depth, std::size_t filter_height,
            std::size_t filter_width) const;

        primitive_argument_type max_pool3d(ir::node_data<double>&& arg,
            std::size_t filter_depth, std::size_t filter_height,
            std::size_t filter_width, std::size_t stride_depth,
            std::size_t stride_height, std::size_t stride_width) const;

        primitive_argument_type max_pool3d_same(ir::node_data<double>&& arg,
            std::size_t filter_depth, std::size_t filter_height,
            std::size_t filter_width) const;

        primitive_argument_type max_pool3d_same(ir::node_data<double>&& arg,
            std::size_t filter_depth, std::size_t filter_height,
            std::size_t filter_width, std::size_t stride_depth,
            std::size_t stride_height, std::size_t stride_width) const;

        primitive_argument_type max_pool_any_pad(ir::node_data<double>&& arg,
            std::size_t filter_depth, std::size_t filter_height,
            std::size_t filter_width, std::string&& padding) const;
        primitive_argument_type max_pool_any_pad(ir::node_data<double>&& arg,
            std::size_t filter_depth, std::size_t filter_height,
            std::size_t filter_width, std::string&& padding,
            std::size_t stride_depth, std::size_t stride_height,
            std::size_t stride_width) const;
    };

    inline primitive create_max_pool3d_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "max_pool3d", std::move(operands), name, codename);
    }
}}}

#endif

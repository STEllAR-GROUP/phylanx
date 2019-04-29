// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_KERAS_SUPPORT_RESIZE_OPERATION)
#define PHYLANX_PLUGINS_KERAS_SUPPORT_RESIZE_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Returns resized image with hight_factor and width_factor
    ///
    /// \param height_factor  Scalar, resizing factor for height
    /// \param width_factor  Scalar, resizing factor for width
    /// \param interpolation String, interpolation type "nearest" or "bilinear"
    class resize_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<resize_operation>
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

        resize_operation() = default;

        resize_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type nearest(ir::node_data<T>&& arg,
            std::int64_t height_factor, std::int64_t width_factor,
            std::string interpolation) const;
        primitive_argument_type bilinear(ir::node_data<double>&& arg,
            std::int64_t height_factor, std::int64_t width_factor,
            std::string interpolation) const;
    };

    inline primitive create_resize_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "resize_images", std::move(operands), name, codename);
    }
}}}

#endif

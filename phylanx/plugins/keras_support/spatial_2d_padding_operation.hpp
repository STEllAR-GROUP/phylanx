// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_KERAS_SUPPORT_SPATIAL_2D_PADDING)
#define PHYLANX_PRIMITIVES_KERAS_SUPPORT_SPATIAL_2D_PADDING

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/futures/future.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief Pads the 2nd and 3rd dimensions of a 4D array with zeros
/// \param x        The 4d array (the quaternion) to pad
/// \param padding  Optional. The default is ((1,1),(1,1)) which means to pad
///                 1 page in the front, 1 page in the rear, 1 row on the top
///                 and 1 row on the buttom respectively.

    class spatial_2d_padding_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<spatial_2d_padding_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        spatial_2d_padding_operation() = default;

        spatial_2d_padding_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type spatial_padding(
            ir::node_data<double>&& arg) const;
        primitive_argument_type spatial_padding(ir::node_data<double>&& arg,
            std::size_t pad_front, std::size_t pad_rear, std::size_t pad_top,
            std::size_t pad_bottom) const;
    };
    inline primitive create_spatial_2d_padding_operation(
        hpx::id_type const& locality, primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(locality, "spatial_2d_padding",
            std::move(operands), name, codename);
    }
}}}

#endif

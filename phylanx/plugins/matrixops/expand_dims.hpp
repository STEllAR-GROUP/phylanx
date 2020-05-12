// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
// Copyright (c) 2018-2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_DIM)
#define PHYLANX_PRIMITIVES_ADD_DIM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
/// \brief Expands the shape of an array by adding a dimension along the given
///        axis.
/// \param a     The scalar, vector, or matrix to be expanded.
/// \param axis  The axis which the dimension is expanded along.

    class expand_dims
      : public primitive_component_base
      , public std::enable_shared_from_this<expand_dims>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        expand_dims() = default;

        expand_dims(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type expand_dims_0d(ir::node_data<T>&& arg) const;
        primitive_argument_type expand_dims_0d(
            primitive_arguments_type&& arg) const;

        template <typename T>
        primitive_argument_type expand_dims_1d(
            ir::node_data<T>&& arg, std::int64_t axis) const;
        template <typename T>
        primitive_argument_type expand_dims_1d(ir::node_data<T>&& arg,
            std::int64_t axis, localities_information&& arr_localities) const;
        primitive_argument_type expand_dims_1d(
            primitive_arguments_type&& arg) const;

        template <typename T>
        primitive_argument_type expand_dims_2d(
            ir::node_data<T>&& arg, std::int64_t axis) const;
        primitive_argument_type expand_dims_2d(
            primitive_arguments_type&& arg) const;

        template <typename T>
        primitive_argument_type expand_dims_3d(
            ir::node_data<T>&& arg, std::int64_t axis) const;
        primitive_argument_type expand_dims_3d(
            primitive_arguments_type&& arg) const;
    };

    inline primitive create_expand_dims(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "expand_dims", std::move(operands), name, codename);
    }
}}}

#endif

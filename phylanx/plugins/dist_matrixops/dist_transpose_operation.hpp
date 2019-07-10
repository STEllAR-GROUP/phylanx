// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_TRANSPOSE_OPERATION_OCT_09_2017_0146PM)
#define PHYLANX_PRIMITIVES_TRANSPOSE_OPERATION_OCT_09_2017_0146PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives
{
    class dist_transpose_operation
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<dist_transpose_operation>
    {
    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

    public:
        static execution_tree::match_pattern_type const match_data;

        dist_transpose_operation() = default;

        dist_transpose_operation(
            execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        bool validate_axes(std::size_t a_dims,
            ir::node_data<std::int64_t>&& axes) const;

        execution_tree::primitive_argument_type transpose1d(
            execution_tree::primitive_argument_type&& arg) const;
        execution_tree::primitive_argument_type transpose2d(
            execution_tree::primitive_argument_type&& arg) const;
        execution_tree::primitive_argument_type transpose2d(
            execution_tree::primitive_argument_type&& arg,
            ir::node_data<std::int64_t>&& axes) const;

        template <typename T>
        execution_tree::primitive_argument_type transpose2d(
            ir::node_data<T>&& arg,
            execution_tree::localities_information&& localities) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        execution_tree::primitive_argument_type transpose3d(
            execution_tree::primitive_argument_type&& arg) const;
        execution_tree::primitive_argument_type transpose3d(
            execution_tree::primitive_argument_type&& arg,
            ir::node_data<std::int64_t>&& axes) const;

        template <typename T>
        execution_tree::primitive_argument_type transpose3d(
            ir::node_data<T>&& arg,
            execution_tree::localities_information&& localities) const;
        template <typename T>
        execution_tree::primitive_argument_type transpose3d(
            ir::node_data<T>&& arg, ir::node_data<std::int64_t>&& axes,
            execution_tree::localities_information&& localities) const;
#endif
    };

    inline execution_tree::primitive
    create_dist_transpose_operation(hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return execution_tree::create_primitive_component(
            locality, "transpose_d", std::move(operands), name, codename);
    }
}}}

#endif

// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_TRANSPOSE_OPERATION_OCT_09_2017_0146PM)
#define PHYLANX_PRIMITIVES_TRANSPOSE_OPERATION_OCT_09_2017_0146PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class transpose_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<transpose_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        transpose_operation() = default;

        transpose_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        bool validate_axes(std::size_t a_dims,
            ir::node_data<std::int64_t>&& axes) const;

        primitive_argument_type transpose0d1d(primitive_argument_type&& arg) const;
        primitive_argument_type transpose2d(primitive_argument_type && arg) const;
        primitive_argument_type transpose2d(primitive_argument_type&& arg,
            ir::node_data<std::int64_t>&& axes) const;

        template <typename T>
        primitive_argument_type transpose2d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type transpose2d(
            ir::node_data<T>&& arg, ir::node_data<std::int64_t>&& axes) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type transpose3d(
            primitive_argument_type&& arg) const;
        primitive_argument_type transpose3d(primitive_argument_type&& arg,
            ir::node_data<std::int64_t>&& axes) const;

        template <typename T>
        primitive_argument_type transpose3d_axes102(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type transpose3d_axes021(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type transpose3d_axes120(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type transpose3d_axes201(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type transpose3d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type transpose3d(
            ir::node_data<T>&& arg, ir::node_data<std::int64_t>&& axes) const;
#endif
    };

    inline primitive create_transpose_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "transpose", std::move(operands), name, codename);
    }
}}}

#endif

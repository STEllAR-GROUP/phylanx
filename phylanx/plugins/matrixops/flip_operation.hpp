// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIXOPS_FLIP_OPERATION)
#define PHYLANX_MATRIXOPS_FLIP_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    /// \brief Reverses the order of elements in an array along the given axis
    /// \param a         The scalar, vector, or matrix to flip
    /// \param axis      Optional. Flip is calculated along the provided axis
    ///                  and an array of the same shape is returned.
    class flip_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<flip_operation>
    {
    protected:
        using val_type = std::int64_t;
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        flip_operation() = default;

        flip_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type flip1d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip1d(ir::node_data<T>&& arg,
            ir::range&& axes) const;

        template <typename T>
        primitive_argument_type flip2d_axis0(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip2d_axis1(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip2d_both_axes(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip2d(ir::node_data<T>&& arg,
            ir::range&& axes) const;

        template <typename T>
        primitive_argument_type flipnd(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flipnd(ir::node_data<T>&& arg,
            ir::range&& axes) const;
    };

    inline primitive create_flip_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "flip", std::move(operands), name, codename);
    }
}}}

#endif

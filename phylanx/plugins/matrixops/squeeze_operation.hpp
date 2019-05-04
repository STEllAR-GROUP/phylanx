// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_MATRIXOPS_SQUEEZE_OPERATION)
#define PHYLANX_PLUGINS_MATRIXOPS_SQUEEZE_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
/// \brief squeeze removes single-dimensional entries from the shape of an
///        array.
/// \param a         The scalar, vector, or matrix to perform squeeze over
/// \param axis      Optional. If provided, squeeze is calculated along the
///                  provided axis for >2d arrays.

    class squeeze_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<squeeze_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        squeeze_operation() = default;

        squeeze_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type squeeze0d(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> axis) const;
        primitive_argument_type squeeze1d(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> axis) const;
        primitive_argument_type squeeze2d(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> axis) const;

        template <typename T>
        primitive_argument_type squeeze1d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze2d_axis0(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze2d_axis1(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze2d_all_axes(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze2d(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type squeeze3d(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> axis) const;

        template <typename T>
        primitive_argument_type squeeze3d_axis0(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze3d_axis1(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze3d_axis2(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze3d_all_axes(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze3d(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis) const;
#endif

    };
    inline primitive create_squeeze_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "squeeze", std::move(operands), name, codename);
    }
}}}

#endif

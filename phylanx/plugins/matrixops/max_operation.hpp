// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIXOPS_MAX_OPERATION)
#define PHYLANX_MATRIXOPS_MAX_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    /// \brief
    /// \param a         The scalar, vector, or matrix to perform max over
    /// \param axis      Optional. If provided, max is calculated along the
    ///                  provided axis and a vector of results is returned.
    ///                  \p keep_dims is ignored if \p axis present. Must be
    ///                  nil if \p keep_dims is set
    /// \param keep_dims Optional. Whether the max value has to have the same
    ///                  number of dimensions as \p a. Ignored if \p axis is
    ///                  anything except nil.


    class max_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<max_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        max_operation() = default;

        max_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type max0d(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis) const;
        template <typename T>
        primitive_argument_type max1d(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;
        template <typename T>
        primitive_argument_type max2d(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;
        template <typename T>
        primitive_argument_type max2d_flat(
            ir::node_data<T>&& arg, bool keep_dims) const;
        template <typename T>
        primitive_argument_type max2d_axis0(
            ir::node_data<T>&& arg, bool keep_dims) const;
        template <typename T>
        primitive_argument_type max2d_axis1(
            ir::node_data<T>&& arg, bool keep_dims) const;
        template <typename T>
        primitive_argument_type maxnd(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;

    private:
        node_data_type dtype_;
    };

    inline primitive create_max_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "max", std::move(operands), name, codename);
    }
}}}

#endif

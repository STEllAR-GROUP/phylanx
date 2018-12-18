// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_PROD)
#define PHYLANX_PRIMITIVES_PROD

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
    /// \brief products the values of the elements of a vector or a matrix or
    ///        returns the value of the scalar that was given to it.
    /// \param a         The scalar, vector, or matrix to perform prod over
    /// \param axis      Optional. If provided, prod is calculated along the
    ///                  provided axis and a vector of results is returned.
    ///                  \p keep_dims is ignored if \p axis present. Must be
    ///                  nil if \p keep_dims is set
    /// \param keep_dims Optional. Whether the prod value has to have the same
    ///                  number of dimensions as \p a. Ignored if \p axis is
    ///                  anything except nil.
    /// This implementation is intended to behave like [NumPy implementation of prod]
    /// (https://docs.scipy.org/doc/numpy-1.15.0/reference/generated/numpy.prod.html).

    class prod_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<prod_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        prod_operation() = default;

        prod_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type prod0d(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;
        template <typename T>
        primitive_argument_type prod1d(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;
        template <typename T>
        primitive_argument_type prod2d(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;
        template <typename T>
        primitive_argument_type prod2d_flat(
            ir::node_data<T>&& arg, bool keep_dims) const;
        template <typename T>
        primitive_argument_type prod2d_axis0(
            ir::node_data<T>&& arg, bool keep_dims) const;
        template <typename T>
        primitive_argument_type prod2d_axis1(
            ir::node_data<T>&& arg, bool keep_dims) const;
        template <typename T>
        primitive_argument_type prodnd(ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;

        node_data_type dtype_;
    };

    inline primitive create_prod_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "prod", std::move(operands), name, codename);
    }
}}}

#endif

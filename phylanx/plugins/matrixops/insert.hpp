//   Copyright (c) 2019 Shahrzad Shirzad
//   Copyright (c) 2019 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_INSERT_FEB_04_2019_0243PM)
#define PHYLANX_PRIMITIVES_INSERT_FEB_04_2019_0243PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    /// \brief Implementation of insert as a Phylanx primitive.
    /// Returns a copy of the array with values inserted.
    /// This implementation is intended to behave like [NumPy implementation of flatten]
    /// (https://docs.scipy.org/doc/numpy-1.15.0/reference/generated/numpy.insert.html).
    /// \param a      It may be a scalar value, vector, matrix or tensor
    /// \param obj    int, slice, or sequence of ints
    /// \param values It may be a scalar value, vector, matrix or tensor
    /// \param axis   (optional)int

    class insert
      : public primitive_component_base
      , public std::enable_shared_from_this<insert>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        insert() = default;

        insert(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename);

    private:
        template <typename T>
        static std::size_t get_vector_size(ir::node_data<T> const& arg);

        template <typename T>
        primitive_argument_type flatten_nd_helper(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;

        template <typename T>
        primitive_argument_type insert_flatten_0d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;

        template <typename T>
        primitive_argument_type insert_flatten_2d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;

        template <typename T>
        primitive_argument_type insert_2d_axis0(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;
        template <typename T>
        primitive_argument_type insert_2d_axis1(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;

        template <typename T>
        primitive_argument_type insert_flatten_3d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;

        template <typename T>
        primitive_argument_type insert_3d_axis0(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;
        template <typename T>
        primitive_argument_type insert_3d_axis1(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;

        template <typename T>
        primitive_argument_type insert_3d_axis2(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;

        template <typename T>
        primitive_argument_type insert_3d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
            std::int64_t axis) const;

        template <typename T>
        primitive_argument_type insert_flatten_nd(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices,
            ir::node_data<T>&& values) const;

        template <typename T>
        primitive_argument_type insert_1d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
            std::int64_t axis) const;
        template <typename T>
        primitive_argument_type insert_2d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
            std::int64_t axis) const;

        template <typename T>
        primitive_argument_type insert_nd(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
            hpx::util::optional<std::int64_t> axis) const;
    };

    inline primitive create_insert(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "insert", std::move(operands), name, codename);
    }
}}}

#endif

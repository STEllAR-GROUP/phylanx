//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_sort)
#define PHYLANX_PRIMITIVES_sort

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
    /// \brief Implementation of sort as a Phylanx primitive.
    /// Returns the sorted array.
    /// This implementation is intended to behave like [NumPy implementation of sort]
    /// (https://docs.scipy.org/doc/numpy-1.15.0/reference/generated/numpy.sort.html).
    /// \param a an array
    /// \param axis axis along which to sort. If None, array is flattened before sorting.
    /// \param kind sorting algorithm
    /// \paramoredr specifies which fields to compare first, second, etc.

    class sort
      : public primitive_component_base
      , public std::enable_shared_from_this<sort>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        sort() = default;

        sort(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type sort1d(
            ir::node_data<T>&& arg, std::int64_t axis, std::string kind) const;
        template <typename T>
        primitive_argument_type sort2d_axis0(
            ir::node_data<T>&& arg, std::string kind) const;
        template <typename T>
        primitive_argument_type sort2d_axis1(
            ir::node_data<T>&& arg, std::string kind) const;
        template <typename T>
        primitive_argument_type sort2d(
            ir::node_data<T>&& arg, std::int64_t axis, std::string kind) const;
        template <typename T>
        primitive_argument_type sort3d_axis0(
            ir::node_data<T>&& arg, std::string kind) const;
        template <typename T>
        primitive_argument_type sort3d_axis1(
            ir::node_data<T>&& arg, std::string kind) const;
        template <typename T>
        primitive_argument_type sort3d_axis2(
            ir::node_data<T>&& arg, std::string kind) const;
        template <typename T>
        primitive_argument_type sort3d(
            ir::node_data<T>&& arg, std::int64_t axis, std::string kind) const;
        template <typename T>
        primitive_argument_type sort_flatten2d(
            ir::node_data<T>&& arg, std::string kind) const;
        template <typename T>
        primitive_argument_type sort_flatten3d(
            ir::node_data<T>&& arg, std::string kind) const;
        template <typename T>
        primitive_argument_type sort_flatten_helper(
            ir::node_data<T>&& arg, std::string kind) const;

        primitive_argument_type sort_flatten(
            primitive_argument_type&& arg, std::string kind) const;
        template <typename T>
        primitive_argument_type sort_helper(
            ir::node_data<T>&& arg, std::int64_t axis, std::string kind) const;
    };

    inline primitive create_sort(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "sort", std::move(operands), name, codename);
    }
}}}

#endif

//   Copyright (c) 2018 Hartmut Kaiser
//   Copyright (c) 2019 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CUMSUM_OCT_03_2018_0953AM)
#define PHYLANX_PRIMITIVES_CUMSUM_OCT_03_2018_0953AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        struct statistics_cumsum_op;
    }

    /// \brief FIXIT:Sums the values of the elements of a vector or a matrix or
    ///        returns the value of the scalar that was given to it.
    /// \param a         The scalar, vector, or matrix to perform sum over
    /// \param axis      Optional. If provided, sum is calculated along the
    ///                  provided axis and a vector of results is returned.
    ///                  \p keep_dims is ignored if \p axis present. Must be
    ///                  nil if \p keep_dims is set
    /// \param keep_dims Optional. Whether the sum value has to have the same
    ///                  number of dimensions as \p a. Ignored if \p axis is
    ///                  anything except nil.

    class cumsum_operation
      : public statistics<detail::statistics_cumsum_op, cumsum_operation>
    {
        using base_type =
            statistics<detail::statistics_cumsum_op, cumsum_operation>;

    public:
        static match_pattern_type const match_data;

        cumsum_operation() = default;

        cumsum_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    public:
        template <typename T>
        primitive_argument_type statistics0d(arg_type<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            hpx::util::optional<T> const& initial) const;

        template <typename T>
        primitive_argument_type statistics1d(arg_type<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            hpx::util::optional<T> const& initial) const;

        template <typename T>
        primitive_argument_type statistics2d_flat(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<T> const& initial) const;

        template <typename T>
        primitive_argument_type statistics2d_axis0(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<T> const& initial) const;

        template <typename T>
        primitive_argument_type statistics2d_axis1(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<T> const& initial) const;
    };

    inline primitive create_cumsum_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "cumsum", std::move(operands), name, codename);
    }
}}}

#endif

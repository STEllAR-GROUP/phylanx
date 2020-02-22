// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_STATISTICS_DEC_24_2018_0227PM)
#define PHYLANX_PRIMITIVES_STATISTICS_DEC_24_2018_0227PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    template <template <class T> class Op, typename Derived>
    class statistics
      : public primitive_component_base
      , public std::enable_shared_from_this<Derived>
    {
    private:
        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }
        Derived const& derived() const
        {
            return static_cast<Derived const&>(*this);
        }

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        template <typename T>
        using arg_type = ir::node_data<T>;

        template <typename T>
        using args_type =
            std::vector<arg_type<T>, arguments_allocator<arg_type<T>>>;

    public:
        statistics() = default;

        statistics(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename);

    //private:
        template <typename T, typename Init>
        primitive_argument_type statistics0d(arg_type<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics1d(arg_type<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics2d(arg_type<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics2d_flat(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics2d_axis0(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics2d_axis1(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics3d(arg_type<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics3d_flat(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics3d_axis0(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics3d_axis1(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics3d_axis2(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics3d_columnslice(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics3d_rowslice(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics3d_pageslice(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T>
        primitive_argument_type statistics3d_slice(arg_type<T>&& arg,
            std::int64_t axis0, std::int64_t axis1, bool keepdims,
            primitive_argument_type&& initial) const;

        primitive_argument_type statistics3d_slice(
            primitive_argument_type&& arg, std::int64_t axis0,
            std::int64_t axis1, bool keepdims,
            primitive_argument_type&& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics4d(arg_type<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics4d_flat(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_axis0(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_axis1(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_axis2(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_axis3(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics4d_slice01(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_slice02(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_slice03(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_slice12(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_slice13(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_slice23(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;

        template <typename T>
        primitive_argument_type statistics4d_slice(arg_type<T>&& arg,
            std::int64_t axis0, std::int64_t axis1, bool keepdims,
            primitive_argument_type&& initial) const;

        primitive_argument_type statistics4d_slice(
            primitive_argument_type&& arg, std::int64_t axis0,
            std::int64_t axis1, bool keepdims,
            primitive_argument_type&& initial) const;

        template <typename T, typename Init>
        primitive_argument_type statistics4d_tensor012(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_tensor013(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_tensor023(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T, typename Init>
        primitive_argument_type statistics4d_tensor123(arg_type<T>&& arg,
            bool keepdims, hpx::util::optional<Init> const& initial) const;
        template <typename T>
        primitive_argument_type statistics4d_tensor(arg_type<T>&& arg,
            std::int64_t axis0, std::int64_t axis1, std::int64_t axis2,
            bool keepdims, primitive_argument_type&& initial) const;

        primitive_argument_type statistics4d_tensor(
            primitive_argument_type&& arg, std::int64_t axis0,
            std::int64_t axis1, std::int64_t axis2, bool keepdims,
            primitive_argument_type&& initial) const;

        template <typename T>
        primitive_argument_type statisticsnd_flat(arg_type<T>&& arg,
            bool keepdims, primitive_argument_type&& initial) const;

        template <typename T>
        primitive_argument_type statisticsnd(arg_type<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis,
            bool keepdims, primitive_argument_type&& initial) const;

        primitive_argument_type statisticsnd_flat(
            primitive_argument_type&& arg, bool keepdims,
            primitive_argument_type&& initial) const;

        primitive_argument_type statisticsnd(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> const& axis,
            bool keepdims, primitive_argument_type&& initial) const;
        primitive_argument_type statisticsnd(primitive_argument_type&& arg,
            ir::range&& axes, bool keepdims,
            primitive_argument_type&& initial) const;

    private:
        node_data_type dtype_;
    };
}}}

#endif

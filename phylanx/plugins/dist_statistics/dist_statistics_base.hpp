// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_STATISTICS_2020_JUN_19_1228PM)
#define PHYLANX_PRIMITIVES_DIST_STATISTICS_2020_JUN_19_1228PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/futures/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    template <template <class T> class Op, typename Derived>
    class dist_statistics_base
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

    public:
        dist_statistics_base() = default;

        dist_statistics_base(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type statistics0d(primitive_argument_type&& arg,
            ir::range&& axes, bool keepdims, primitive_argument_type&& initial,
            node_data_type dtype, eval_context ctx) const;

        primitive_argument_type statistics1d(primitive_argument_type&& arg,
            ir::range&& axes, bool keepdims, primitive_argument_type&& initial,
            node_data_type dtype, eval_context ctx) const;

        primitive_argument_type statistics2d(primitive_argument_type&& arg,
            ir::range&& axes, bool keepdims, primitive_argument_type&& initial,
            node_data_type dtype, eval_context ctx) const;

        primitive_argument_type statistics3d(primitive_argument_type&& arg,
            ir::range&& axes, bool keepdims, primitive_argument_type&& initial,
            node_data_type dtype, eval_context ctx) const;

        primitive_argument_type statisticsnd(primitive_argument_type&& arg,
            ir::range&& axes, bool keepdims, primitive_argument_type&& initial,
            node_data_type dtype, eval_context ctx) const;

        primitive_argument_type statistics0d(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            primitive_argument_type&& initial, node_data_type dtype,
            eval_context ctx) const;

        primitive_argument_type statistics1d(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            primitive_argument_type&& initial, node_data_type dtype,
            eval_context ctx) const;

        primitive_argument_type statistics2d(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            primitive_argument_type&& initial, node_data_type dtype,
            eval_context ctx) const;

        primitive_argument_type statistics3d(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            primitive_argument_type&& initial, node_data_type dtype,
            eval_context ctx) const;

        primitive_argument_type statisticsnd(primitive_argument_type&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            primitive_argument_type&& initial, node_data_type dtype,
            eval_context ctx) const;
    };
}}}    // namespace phylanx::execution_tree::primitives

#endif

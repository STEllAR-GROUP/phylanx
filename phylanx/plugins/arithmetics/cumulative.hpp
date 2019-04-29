//   Copyright (c) 2018-2019 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CUMULATIVE_JAN_22_2019_0345PM)
#define PHYLANX_PRIMITIVES_CUMULATIVE_JAN_22_2019_0345PM

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

namespace phylanx { namespace execution_tree { namespace primitives
{
    template <typename Op, typename Derived>
    class cumulative
      : public primitive_component_base
      , public std::enable_shared_from_this<Derived>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        cumulative() = default;

        cumulative(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type cumulative_helper(
            primitive_arguments_type&& ops,
            hpx::util::optional<std::int64_t>&& axis) const;

        template <typename T>
        primitive_argument_type cumulative0d(primitive_arguments_type&& ops,
            hpx::util::optional<std::int64_t>&& axis) const;

        template <typename T>
        primitive_argument_type cumulative1d(primitive_arguments_type&& ops,
            hpx::util::optional<std::int64_t>&& axis) const;

        template <typename T>
        primitive_argument_type cumulative2d(primitive_arguments_type&& ops,
            hpx::util::optional<std::int64_t>&& axis) const;

        template <typename T>
        primitive_argument_type cumulative2d_noaxis(
            primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumulative2d_columns(
            primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumulative2d_rows(
            primitive_arguments_type&& ops) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type cumulative3d(primitive_arguments_type&& ops,
            hpx::util::optional<std::int64_t>&& axis) const;

        template <typename T>
        primitive_argument_type cumulative3d_noaxis(
            primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumulative3d_pages(
            primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumulative3d_columns(
            primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumulative3d_rows(
            primitive_arguments_type&& ops) const;
#endif
    };
}}}

#endif

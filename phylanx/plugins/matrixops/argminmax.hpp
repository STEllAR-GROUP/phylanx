// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARGMINMAX_2019_01_19_1130AM)
#define PHYLANX_PRIMITIVES_ARGMINMAX_2019_01_19_1130AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    template <typename Op, typename Derived>
    class argminmax
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
        argminmax() = default;

        argminmax(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type argminmax0d(
            primitive_arguments_type&& args) const;
        primitive_argument_type argminmax1d(
            primitive_arguments_type&& args) const;
        primitive_argument_type argminmax2d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type argminmax0d(std::size_t numargs,
            ir::node_data<T>&& args, std::int64_t axis) const;
        template <typename T>
        primitive_argument_type argminmax1d(std::size_t numargs,
            ir::node_data<T>&& args, std::int64_t axis) const;
        template <typename T>
        primitive_argument_type argminmax2d(std::size_t numargs,
            ir::node_data<T>&& args, std::int64_t axis) const;

        template <typename T>
        primitive_argument_type argminmax2d_flatten(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type argminmax2d_0_axis(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type argminmax2d_1_axis(
            ir::node_data<T>&& arg) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type argminmax3d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type argminmax3d(std::size_t numargs,
            ir::node_data<T>&& args, std::int64_t axis) const;

        template <typename T>
        primitive_argument_type argminmax3d_flatten(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type argminmax3d_0_axis(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type argminmax3d_1_axis(
            ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type argminmax3d_2_axis(
            ir::node_data<T>&& arg) const;
#endif
    };
}}}

#endif

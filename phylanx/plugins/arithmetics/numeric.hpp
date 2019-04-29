// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_NUMERIC_OCT_31_2018_0132PM)
#define PHYLANX_PRIMITIVES_NUMERIC_OCT_31_2018_0132PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    template <typename Op, typename Derived>
    class numeric
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
        numeric() = default;

        numeric(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type numeric0d0d(
            arg_type<T>&& lhs, arg_type<T>&& rhs) const;
        template <typename T>
        primitive_argument_type numeric0d0d(args_type<T> && args) const;

        template <typename T>
        primitive_argument_type numeric1d1d(
            arg_type<T>&& lhs, arg_type<T>&& rhs) const;
        template <typename T>
        primitive_argument_type numeric1d1d(args_type<T> && args) const;

        template <typename T>
        primitive_argument_type numeric2d2d(
            arg_type<T>&& lhs, arg_type<T>&& rhs) const;
        template <typename T>
        primitive_argument_type numeric2d2d(args_type<T> && args) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type numeric3d3d(
            arg_type<T>&& lhs, arg_type<T>&& rhs) const;
        template <typename T>
        primitive_argument_type numeric3d3d(args_type<T> && args) const;
#endif

    protected:
        template <typename T>
        primitive_argument_type handle_numeric_operands_helper(
            primitive_argument_type&& op1, primitive_argument_type&& op2) const;
        template <typename T>
        primitive_argument_type handle_numeric_operands_helper(
            primitive_arguments_type&& ops) const;

        primitive_argument_type handle_numeric_operands(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type handle_numeric_operands(
            primitive_arguments_type&& ops) const;

    protected:
        node_data_type dtype_;
    };
}}}

#endif



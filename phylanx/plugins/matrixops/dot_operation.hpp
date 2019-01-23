//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DOT_OPERATION_OCT_10_2017_0501PM)
#define PHYLANX_PRIMITIVES_DOT_OPERATION_OCT_10_2017_0501PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class dot_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<dot_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        dot_operation() = default;

        dot_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type dot0d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type dot1d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type dot2d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        template <typename T>
        primitive_argument_type dot0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot0d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot0d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot0d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type dot1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot1d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot1d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot1d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type dot2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot2d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot2d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot2d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type dot0d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot1d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type dot2d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        primitive_argument_type dot3d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        template <typename T>
        primitive_argument_type dot3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif
    };

    inline primitive create_dot_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "dot", std::move(operands), name, codename);
    }
}}}

#endif

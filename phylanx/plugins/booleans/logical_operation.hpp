//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Tiany Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LOGICAL_OPERATION_SEP_02_2018_0511PM)
#define PHYLANX_PRIMITIVES_LOGICAL_OPERATION_SEP_02_2018_0511PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    template <typename Op>
    class logical_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<logical_operation<Op>>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operlogicals,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        logical_operation() = default;

        logical_operation(primitive_arguments_type&& operlogicals,
            std::string const& name, std::string const& codename);

    private:
        struct visit_logical;

        template <typename T>
        primitive_argument_type logical0d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical0d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type logical0d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif
        template <typename T>
        primitive_argument_type logical0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type logical1d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical1d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical1d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type logical1d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif
        template <typename T>
        primitive_argument_type logical1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

        template <typename T>
        primitive_argument_type logical2d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical2d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical2d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type logical2d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif
        template <typename T>
        primitive_argument_type logical2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type logical3d0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical3d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical3d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical3d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type logical3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
#endif
        template <typename T>
        primitive_argument_type logical_all(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
    };
}}}

#endif



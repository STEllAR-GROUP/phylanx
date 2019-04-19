//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Tianyi Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_COMPARISON_SEP_02_2018_0510PM)
#define PHYLANX_PRIMITIVES_COMPARISON_SEP_02_2018_0510PM

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
    template <typename Op>
    class comparison
      : public primitive_component_base
      , public std::enable_shared_from_this<comparison<Op>>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        comparison() = default;

        comparison(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        struct visit_comparison;

        template <typename T>
        primitive_argument_type comparison0d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison0d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison0d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type comparison0d3d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
#endif
        template <typename T>
        primitive_argument_type comparison0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;

        template <typename T>
        primitive_argument_type comparison1d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison1d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison1d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type comparison1d3d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
#endif
        template <typename T>
        primitive_argument_type comparison1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;

        template <typename T>
        primitive_argument_type comparison2d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison2d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison2d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type comparison2d3d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
#endif
        template <typename T>
        primitive_argument_type comparison2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type comparison3d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison3d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison3d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison3d3d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
        template <typename T>
        primitive_argument_type comparison3d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
#endif

        template <typename T>
        primitive_argument_type comparison_all(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool propagate_type) const;
    };
}}}

#endif



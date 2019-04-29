// Copyright (c) 2019 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_ARGSORT_PRIMITIVE_HPP)
#define PHYLANX_ARGSORT_PRIMITIVE_HPP

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
    class argsort
      : public primitive_component_base
      , public std::enable_shared_from_this<argsort>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        argsort() = default;

        /**
       * https://docs.scipy.org/doc/numpy/reference/generated/numpy.argsort.html#numpy.argsort.
       */

        argsort(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type argsort1d(ir::node_data<T>&& in_array,
            std::int64_t axis, std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort2d_axis0(ir::node_data<T>&& in_array,
            std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort2d_axis1(ir::node_data<T>&& in_array,
            std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort2d(ir::node_data<T>&& in_array,
            std::int64_t axis, std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort3d_axis0(ir::node_data<T>&& in_array,
            std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort3d_axis1(ir::node_data<T>&& in_array,
            std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort3d_axis2(ir::node_data<T>&& in_array,
            std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort3d(ir::node_data<T>&& in_array,
            std::int64_t axis, std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort_flatten2d(ir::node_data<T>&& in_array,
            std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort_flatten3d(ir::node_data<T>&& in_array,
            std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort_flatten_helper(
            ir::node_data<T>&& in_array, std::int64_t axis, std::string kind,
            std::string order) const;

        primitive_argument_type argsort_flatten(
            primitive_argument_type&& in_array, std::int64_t axis,
            std::string kind, std::string order) const;

        template <typename T>
        primitive_argument_type argsort_helper(ir::node_data<T>&& in_array,
            std::int64_t axis, std::string kind, std::string order) const;
    };
    ;

    inline primitive create_argsort(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "argsort", std::move(operands), name, codename);
    }
}}}

#endif /* PHYLANX_ARGSORT_PRIMITIVE_HPP */

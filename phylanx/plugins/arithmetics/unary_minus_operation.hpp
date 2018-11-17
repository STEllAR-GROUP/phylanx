// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_UNARY_MINUS_OPERATION_OCT_10_2017_0248PM)
#define PHYLANX_PRIMITIVES_UNARY_MINUS_OPERATION_OCT_10_2017_0248PM

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
    class unary_minus_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<unary_minus_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        unary_minus_operation() = default;

        unary_minus_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type neg0d(ir::node_data<T>&& op) const;
        template <typename T>
        primitive_argument_type neg1d(ir::node_data<T>&& op) const;
        template <typename T>
        primitive_argument_type neg2d(ir::node_data<T>&& op) const;

        primitive_argument_type neg0d(primitive_argument_type&& op) const;
        primitive_argument_type neg1d(primitive_argument_type&& op) const;
        primitive_argument_type neg2d(primitive_argument_type&& op) const;

    protected:
        node_data_type dtype_;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_unary_minus_operation(
        hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__minus", std::move(operands), name, codename);
    }
}}}

#endif



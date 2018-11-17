// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_HSTACK_OPERATION_JAN13_1200_2018_H
#define PHYLANX_HSTACK_OPERATION_JAN13_1200_2018_H

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class hstack_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<hstack_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        hstack_operation() = default;

        hstack_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        std::size_t get_vecsize(
            primitive_arguments_type const& args) const;

        template <typename T>
        primitive_argument_type hstack0d1d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type hstack0d1d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type hstack2d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type hstack2d(
            primitive_arguments_type&& args) const;

    private:
        node_data_type dtype_;
    };

    inline primitive create_hstack_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "hstack", std::move(operands), name, codename);
    }
}}}


#endif //PHYLANX_HSTACK_OPERATION_JAN13_1200_2018_H

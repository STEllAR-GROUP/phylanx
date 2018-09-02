// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_VSTACK_OPERATION_JAN13_1200_2018_H
#define PHYLANX_VSTACK_OPERATION_JAN13_1200_2018_H

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
    class vstack_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<vstack_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args) const;

    public:
        static match_pattern_type const match_data;

        vstack_operation() = default;

        vstack_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params, eval_mode) const override;

    private:
        template <typename T>
        primitive_argument_type vstack0d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type vstack0d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type vstack1d2d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type vstack1d2d(
            primitive_arguments_type&& args) const;
    };

    inline primitive create_vstack_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "vstack", std::move(operands), name, codename);
    }
}}}


#endif //PHYLANX_VSTACK_OPERATION_JAN13_1200_2018_H

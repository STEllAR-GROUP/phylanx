// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_FOR_OPERATION_HPP
#define PHYLANX_FOR_OPERATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class for_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<for_operation>
    {
    public:
        static match_pattern_type const match_data;

        for_operation() = default;

        for_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    private:
        struct iteration_for;
    };

    inline primitive create_for_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "for", std::move(operands), name, codename);
    }
}}}

#endif //PHYLANX_FOR_OPERATION_HPP

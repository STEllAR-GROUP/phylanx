// Copyright (c) 2021 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/futures/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    /// \brief
    /// \param
    class delete_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<delete_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        delete_operation() = default;

        delete_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type delete_elements(
            primitive_argument_type&& target,
            primitive_argument_type&& index, eval_context ctx) const;

        primitive_argument_type delete_from_dict(ir::dictionary&& target,
            primitive_argument_type&& index, eval_context ctx) const;
        primitive_argument_type delete_from_list(ir::range&& target,
            primitive_argument_type&& index, eval_context ctx) const;
    };

    inline primitive create_delete_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "delete", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::execution_tree::primitives

// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_INVERSE_OPERATION_OCT_09_2017_0154PM)
#define PHYLANX_PRIMITIVES_INVERSE_OPERATION_OCT_09_2017_0154PM

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
    class inverse_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<inverse_operation>
    {
    protected:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        inverse_operation() = default;

        inverse_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type inverse0d(operand_type&& ops) const;
        primitive_argument_type inverse2d(operand_type&& ops) const;
    };

    inline primitive create_inverse_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "inverse", std::move(operands), name, codename);
    }
}}}

#endif

// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_EXPONENTIAL_OPERATION_HPP_OCT031241PM
#define PHYLANX_EXPONENTIAL_OPERATION_HPP_OCT031241PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class exponential_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<exponential_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

    public:
        static match_pattern_type const match_data;

        exponential_operation() = default;

        exponential_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type exponential0d(operand_type&& ops) const;
        primitive_argument_type exponential1d(operand_type&& ops) const;
        primitive_argument_type exponentialxd(operand_type&& ops) const;
    };

    PHYLANX_EXPORT primitive create_exponential_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif    //PHYLANX_EXPONENTIAL_OPERATION_HPP_OCT031241PM

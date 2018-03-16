// Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GRADIENT_PRIMITIVE_HPP
#define PHYLANX_GRADIENT_PRIMITIVE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class gradient_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<gradient_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

    public:
        static match_pattern_type const match_data;

        gradient_operation() = default;

        /**
         * @brief Gradient Operation Primitive
         *
         * Returns gradient of the input
         *
         * @param operands phylanx node data objects
         *
         * If used inside PhySL:
         *
         *      gradient (input )
         *
         *          input : Vector or a Matrix
         *
         *  Note to self: https://stackoverflow.com/questions/24633618/what-does-numpy-gradient-do/24633888
         */

        gradient_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        primitive_argument_type gradient0d(operands_type&& ops) const;
        primitive_argument_type gradient1d(operands_type&& ops) const;
        primitive_argument_type gradient2d(operands_type&& ops) const;
    };

    PHYLANX_EXPORT primitive create_gradient_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif //PHYLANX_GRADIENT_PRIMITIVE_HPP
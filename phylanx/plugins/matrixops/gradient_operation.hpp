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

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class gradient_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<gradient_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        gradient_operation() = default;

        /**
         * @brief Gradient Operation Primitive
         *
         * Returns gradient of the input
         *
         * @param operands phylanx node data objects
         * @param name The name of the primitive
         * @param codename The codename of the primitive
         *
         * If used inside PhySL:
         *
         *      gradient (input , axis)
         *
         *          input : Vector or a Matrix
         *          axis  : Axis to perform gradient along (optional)
         */

        gradient_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type gradient0d(primitive_arguments_type&& ops) const;
        primitive_argument_type gradient1d(primitive_arguments_type&& ops) const;
        primitive_argument_type gradient2d(primitive_arguments_type&& ops) const;

        template <typename T>
        primitive_argument_type gradient1d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type gradient2d(
            ir::node_data<T>&& arg, std::int64_t axis) const;
    };

    inline primitive create_gradient_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "gradient", std::move(operands), name, codename);
    }
}}}

#endif //PHYLANX_GRADIENT_PRIMITIVE_HPP

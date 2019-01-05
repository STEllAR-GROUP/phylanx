//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DETERMINANT_OCT_09_2017_0158PM)
#define PHYLANX_PRIMITIVES_DETERMINANT_OCT_09_2017_0158PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
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
    class determinant
      : public primitive_component_base
      , public std::enable_shared_from_this<determinant>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        determinant() = default;

        determinant(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type determinant0d(
            primitive_argument_type&& op) const;
        primitive_argument_type determinant2d(
            primitive_argument_type&& op) const;

        template <typename T>
        primitive_argument_type determinant0d(ir::node_data<T>&& op) const;
        template <typename T>
        primitive_argument_type determinant2d(ir::node_data<T>&& op) const;
    };

    inline primitive create_determinant(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "determinant", std::move(operands), name, codename);
    }
}}}

#endif

// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_TRANSPOSE_OPERATION_OCT_09_2017_0146PM)
#define PHYLANX_PRIMITIVES_TRANSPOSE_OPERATION_OCT_09_2017_0146PM

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
    class transpose_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<transpose_operation>
    {
    protected:
        using operand_type = ir::node_data<double>;
        using operands_type =
            std::vector<operand_type, arguments_allocator<operand_type>>;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            std::string const& name, std::string const& codename) const;

    public:
        static match_pattern_type const match_data;

        transpose_operation() = default;

        transpose_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args) const override;

    private:
        primitive_argument_type transpose0d1d(operands_type&& ops) const;
        primitive_argument_type transpose2d(operands_type && ops) const;
    };

    inline primitive create_transpose_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "transpose", std::move(operands), name, codename);
    }
}}}

#endif

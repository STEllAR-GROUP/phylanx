// Copyright (c) 2017 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_SQUARE_ROOT_OPERATION)
#define PHYLANX_PRIMITIVES_SQUARE_ROOT_OPERATION

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
    class square_root_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<square_root_operation>
    {
    protected:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args,
            std::string const& name, std::string const& codename) const;

    public:
        static match_pattern_type const match_data;

        square_root_operation() = default;

        square_root_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type square_root_0d(operands_type && ops) const;
        primitive_argument_type square_root_1d(operands_type && ops) const;
        primitive_argument_type square_root_2d(operands_type && ops) const;
    };

    PHYLANX_EXPORT primitive create_square_root_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif

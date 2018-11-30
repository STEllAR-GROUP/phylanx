// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ASSERT_CONDITION)
#define PHYLANX_PRIMITIVES_ASSERT_CONDITION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Debug assertion
    /// Ensures a condition is true and fails otherwise
    /// \param cond A True or False statement
    class assert_condition
      : public primitive_component_base
      , public std::enable_shared_from_this<assert_condition>
    {
    public:
        static match_pattern_type const match_data;

        assert_condition() = default;

        assert_condition(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        using operand_type = primitive_arguments_type;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
    };

    PHYLANX_EXPORT primitive create_assert_condition(
        hpx::id_type const& locality, primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif



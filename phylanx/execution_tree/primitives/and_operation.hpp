//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_AND_OPERATION_OCT_06_2017_0522PM)
#define PHYLANX_PRIMITIVES_AND_OPERATION_OCT_06_2017_0522PM

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
    class and_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<and_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        and_operation() = default;

        and_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

        using operand_type = ir::node_data<bool>;
        using operands_type = std::vector<primitive_argument_type>;

    private:
        struct visit_and;

        template <typename T>
        primitive_argument_type and0d1d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and0d2d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and0d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and1d0d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and1d1d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and1d2d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and1d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and2d0d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and2d1d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and2d2d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and2d(T&& lhs, T&& rhs) const;

        template <typename T>
        primitive_argument_type and_all(T&& lhs, T&& rhs) const;
    };

    PHYLANX_EXPORT primitive create_and_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif



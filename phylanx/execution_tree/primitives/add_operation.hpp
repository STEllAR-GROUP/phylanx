// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM)
#define PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM

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
    class add_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<add_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;

    public:
        static match_pattern_type const match_data;

        add_operation() = default;

        add_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type add0d0d(args_type && args) const;
        primitive_argument_type add0d1d(args_type && args) const;
        primitive_argument_type add0d2d(args_type && args) const;
        primitive_argument_type add0d(args_type && args) const;
        primitive_argument_type add1d0d(args_type && args) const;
        primitive_argument_type add1d1d(args_type && args) const;
        primitive_argument_type add1d2d(args_type&& args) const;
        primitive_argument_type add1d(args_type && args) const;
        primitive_argument_type add2d0d(args_type && args) const;
        primitive_argument_type add2d1d(args_type&& args) const;
        primitive_argument_type add2d2d(args_type && args) const;
        primitive_argument_type add2d(args_type && args) const;
    };

    PHYLANX_EXPORT primitive create_add_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif

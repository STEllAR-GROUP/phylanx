// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_HSTACK_OPERATION_JAN13_1200_2018_H
#define PHYLANX_HSTACK_OPERATION_JAN13_1200_2018_H

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class hstack_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<hstack_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;
        using storage1d_type = typename arg_type::storage1d_type;
        using storage2d_type = typename arg_type::storage2d_type;

    public:
        static match_pattern_type const match_data;

        hstack_operation() = default;

        hstack_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        std::size_t get_vecsize(args_type& args) const;
        primitive_argument_type hstack0d1d(args_type&& args) const;
        primitive_argument_type hstack2d(args_type&& args) const;
    };

    PHYLANX_EXPORT primitive create_hstack_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}


#endif //PHYLANX_HSTACK_OPERATION_JAN13_1200_2018_H

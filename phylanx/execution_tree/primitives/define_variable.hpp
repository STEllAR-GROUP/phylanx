//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DEF_OPERATION_OCT_13_2017_1120AM)
#define PHYLANX_PRIMITIVES_DEF_OPERATION_OCT_13_2017_1120AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    // This primitive creates a new instance of the variable of the given name
    // and initializes it by evaluating the given body.
    //
    // This is a helper primitive needed for proper binding of the expression
    // value to a variable.
    class define_variable
      : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;
        static match_pattern_type const match_data_define;
        static match_pattern_type const match_data_lambda;

        define_variable() = default;

        define_variable(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        // Create a new instance of the variable and initialize it with the
        // value as returned by evaluating the given body.
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

        void store(primitive_argument_type&& val) override;
        void store_set_1d(
            ir::node_data<double>&& data, std::vector<int64_t>&& list) override;
        void store_set_2d(ir::node_data<double>&& data,
            std::vector<int64_t>&& list_row,
            std::vector<int64_t>&& list_col) override;

        // return the topology for this variable definition
        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;
    };
}}}

#endif



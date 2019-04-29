//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_VARIABLE_FACTORY_JAN_03_2019_0127PM)
#define PHYLANX_PRIMITIVES_VARIABLE_FACTORY_JAN_03_2019_0127PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class variable_factory : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        variable_factory() = default;

        variable_factory(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        void store(primitive_argument_type&& data,
            primitive_arguments_type&& params, eval_context ctx) override;

        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

    private:
        primitive_argument_type body_;
        bool create_variable_;
    };
}}}

#endif



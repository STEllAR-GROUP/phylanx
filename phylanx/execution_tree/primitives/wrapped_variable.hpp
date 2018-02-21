//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_WRAPPED_VARIABLE_OCT_22_2017_0111PM)
#define PHYLANX_PRIMITIVES_WRAPPED_VARIABLE_OCT_22_2017_0111PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class wrapped_variable : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        wrapped_variable() = default;

        wrapped_variable(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        void store(primitive_argument_type && val) override;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

        topology expression_topology(
            std::set<std::string>&& functions) const override;

    private:
        primitive_argument_type target_;
    };
}}}

#endif


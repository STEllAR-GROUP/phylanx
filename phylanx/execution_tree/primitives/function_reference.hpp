//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FUNCTION_REFERENCE_JAN_23_2018_0557PM)
#define PHYLANX_PRIMITIVES_FUNCTION_REFERENCE_JAN_23_2018_0557PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class function_reference : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        function_reference() = default;

        function_reference(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

        primitive_argument_type bind(
            std::vector<primitive_argument_type> const& args) const override;

        topology expression_topology(
            std::set<std::string>&& functions) const override;
    };
}}}

#endif


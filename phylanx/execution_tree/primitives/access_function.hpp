//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ACCESS_FUNCTION_JUN_29_2018_0535AM)
#define PHYLANX_PRIMITIVES_ACCESS_FUNCTION_JUN_29_2018_0535AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class access_function : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        access_function() = default;

        access_function(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        void store(primitive_argument_type&& val) override;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params,
            eval_mode mode) const override;

        bool bind(std::vector<primitive_argument_type> const& args,
            bind_mode mode) const override;

        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

    private:
        mutable primitive_argument_type bound_value_;
    };
}}}

#endif


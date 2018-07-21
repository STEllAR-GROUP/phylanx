//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ACCESS_VARIABLE_OCT_22_2017_0111PM)
#define PHYLANX_PRIMITIVES_ACCESS_VARIABLE_OCT_22_2017_0111PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class access_variable
      : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        access_variable() = default;

        access_variable(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        void store(std::vector<primitive_argument_type>&& val) override;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params,
            eval_mode mode) const override;

        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

    private:
        std::shared_ptr<primitive_component> target_;
    };
}}}

#endif


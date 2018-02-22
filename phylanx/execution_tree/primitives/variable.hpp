//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_VARIABLE_SEP_05_2017_1105AM)
#define PHYLANX_PRIMITIVES_VARIABLE_SEP_05_2017_1105AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/lcos/local/spinlock.hpp>

#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class variable : public primitive_component_base
    {
        using mutex_type = hpx::lcos::local::spinlock;

    public:
        static match_pattern_type const match_data;

        variable() = default;

        variable(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& params) const override;

        void store(primitive_argument_type && data) override;

        topology expression_topology(
            std::set<std::string>&& functions) const override;

    private:
        mutable bool evaluated_;
        mutable mutex_type mtx_;
    };

    PHYLANX_EXPORT primitive create_variable(hpx::id_type const& locality,
        primitive_argument_type&& operand,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif



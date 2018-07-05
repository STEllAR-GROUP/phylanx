//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LAMBDA_JUN_28_2018_1010AM)
#define PHYLANX_PRIMITIVES_LAMBDA_JUN_28_2018_1010AM

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
    class lambda
      : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        lambda() = default;

        lambda(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        // return the topology for this function definition
        topology expression_topology(
            std::set<std::string>&& functions) const override;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params,
            eval_mode mode) const override;

        bool bind(
            std::vector<primitive_argument_type> const& args) const override;

        void store(primitive_argument_type&& data) override;
        void set_num_arguments(std::size_t) override;

    private:
        std::size_t num_arguments_;
    };
}}}

#endif


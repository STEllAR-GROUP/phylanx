//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_TARGET_REFERENCE_JAN_23_2018_0557PM)
#define PHYLANX_PRIMITIVES_TARGET_REFERENCE_JAN_23_2018_0557PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class target_reference : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        target_reference() = default;

        target_reference(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context) const override;
        hpx::future<primitive_argument_type> eval(
            primitive_argument_type&& param, eval_context) const override;

        bool bind(primitive_arguments_type const& args,
            eval_context ctx) const override;

        void store(primitive_arguments_type&& data,
            primitive_arguments_type&& params, eval_context ctx) override;

        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

        // initialize evaluation context
        virtual void set_eval_context(eval_context ctx) override;

    private:
        eval_context ctx_;
    };
}}}

#endif


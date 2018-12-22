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

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class variable
      : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        variable() = default;

        variable(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context) const override;
        hpx::future<primitive_argument_type> eval(
            primitive_argument_type && arg, eval_context ctx) const override;

        bool bind(primitive_arguments_type const& params,
            eval_context ctx) const override;

        void store(primitive_arguments_type&& data,
            primitive_arguments_type&& params) override;
        void store(primitive_argument_type&& data,
            primitive_arguments_type&& params) override;

        topology expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const override;

    protected:
        void store1dslice(primitive_arguments_type&& data,
            primitive_arguments_type&& params);
        void store2dslice(primitive_arguments_type&& data,
            primitive_arguments_type&& params);
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        void store3dslice(primitive_arguments_type&& data,
            primitive_arguments_type&& params);
#endif

    private:
        mutable primitive_argument_type bound_value_;
        bool value_set_;
    };

    PHYLANX_EXPORT primitive create_variable(hpx::id_type const& locality,
        primitive_argument_type&& operand,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif



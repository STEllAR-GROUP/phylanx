//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DEBUG_OUTPUT_JAN_18_2018_0120PM)
#define PHYLANX_PRIMITIVES_DEBUG_OUTPUT_JAN_18_2018_0120PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class debug_output
        : public primitive_component_base
        , public std::enable_shared_from_this<debug_output>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        using args_type = primitive_arguments_type;

    public:
        static match_pattern_type const match_data;

        debug_output() = default;

        debug_output(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type operand_;
    };

    PHYLANX_EXPORT primitive create_debug_output(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "");

    ///////////////////////////////////////////////////////////////////////////
    extern match_pattern_type const locality_match_data;

    hpx::future<primitive_argument_type> locality_id(
        phylanx::execution_tree::primitive_arguments_type const&,
        phylanx::execution_tree::primitive_arguments_type const&,
        std::string const&, std::string const&, eval_context);
}}}

#endif



//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ANNOTATE_JUN_17_2019_0138PM)
#define PHYLANX_PRIMITIVES_ANNOTATE_JUN_17_2019_0138PM

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
    class annotate_primitive
      : public primitive_component_base
      , public std::enable_shared_from_this<annotate_primitive>
    {
    public:
        static match_pattern_type const match_data_annotate;
        static match_pattern_type const match_data_annotate_d;

        annotate_primitive() = default;

        annotate_primitive(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    protected:
        hpx::future<primitive_argument_type> eval_annotate(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const;
        hpx::future<primitive_argument_type> eval_annotate_d(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const;

    protected:
        primitive_argument_type annotate(primitive_argument_type&& target,
            ir::range&& args) const;
        primitive_argument_type annotate_d(primitive_argument_type&& target,
            std::string&& name, ir::range&& args) const;

    private:
        std::string func_name_;
    };

    PHYLANX_EXPORT primitive create_annotate(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif


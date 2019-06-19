//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_PRIMITIVE_NAME_FEB_26_2019_1510PM)
#define PHYLANX_PRIMITIVES_PRIMITIVE_NAME_FEB_26_2019_1510PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class primitive_name
      : public primitive_component_base
      , public std::enable_shared_from_this<primitive_name>
    {
    public:
        static match_pattern_type const match_data;

        primitive_name() = default;

        primitive_name(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
    };

    PHYLANX_EXPORT primitive create_primitive_name(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif



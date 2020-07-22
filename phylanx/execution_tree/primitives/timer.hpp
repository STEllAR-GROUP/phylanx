//  Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_TIMER_JUL_17_2020_0439PM)
#define PHYLANX_PRIMITIVES_TIMER_JUL_17_2020_0439PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/futures/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    class timer
      : public primitive_component_base
      , public std::enable_shared_from_this<timer>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        timer() = default;

        timer(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename);
    };

    PHYLANX_EXPORT primitive create_timer(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "");

}}}    // namespace phylanx::execution_tree::primitives

#endif

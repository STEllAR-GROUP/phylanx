// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIV_OPERATION_OCT_07_2017_0631PM)
#define PHYLANX_PRIMITIVES_DIV_OPERATION_OCT_07_2017_0631PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/numeric.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct div_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class div_operation : public numeric<detail::div_op, div_operation>
    {
        using base_type = numeric<detail::div_op, div_operation>;

    public:
        static match_pattern_type const match_data;

        div_operation() = default;

        div_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_div_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__div", std::move(operands), name, codename);
    }
}}}

#endif

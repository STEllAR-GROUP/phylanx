//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_less_equal_OCT_07_2017_0226PM)
#define PHYLANX_PRIMITIVES_less_equal_OCT_07_2017_0226PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/booleans/comparison.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct less_equal_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class less_equal : public comparison<detail::less_equal_op>
    {
        using base_type = comparison<detail::less_equal_op>;

    public:
        static match_pattern_type const match_data;

        less_equal() = default;

        less_equal(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_less_equal(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__le", std::move(operands), name, codename);
    }
}}}

#endif



// Copyright (c) 2017-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_MINIMUM_JAN_10_2019_0138PM)
#define PHYLANX_PRIMITIVES_MINIMUM_JAN_10_2019_0138PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/numeric.hpp>

#include <hpx/futures/future.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct minimum_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class minimum : public numeric<detail::minimum_op, minimum>
    {
        using base_type = numeric<detail::minimum_op, minimum>;

    public:
        static match_pattern_type const match_data;

        minimum() = default;

        minimum(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_minimum(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "minimum", std::move(operands), name, codename);
    }
}}}

#endif

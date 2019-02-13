//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CUMSUM_OCT_03_2018_0953AM)
#define PHYLANX_PRIMITIVES_CUMSUM_OCT_03_2018_0953AM

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/cumulative.hpp>

#include <string>
#include <utility>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct cumsum_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class cumsum
      : public cumulative<detail::cumsum_op, cumsum>
    {
        using base_type = cumulative<detail::cumsum_op, cumsum>;

    public:
        static match_pattern_type const match_data;

        cumsum() = default;

        cumsum(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_cumsum(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "cumsum", std::move(operands), name, codename);
    }
}}}

#endif

//   Copyright (c) 2018 Hartmut Kaiser
//   Copyright (c) 2019 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CUMPROD_JAN_03_2019_1053AM)
#define PHYLANX_PRIMITIVES_CUMPROD_JAN_03_2019_1053AM

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/cumulative.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <utility>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct cumprod_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class cumprod
      : public cumulative<detail::cumprod_op, cumprod>
    {
        using base_type = cumulative<detail::cumprod_op, cumprod>;

    public:
        static match_pattern_type const match_data;

        cumprod() = default;

        cumprod(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_cumprod(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "cumprod", std::move(operands), name, codename);
    }
}}}

#endif

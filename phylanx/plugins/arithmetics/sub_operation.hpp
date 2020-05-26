// Copyright (c) 2017 Bibek Wagle
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_SUB_OPERATION_SEP_15_2017_1035AM)
#define PHYLANX_PRIMITIVES_SUB_OPERATION_SEP_15_2017_1035AM

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
        struct sub_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class sub_operation : public numeric<detail::sub_op, sub_operation>
    {
        using base_type = numeric<detail::sub_op, sub_operation>;

    public:
        static match_pattern_type const match_data;

        sub_operation() = default;

        sub_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_sub_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__sub", std::move(operands), name, codename);
    }
}}}

#endif

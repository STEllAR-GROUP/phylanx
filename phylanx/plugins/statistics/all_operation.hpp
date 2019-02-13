//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ALL_OPERATION_FEB_25_2018_1307PM)
#define PHYLANX_PRIMITIVES_ALL_OPERATION_FEB_25_2018_1307PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        struct statistics_all_op;
    }

    class all_operation
      : public statistics<detail::statistics_all_op, all_operation>
    {
        using base_type =
            statistics<detail::statistics_all_op, all_operation>;

    public:
        static match_pattern_type const match_data;

        all_operation() = default;

        all_operation(primitive_arguments_type&& params,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_all_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "all", std::move(operands), name, codename);
    }
}}}

#endif

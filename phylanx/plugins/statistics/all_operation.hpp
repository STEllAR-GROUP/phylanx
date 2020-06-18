//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018-2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ALL_OPERATION_FEB_25_2018_1307PM)
#define PHYLANX_PRIMITIVES_ALL_OPERATION_FEB_25_2018_1307PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/common/statistics_operations.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    class all_operation
      : public statistics_base<common::statistics_all_op, all_operation>
    {
        using base_type =
            statistics_base<common::statistics_all_op, all_operation>;

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
}}}    // namespace phylanx::execution_tree::primitives

#endif

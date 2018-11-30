//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_AND_OPERATION_OCT_06_2017_0522PM)
#define PHYLANX_PRIMITIVES_AND_OPERATION_OCT_06_2017_0522PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/booleans/logical_operation.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct and_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class and_operation : public logical_operation<detail::and_op>
    {
        using base_type = logical_operation<detail::and_op>;

    public:
        static match_pattern_type const match_data;

        and_operation() = default;

        and_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_and_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__and", std::move(operands), name, codename);
    }
}}}

#endif



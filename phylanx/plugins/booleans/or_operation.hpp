//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_OR_OPERATION_OCT_06_2017_0522PM)
#define PHYLANX_PRIMITIVES_OR_OPERATION_OCT_06_2017_0522PM

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
        struct or_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class or_operation : public logical_operation<detail::or_op>
    {
        using base_type = logical_operation<detail::or_op>;

    public:
        static match_pattern_type const match_data[2];

        or_operation() = default;

        or_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_or_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__or", std::move(operands), name, codename);
    }
}}}

#endif



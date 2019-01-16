//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_XOR_OPERATION_JAN_15_2019_0417PM)
#define PHYLANX_PRIMITIVES_XOR_OPERATION_JAN_15_2019_0417PM

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
        struct xor_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class xor_operation : public logical_operation<detail::xor_op>
    {
        using base_type = logical_operation<detail::xor_op>;

    public:
        static match_pattern_type const match_data[2];

        xor_operation() = default;

        xor_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_xor_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__xor", std::move(operands), name, codename);
    }
}}}

#endif



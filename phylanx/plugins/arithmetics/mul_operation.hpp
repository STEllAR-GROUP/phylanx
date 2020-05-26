// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2017 Alireza Kheirkhahan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_MUL_OPERATION_SEP_25_2017_0900PM)
#define PHYLANX_PRIMITIVES_MUL_OPERATION_SEP_25_2017_0900PM

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
        struct mul_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class mul_operation : public numeric<detail::mul_op, mul_operation>
    {
        using base_type = numeric<detail::mul_op, mul_operation>;

    public:
        static match_pattern_type const match_data;

        mul_operation() = default;

        mul_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    public:
        template <typename T>
        primitive_argument_type handle_numeric_operands_helper(
            primitive_argument_type&& op1, primitive_argument_type&& op2) const;

        template <typename T>
        primitive_argument_type handle_numeric_operands_helper(
            primitive_arguments_type&& ops) const;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_mul_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__mul", std::move(operands), name, codename);
    }
}}}

#endif

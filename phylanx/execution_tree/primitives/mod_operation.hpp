// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2020 Steven R. Brandt
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_MOD_OPERATION_JUN_18_2020_1202PM)
#define PHYLANX_PRIMITIVES_MOD_OPERATION_JUN_18_2020_1202PM

#include <phylanx/config.hpp>

#include <hpx/futures/future.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct mod_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class mod_operation 
      : public primitive_component_base
      , public std::enable_shared_from_this<mod_operation>
    {
        using base_type = primitive_component_base;

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args, eval_context ctx) const;

    public:
        static match_pattern_type const match_data;

        mod_operation() = default;

        mod_operation(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename);
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_mod_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__mod", std::move(operands), name, codename);
    }
}}}

#endif

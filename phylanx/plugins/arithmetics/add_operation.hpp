// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM)
#define PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/numeric.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct add_op;
    }

    ///////////////////////////////////////////////////////////////////////////
    class add_operation : public numeric<detail::add_op, add_operation>
    {
        using base_type = numeric<detail::add_op, add_operation>;

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args, eval_context ctx) const;

    public:
        static match_pattern_type const match_data;

        add_operation() = default;

        add_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type handle_list_operands(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        primitive_argument_type handle_list_operands(
            primitive_arguments_type&& ops) const;

        void append_element(primitive_arguments_type& result,
            primitive_argument_type&& rhs) const;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_add_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__add", std::move(operands), name, codename);
    }
}}}

#endif

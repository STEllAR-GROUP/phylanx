// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Nanmiao Wu
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_ALL_TO_ALL)
#define PHYLANX_ALL_TO_ALL

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <utility>

namespace phylanx { namespace execution_tree { namespace primitives {

    class all_gather
      : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        all_gather() = default;

        all_gather(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        hpx::future<primitive_argument_type> all_gather(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args, eval_context ctx) const;
    };

    inline primitive create_all_gather(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "all_gather", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::execution_tree::primitives

#endif

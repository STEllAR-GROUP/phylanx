//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARANGE_SEP_05_2018_1214PM)
#define PHYLANX_PRIMITIVES_ARANGE_SEP_05_2018_1214PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class arange
      : public primitive_component_base
      , public std::enable_shared_from_this<arange>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        template <typename T>
        primitive_argument_type arange_helper(
            primitive_arguments_type&& args) const;

    public:
        static match_pattern_type const match_data;

        arange() = default;

        arange(primitive_arguments_type&& args,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_arange(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "arange", std::move(operands), name, codename);
    }
}}}

#endif

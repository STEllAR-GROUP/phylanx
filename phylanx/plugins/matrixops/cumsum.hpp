//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CUMSUM_OCT_03_2018_0953AM)
#define PHYLANX_PRIMITIVES_CUMSUM_OCT_03_2018_0953AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class cumsum
      : public primitive_component_base
      , public std::enable_shared_from_this<cumsum>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        cumsum() = default;

        cumsum(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type cumsum_helper(
            primitive_arguments_type&& ops) const;

        template <typename T>
        primitive_argument_type cumsum0d(primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumsum1d(primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumsum2d(primitive_arguments_type&& ops) const;

        template <typename T>
        primitive_argument_type cumsum2d_noaxis(
            primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumsum2d_columns(
            primitive_arguments_type&& ops) const;
        template <typename T>
        primitive_argument_type cumsum2d_rows(
            primitive_arguments_type&& ops) const;

    private:
        node_data_type dtype_;
    };

    inline primitive create_cumsum(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "cumsum", std::move(operands), name, codename);
    }
}}}

#endif

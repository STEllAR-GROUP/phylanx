// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2018 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_MATRIXOPS_SQUEEZE_OPERATION)
#define PHYLANX_PLUGINS_MATRIXOPS_SQUEEZE_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class squeeze_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<squeeze_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        using val_type = double;
        using arg_type = ir::node_data<val_type>;
        using args_type = std::vector<arg_type, arguments_allocator<arg_type>>;

    public:
        static match_pattern_type const match_data;

        squeeze_operation() = default;

        squeeze_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type squeeze0d(
            arg_type&& arg, hpx::util::optional<std::int64_t> axis) const;
        primitive_argument_type squeeze1d(
            arg_type&& arg, hpx::util::optional<std::int64_t> axis) const;
        primitive_argument_type squeeze2d(
            arg_type&& arg, hpx::util::optional<std::int64_t> axis) const;
    };
    inline primitive create_squeeze_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "squeeze", std::move(operands), name, codename);
    }
}}}

#endif 

// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_RESHAPE_OPERATION
#define PHYLANX_RESHAPE_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/small_vector.hpp>

#include <hpx/lcos/future.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class reshape_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<reshape_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
        using arg_type = ir::node_data<std::int64_t>;
        using args_type = std::vector<arg_type>;

    public:
        static match_pattern_type const match_data;

        reshape_operation() = default;

        reshape_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        bool validate_shape(std::int64_t, ir::range&& arg) const;
        primitive_argument_type reshape0d(
            primitive_argument_type&& arr, ir::range&& arg) const;

        template <typename T>
        primitive_argument_type reshape0d(ir::node_data<T>&& arr,
            ir::range&& arg) const;

    private:
        node_data_type dtype_;
    };

    inline primitive create_reshape_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "reshape", std::move(operands), name, codename);
    }
}}}

#endif

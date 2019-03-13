// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_KERAS_SUPPORT_ONE_HOT_OPERATION)
#define PHYLANX_PLUGINS_KERAS_SUPPORT_ONE_HOT_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx {  namespace execution_tree {  namespace primitives  {
/// \brief please refere to https://keras.io/backend/#one_hot

    class one_hot_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<one_hot_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
        using val_type = std::int64_t;
        using arg_type = ir::node_data<val_type>;

    public:
        static match_pattern_type const match_data;

        one_hot_operation() = default;

        one_hot_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type one_hot0d(
            arg_type&& arg, val_type num_classes) const;
        primitive_argument_type one_hot1d(
            arg_type&& arg, val_type num_classes) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type one_hot2d(
            arg_type&& arg, val_type num_classes) const;
#endif
    };
    inline primitive create_one_hot_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "one_hot", std::move(operands), name, codename);
    }
}}}

#endif

// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_MATRIXOPS_SOFTMAX_OPERATION)
#define PHYLANX_PLUGINS_MATRIXOPS_SOFTMAX_OPERATION

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
    /// \brief
    ///
    /// \param a         The scalar, vector, or matrix to perform softmax over
    /// \param axis      Optional. If provided,
    ///

    class softmax_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<softmax_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
        using val_type = double;
        using arg_type = ir::node_data<val_type>;

    public:
        static match_pattern_type const match_data;

        softmax_operation() = default;

        softmax_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type softmax0d(arg_type&& arg) const;
        //primitive_argument_type softmax1d(primitive_argument_type&& arg) const;
        //primitive_argument_type softmax2d(primitive_argument_type&& arg) const;

    };
    inline primitive create_softmax_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "softmax", std::move(operands), name, codename);
    }
}}}

#endif

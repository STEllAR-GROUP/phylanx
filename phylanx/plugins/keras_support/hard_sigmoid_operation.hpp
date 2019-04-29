// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_KERAS_SUPPORT_HARD_SIGMOID_OPERATION)
#define PHYLANX_PLUGINS_KERAS_SUPPORT_HARD_SIGMOID_OPERATION

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

namespace phylanx {  namespace execution_tree {  namespace primitives
{
/// \brief Returns an array of the same shape which is the Segment-wise linear
///        approximation of the sigmoid function.
///
/// \param a  The scalar, vector, matrix, or tensor to perform hard_sigmoid over
///
    class hard_sigmoid_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<hard_sigmoid_operation>
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

        hard_sigmoid_operation() = default;

        hard_sigmoid_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type hard_sigmoid0d(arg_type&& arg) const;
        primitive_argument_type hard_sigmoid1d(arg_type&& arg) const;
        primitive_argument_type hard_sigmoid2d(arg_type&& arg) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type hard_sigmoid3d(arg_type&& arg) const;
#endif
    };

    inline primitive create_hard_sigmoid_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "hard_sigmoid", std::move(operands), name, codename);
    }
}}}

#endif
